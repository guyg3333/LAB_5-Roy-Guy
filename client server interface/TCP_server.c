/****************** SERVER CODE ****************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>



#define DEBUG	  printf //uncomment to remove all printf's
#define FRAME_SIZE 1024
#define PORT 2500


#define HELLO 		0		
#define ASK_SONG 	1
#define UP_SONG 	2 

#define WELCOME 0
#define PREMIT_SONG 2

#define COM_TYPE	0
#define STAEION_NUM	1
#define SONG_SIZE	1
#define SONG_NAME_SIZE	5
#define SONG_NAME	6	


#define WELCOME_SIZE 9

#define  REPLY_TYPE 	0
#define  NUM_STATION	1
#define  M_GROUP		3
#define  PORT_NUM		7

#define ANNOUNCE 1
#define NAME_SIZE 1


/* -----------struct----------- */

typedef union _group_16{

	char u8[2];
	short u16;

} group_16;


typedef union _group_32{

	char u8[4];
	int u32;

} group_32;

typedef struct dynamic_souket{

	int num_of_souket;
	int size;
	int *souket_array;

}dynamic_souket;



typedef struct linkls{

	linkls* next;
	int souket;
	FILE *fd  ;
	int num_of_byte;

}linkls;




typedef struct dynamic_station{

	int num_of_station;
	int size;
	char **song_name;

}dynamic_station;





static int num_of_client;
static group_32 mulyicastGroup;
static group_16 port_num;
static dynamic_souket souket_struct;
static dynamic_station station_struct;
static  fd_set readfds;
static linkls Linkls;

void make_Wellcom_p(char *buffer);
int make_Song_p(char *buffer, short station);
char getSongName(char *name,short station);
short getnumStations();
void Apllication_function(int newSocket);
void init_souket_array();
void add_souket(int newSocket);
void rmv_souket(int newSocket);



int main( ){

	int welcomeSocket, newSocket,i;

	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;


	init_souket_array();   // init the souket_struct

	//struct timeval timeout;
	DEBUG("Start \n");

	mulyicastGroup.u8[0] = 224;
	mulyicastGroup.u8[1] = 1;
	mulyicastGroup.u8[2] = 2;
	mulyicastGroup.u8[3] = 3;


	port_num.u16 = 2545;

	welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
	add_souket(welcomeSocket);	 // add the welcome souket




	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	if(listen(welcomeSocket,5)==0)
		printf("Listening\n");
	else
		printf("Error\n");

	addr_size = sizeof serverStorage;
	newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
	add_souket(newSocket);	 // add the new souket


	//while1
	while(1){

		
		FD_ZERO(&readfds); //clear the readfds array

		for(i=0;i < souket_struct.num_of_souket;i++)    {   //init readfds
			FD_SET(souket_struct.souket_array[i],&readfds);
			}


		if(select(souket_struct.num_of_souket+3,&readfds, NULL,NULL,NULL)<0){     // if error

			perror("select");
			goto CLOSE;
		}



		if(FD_ISSET(welcomeSocket,&readfds)){   // if new welcomeSocket
			newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
			add_souket(newSocket);  // add the new souket
		}


		for(i=1;i < souket_struct.num_of_souket;i++)      		 //find if client jump
			if(FD_ISSET(souket_struct.souket_array[i],&readfds))
				Apllication_function(souket_struct.souket_array[i]);

	}//while

	CLOSE:

	printf("close\n");
	for(i=0;i < souket_struct.num_of_souket;i++)       //case of error
		close(souket_struct.souket_array[i]);

	free(souket_struct.souket_array);


	return 0;
}



short getnumStations(){

	return 6;
}


char getSongName(char *name,short station){
	int i;
	char buffer[20] = "ROY";

	for(i=0;i<3;i++)
		*name++ = buffer[i];

	return 3;
}


//set buffer for
void make_Wellcom_p(char *buffer){
	group_16 num_16;
	int i;

	buffer[REPLY_TYPE] = WELCOME; //0

	num_16.u16 = getnumStations();

	for(i = 0;i<2;i++)
		buffer[i+NUM_STATION] = num_16.u8[i]; // 1

	for(i = 0 ;i<4;i++)
		buffer[i+M_GROUP] = mulyicastGroup.u8[i]; //3

	for(i = 0 ;i<2;i++) //7
		buffer[i+PORT_NUM] = port_num.u8[i];

}


//set buffer with ask song packet
int make_Song_p(char *buffer, short station)
{
	char song_name_size;
	char name[50];
	int i;


	buffer[REPLY_TYPE] = ANNOUNCE; //1
	song_name_size  = getSongName(name,station);

	buffer[NAME_SIZE] = song_name_size; //1

	for(i = 0 ;i<song_name_size;i++)
		buffer[i+SONG_NAME_SIZE] = name[i];

	return song_name_size + 2;

}





//sand the application packet regard to the client massage
Apllication_function(int newSocket){

	char buffer[1024];
	char song_name[200];
	unsigned char song_name_size,premit;
	group_16 num_16;
	group_32 song_size;
	int i,rcv_f,num;
	linkls souket;


		DEBUG("enter application\n");	
			          
		rcv_f = recv(newSocket, buffer, 200, 0);
		switch(rcv_f){
		case -1: perror("receive error");
				  rmv_souket(newSocket);
				  return;

		case  0: printf("finACK \n");
				 rmv_souket(newSocket);
				 return;
		}//switch


		switch(buffer[COM_TYPE]){

		case HELLO:	
									  //Hello
			DEBUG("HELLO\n");				          
			make_Wellcom_p(buffer);
			i = send(newSocket,buffer,WELCOME_SIZE,0);
			if(i == -1)
			{
				perror("receive error");
				break;
			}

			break;


		case ASK_SONG:	  								  //Ask_song

			for(i = 0 ;i<2;i++)
				num_16.u8[i] = buffer[i+STAEION_NUM];

			i = make_Song_p(buffer,num_16.u16);
			i= send(newSocket,buffer,i,0);
			if(i == -1)
			{
				perror("receive error");
				break;
			}
         	break;


		case UP_SONG:                                       //Upsong
				
				
				
			//1. find if the client uplouading allready // cheak in the souket array flag 
			//2. if the secound byte equal to zero do 2 3 else do 4
			//3. open the file and add the buffer data to it and return
			//4. close the file and sand ip massage to the other procces and clear the fd file from the array

			souket = find(newSocket);

			if(souket == NULL) // case this is new ask
			{
				for(i = 0 ;i<4;i++)
					song_size.u8[i] = buffer[i+SONG_SIZE];

				for(i = 0 ;i<1;i++)
					song_name_size = buffer[i+SONG_NAME_SIZE];

				for(i = 0 ;i<song_name_size;i++)
					song_name[i] = buffer[i + SONG_NAME];

				premit = 0; //init premiut

				if(add_next(newSocket,song_name,song_size.u32)) // case it secssed
					premit = 1;


				buffer[0] = PREMIT_SONG; //2
				buffer[1] = premit;
				send(newSocket,buffer,2,0);

							if(i == -1)
							{
								perror("receive error");
								break;
							}

			}

			else {


				if(souket.num_of_byte < 1024)
				{
					num = souket.num_of_byte;
					souket.num_of_byte = 0;
				}
				else
				{
					num = 1024;
					souket.num_of_byte -= 1024;
				}

					for(int i = 0;i< num ; i++)
						fputc(buffer[i+1], souket.fd);


				if(souket.num_of_byte == 0)
				{
					//clos procedure
				}

			}



			// Upload_song(song_size.u32,song_name_size,song_name); 

			break;
				
		                                       //Upsong
				
			
		
				
			
			


		default:

			printf("unknown massage type  closing TCP connection");
			close(newSocket);

		}//switch
}//Apllication_function




void init_souket_array(){

	souket_struct.num_of_souket = 0;
	souket_struct.size = 5;
	souket_struct.souket_array =(int *)calloc(souket_struct.size,sizeof(int));

}

void add_souket(int newSocket){
	int i,empty_space;

	if(souket_struct.num_of_souket+1 >= souket_struct.size ) // if there is need to make some more rome in the array
	{
		souket_struct.souket_array  =(int *)realloc(souket_struct.souket_array,(souket_struct.size+5)*sizeof(int)); // malloc new space
		souket_struct.size = souket_struct.size+5;
	}

	//find empty space

	for(i=0;i<souket_struct.size ;i++)
		if(souket_struct.souket_array[i] == 0)
			break; 


	souket_struct.num_of_souket = souket_struct.num_of_souket+1;       //incremant num of souket 
	souket_struct.souket_array[i] = newSocket;	           //place the new souket in the empty place
	//FD_SET(newSocket,&readfds);
	DEBUG("num of souket %d \n",souket_struct.num_of_souket);				          			          

}


void rmv_souket(int newSocket){
	int i;
	int *temp;
	if(souket_struct.num_of_souket>0)
	{
		for(i=0;i<souket_struct.size ;i++) //find the souket to rmv
			if(souket_struct.souket_array[i] == newSocket)
			{
				souket_struct.souket_array[i] = 0;
				close(newSocket);

				souket_struct.num_of_souket = souket_struct.num_of_souket-1;

				//if the num of souket is less then the size-10 then shrink the size by 5
				if(souket_struct.num_of_souket < souket_struct.size-10)
				{
					temp  =(int *)calloc(souket_struct.size-5,sizeof(int));

					for(i=0;i<souket_struct.size-5 ;i++) //do hard copy to the new location
						if(souket_struct.souket_array[i]!=0)
							*temp++ = souket_struct.souket_array[i];

					free(souket_struct.souket_array); //free the old location
					souket_struct.souket_array = temp; //point to the new location
				}//if
			}//if
	}//if	
	else printf("no argument to remove\n");

	DEBUG("num of souket %d \n",souket_struct.num_of_souket);				          			          
}//rmv_souket





void init_station_struct (){

stations.size = 5;
stations.num_of_station = 0;
stations.song_name =(char **)calloc(stations.size,sizeof(char*));

}//init_station

//thi methode get name of song and reurn the station number
void add_station(char * song_name , unsigned char name_size )
{

if(stations.num_of_station +1 >= stations.size){ //realloc some more room

	stations.song_name =(char **)realloc(stations.size+5,sizeof(char*));
	stations.size += 5; }


stations.song_name[stations.num_of_station] = (char *)malloc(name_size*sizeof(char*));

for (int i=0;i<name_size;i++)
	stations.song_name[stations.num_of_station][i] = song_name[i];

stations.num_of_station += 1;

}//add_station 

 



unsigned char getSongName(char* name,short station){

unsigned char i = 0;

while(stations.song_name[stations.num_of_station][i] != '\0' )
	{	
	name[i] = stations.song_name[stations.num_of_station][i];
	i++;
	}//while

	return i+1;
}//getSongName



/*
typedef struct fd_up_linkls{

	char* next;
	int souket;
	int fd	  ;

}dynamic_souket;

//to do
//init
//add
//remove
//find
*/

void init_Linkls(){

	Linkls.next = null;
        Linkls.souket = 0;
	Linkls.fd = 0;
	Linkls.num_of_byte = 0;
}



int add_next(int souket,char* name, int num_of_byte){

FILE *temp;
*linkls next,current,add;  


     add = (linkls*)calloc(1,sizeof(linkls)); //case fiald to calloc
     if(pointer < 0)
    	 return -1;

temp = fopen( "%s.txt",name, "w"); //case fiald to open file
	if(temp < 0)	
	return -1;


add.fd = temp ;
add.num_of_byte = num_of_byte;
add.souket = souket;
add.next = null;


//Linkls

Linkls = current;
current.next = next;

while(next != null){
current = next
next.next = next;
}

current.next = add;
return 1;
}//add_next


linkls* find_Linkls(int souket){

*linkls current,ans;

while(current.next != null){

if(current.souket == souket)
return current;

current = current.next;

}

return null;

}//ind_Linkls

int remove_ls(int souket){
	
*linkls current,next;

Song
while(next.next != null){

if(next.souket == souket)
{
	fclose(next.fd); //close file
	current.next = next.next;
	free(next);
	printf("remove souket);
	return 1;
	}
current = next;
next = next.next;}

printf("no such souket);
return -1;

}


