/****************** SERVER CODE ****************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>



#define DST_IP "239.0.0.1"
#define DST_PORT 6000


#define STREAM	0
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
#define  M_GROUP	3
#define  PORT_NUM	7

#define ANNOUNCE 	1
#define NAME_SIZE 	1
#define SONG_AN_NAME 2

#define NEW_STATION 4

#define INVALIDCOMMAND 3;
/* -----------Union----------- */

typedef union _group_16{

	unsigned char u8[2];
	short u16;

} group_16;


typedef union _group_32{

	unsigned char u8[4];
	int u32;

} group_32;

/*-------------------------------*/



/* handle the number of client in the server
--------------------------------------
in this program the server hold a static 
varible name " stations "

to use this struct use the fallowing function"

	void init_souket_array();
	void add_souket(int newSocket);
	void rmv_souket(int newSocket);
*/
typedef struct dynamic_souket{  

	int num_of_souket;
	int size;
	int *souket_array;

}dynamic_souket;




/* handle the number of client uploading song
---------------------------------------------

to use this struct use the fallowing function:

	void init_Linkls();
	int add_next(int souket,char* name, int num_of_byte);
	int remove_ls(int souket);
	void LS_size();
*/
typedef struct linkls{

	struct linkls * next;
	int souket;
	FILE *fd  ;
	char name[200];
	int num_of_byte;
	unsigned char name_size;

}linkls;




/*handle the number of station and their song name
---------------------------------------------------

	void init_station_struct ()
	void add_station(char * song_name , unsigned char name_size )
	unsigned char getSongName(char* name,short station)
*/
typedef struct{

	int num_of_station;
	int size;
	char **song_name;

}dynamic_station;

typedef struct{
	int sd;
	FILE *songFD;
	char *songName;
	//int numOfStation;
	//listener list[100];
}station;





//static int num_of_client;
static group_32 mulyicastGroup;
static group_16 port_num;
static dynamic_souket souket_struct;
static dynamic_station stations;
static  fd_set readfds;
static linkls Linkls;
static int max;

struct sockaddr_in tmpIP;
struct in_addr iaddr;
struct sockaddr_in saddr;
struct timespec waitTime;
int sd,i,numOfFrames,len;
int fileSize = 50000;
FILE *fd;
char databuf[1024];
int len;
unsigned char TTL = 128;
char one = 1;
int cn = 0,ssent,i=0;
FILE *fp;
station *stations_radio;
int songsNumber = 0;

void finish(char* s){
	perror(s);
	exit(1);
}





void make_Wellcom_p(char *buffer);
int make_Song_p(unsigned char *buffer, short station);
unsigned char getSongName(char* name,short station);
short getnumStations();
void Apllication_function(int newSocket);
void init_souket_array();
void add_souket(int newSocket);
void rmv_souket(int newSocket);
linkls * find(int souket);
int add_next(int souket,unsigned char song_name_size,char* name, int num_of_byte);
int remove_ls(int souket);
void add_station(char* song_name , unsigned char name_size );
void LS_iteam();
void print_soukets();
void init_station_struct ();
void send_inv(int Socket,char *invM);
int user_input();
void print_station();




int main( ){

	int welcomeSocket, newSocket,i;

	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	 pthread_t Stream;


	init_souket_array();   // init the souket_struct
	init_Linkls();
	init_station_struct();

	//struct timeval timeout;
	DEBUG("Start \n");


	mulyicastGroup.u8[0] = 224;
	mulyicastGroup.u8[1] = 1;
	mulyicastGroup.u8[2] = 2;
	mulyicastGroup.u8[3] = 3;



	port_num.u16 = 2545;





    pthread_create(&Stream, NULL, radio_stream, (void *));


	welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
	add_souket(welcomeSocket);	 // add the welcome souket




	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	//serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//serverAddr.sin_addr.s_addr = inet_addr("127.168.1.2");
	serverAddr.sin_addr.s_addr = inet_addr("132.72.105.95");

	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));


	if(listen(welcomeSocket,5)==0){
		printf("  ccccc   ssss   EEEEEE  \n");
		printf(" cc      ss      E       \n");
		printf(" cc        ss    EEEEEE  \n");
		printf(" ccc        sss  E     	 \n");
		printf("  ccccC  sssss   EEEEEE  \n");
	}
	else
		printf("Error\n");
/*
	addr_size = sizeof serverStorage;
	newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
	add_souket(newSocket);	 // add the new socket
*/


	START:

	printf("Enter:\n1. 'p' to see all the data bases of the available stations\n2. 'q' to quit the program\n");

	//main loop 
	while(1){



		//DEBUG("main while\n");
		FD_ZERO(&readfds); 					//clear the readfds

		for(i=0;i < souket_struct.size;i++)    {   	//init readfds
			if(souket_struct.souket_array[i]!=0)
			FD_SET(souket_struct.souket_array[i],&readfds); 
			}

		FD_SET(0,&readfds); //Stream

		if(select(100,&readfds, NULL,NULL,NULL)<0){     	// wait for select signal

			perror("select");
			//goto CLOSE;
		}
		//DEBUG("select\n");



		if(FD_ISSET(welcomeSocket,&readfds)){   		// check if new welcomeSocket
			newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
			add_souket(newSocket);  			// add the new souket
		}


		for(i=1;i < souket_struct.num_of_souket;i++)      		 //check if new client message
			if(FD_ISSET(souket_struct.souket_array[i],&readfds))
				Apllication_function(souket_struct.souket_array[i]); // answer to client message  


		if(FD_ISSET(STREAM,&readfds)){

				if(user_input()==-1)
					goto CLOSE;
					goto START; }
	}//while

	CLOSE:

	printf("Bye Bye..:) \n");

	/*
	close_sock(); TODO
	close_ls();
	clos_station();
	*/

	for(i=0;i < souket_struct.num_of_souket;i++)       //case of error
		close(souket_struct.souket_array[i]);

	free(souket_struct.souket_array);
	return 0;
}//main




//sand the application packet regard to the client massage
//set buffer with ask song packet




/* -----------  Radio stream --------------  */



//radio_stream
void *radio_stream(){

	while(1)
		{

			if(songsNumber < stations.num_of_station ) // if there is new station
			{
				add_new_station(stations.song_name[songsNumber]);
			}

			for(i = 0;i < songsNumber; i++)
			{
				if(feof(stations_radio[i].songFD))
					rewind(stations_radio[i].songFD);	//if the song is over, start over

				tmpIP.sin_addr.s_addr = (inet_addr(DST_IP)+htonl(i));// saddr.sin_addr.s_addr + htonl(i);

				if((len = fread(databuf,1,1024,stations_radio[i].songFD)) < 0)
					finish("failed to read the file !!\n");

				if(sendto(stations_radio[i].sd,databuf,len,0,(struct sockaddr*)&tmpIP,(size_t)sizeof(tmpIP)) == -1)
					finish("failed to send song !!\n");
			}//for

			waitTime.tv_sec=0;
			waitTime.tv_nsec = 62500000;
			nanosleep(&waitTime,NULL);
		}//while



}//radio_stream


void add_new_station(char* songName){

char buf[200];

	// set content of struct saddr and imreq to zero
	memset((char*)&saddr, 0, sizeof(struct sockaddr_in));
    memset(&iaddr, 0, sizeof(struct in_addr));
	memset((char*)&tmpIP, 0, sizeof(struct sockaddr_in));

	snprintf(buf, sizeof(buf), "MP3_FILE/%s",songName);


	if(!((stations_radio[songsNumber]).songFD = fopen(buf,"rb")))
		finish("failed to open file \n");
	else
		printf("file opened \n");

	saddr.sin_family = PF_INET;
   	saddr.sin_port = htons(0); // Use the first free port
 	saddr.sin_addr.s_addr = htonl(INADDR_ANY); // bind socket to any interface

	//Temp sockaddr struct
	tmpIP.sin_addr.s_addr = saddr.sin_addr.s_addr;
	tmpIP.sin_family = PF_INET;
	tmpIP.sin_port = htons(0);


	//Open a UDP socket
	if((stations_radio[songsNumber].sd = socket(PF_INET,SOCK_DGRAM,0)) < 0)
		finish("Error opening socket");
	else
		printf("Opening the datagram socket...OK.\n");


	if(bind(stations_radio[songsNumber].sd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in)) == -1)
		finish("Error binding socket to interface");
	else
		printf("bind to interface...ok\n");


	iaddr.s_addr = INADDR_ANY; // use DEFAULT interface

	// Set the outgoing interface to DEFAULT
   	setsockopt(stations_radio[songsNumber].sd, IPPROTO_IP, IP_MULTICAST_IF, &iaddr, sizeof(struct in_addr));

	// Set multicast packet TTL to 255
	setsockopt(stations_radio[songsNumber].sd, IPPROTO_IP, IP_MULTICAST_TTL, &TTL, sizeof(TTL));

	// set destination multicast address
   	tmpIP.sin_family = PF_INET;
   	tmpIP.sin_addr.s_addr = inet_addr(DST_IP)+htonl(songsNumber);//saddr.sin_addr.s_addr + htonl(songsNumber);
   	tmpIP.sin_port = htons(DST_PORT);


	// join the multicast group
   	setsockopt(stations_radio[songsNumber].sd, IPPROTO_IP, IP_ADD_MEMBERSHIP,DST_IP, 15);

	fp = popen("play-tmp3->/dev/null 2>&1","w");   //open a pipe. output is sent to dev/null

	songsNumber++;



}





/*-------------------------------------------*/



void Apllication_function(int newSocket){

	unsigned char buffer[1050] ={0};
	char song_name[200];
	unsigned char song_name_size,premit;
	group_16 num_16 ={0};
	group_32 song_size = {0};
	int i,rcv_f;
	linkls * souket;

		DEBUG("application\n");
		rcv_f = recv(newSocket, buffer, 1024, 0);
		if(rcv_f<= 0)
		{
			if(rcv_f<0)
			perror("receive error");

			else
			printf("finack -rm: %d \n",newSocket);

			rmv_souket(newSocket);
			return;
		}



		souket = find(newSocket);
		if(souket != NULL) // if the souket is sanding song data
		{
			DEBUG("souket != NULL\n");
			souket->num_of_byte -= rcv_f;

				for(i = 0;i< rcv_f ; i++)
					fputc(buffer[i], souket->fd);

			if(souket->num_of_byte == 0)
			{
				add_station(souket->name,souket->name_size);
				remove_ls(souket->souket);

				//print_station();

				num_16.u16 = stations.num_of_station;
				buffer[0] = NEW_STATION;
				buffer[1] = num_16.u8[0];
				buffer[2] = num_16.u8[1];


				for(i=1;i < souket_struct.size;i++)    //init readfds
				{
					if(souket_struct.souket_array[i]!=0)
					{
						DEBUG("souket_struct.souket_array[i]!=0 : %d",souket_struct.souket_array[i]!=0);
						send(souket_struct.souket_array[i],buffer,3,0); //sand premit massge
							if(i == -1)
								perror("send error");
					}//if send error

				}//for

			}//if the end of reciving
		}//if find

		else
		{

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

			DEBUG("ASK_SONG\n");
			num_16.u8[0] = buffer[0+STAEION_NUM];
			num_16.u8[1] = buffer[1+STAEION_NUM];

			i = make_Song_p(buffer,num_16.u16);
			i= send(newSocket,buffer,i,0);
			if(i == -1)
			{
				perror("receive error");
				break;
			}
         	break;


		case UP_SONG:                                       //Upsong

			DEBUG("UP_SONG\n");
				
			//1. find if the client uplouading allready // cheak in the souket array flag 
			//2. if the secound byte equal to zero do 2 3 else do 4
			//3. open the file and add the buffer data to it and return
			//4. close the file and sand ip massage to the other procces and clear the fd file from the array

			//souket = find(newSocket);
			//if(souket == NULL) // case this is new ask
			//{


				for(i = 0 ;i<4;i++)
					song_size.u8[i] = buffer[i+SONG_SIZE];

				for(i = 0 ;i<1;i++)
					song_name_size = buffer[i+SONG_NAME_SIZE];

				for(i = 0 ;i<song_name_size;i++)
					song_name[i] = buffer[i + SONG_NAME];


				DEBUG("strlen %d  song name size %d",strlen(song_name),song_name_size);
				 if(strlen(song_name) != song_name_size){
					send_inv(newSocket,"\n ERROR :) SongNameSize field isn't equal to the size of the song name\n");
					DEBUG("UP_SONG error 1\n");
					rmv_souket(newSocket);
					DEBUG("UP_SONG error 2\n");

				    break;
				 }


				premit = 1; //init premiut
				if(add_next(newSocket,song_name_size,song_name,song_size.u32) == -1) // case it secssed add the new
					{
					premit = 0;
					perror("add_next :");
					}

				buffer[0] = PREMIT_SONG; //2
				buffer[1] = premit;
				send(newSocket,buffer,2,0); //sand premit massge

							if(i == -1)
							{
								perror("send error");
								break;
							}

			//}
			break;

			// Upload_song(song_size.u32,song_name_size,song_name);

		default:

			DEBUG("default\n");
			printf("unknown massage type  closing TCP connection");
			rmv_souket(newSocket);


		}//switch
	}//else

}//Apllication_function



int user_input(){

char buffer[20];

DEBUG("user_input()\n");
	memset((void*) buffer,0,20);
	read(STREAM,(void*)buffer,20);

	   printf("%s\n",buffer);

	   switch(buffer[0]){

	   case 'q':
	   case 'Q':
		   return -1;

	   case 'p':
	   case 'P':
		    print_soukets();
		    print_station();
		    //LS_iteam();
		    break;

	   default:
		   printf("unknown input\n");

	   }
	return 1; //TODO when return -1 close
}//user_input


//set buffer for
void make_Wellcom_p(char *buffer){
	group_16 num_16;
	int i;

	buffer[REPLY_TYPE] = WELCOME; //0

	num_16.u16 = stations.num_of_station;

	for(i = 0;i<2;i++)
		buffer[i+NUM_STATION] = num_16.u8[i]; // 1

	for(i = 0 ;i<4;i++)
		buffer[i+M_GROUP] = mulyicastGroup.u8[i]; //3

	for(i = 0 ;i<2;i++) //7
		buffer[i+PORT_NUM] = port_num.u8[i];

}//make_Wellcom_p

int make_Song_p(unsigned char *buffer, short station)
{

	unsigned char song_name_size;
	int i;
	char ans[20] = "No souch station";

	buffer[REPLY_TYPE] = ANNOUNCE; //1
	if(station <= stations.num_of_station-1 && station >=0 )
	{
		song_name_size  = strlen(stations.song_name[station]);
		buffer[NAME_SIZE] = song_name_size;
		snprintf(buffer + SONG_AN_NAME, song_name_size +1, "%s",(char *)stations.song_name[station]);

	}//if
	else
	{
		song_name_size  = strlen(ans);
		buffer[NAME_SIZE] = song_name_size;
		snprintf(buffer + SONG_AN_NAME, song_name_size +1, "%s",ans);
	}//else
		return song_name_size + 2;

}//make_Song_p




/* handle the number of client in the server
--------------------------------------
in this program the server hold a static 
varible name " stations "

to use this struct use the fallowing function"

	void init_souket_array();
	void add_souket(int newSocket);
	void rmv_souket(int newSocket);
*/


void init_souket_array(){

	souket_struct.num_of_souket = 0;
	souket_struct.size = 5;
	souket_struct.souket_array =(int *)calloc(souket_struct.size,sizeof(int));

}

void add_souket(int newSocket){
	int i;

	max |= newSocket;

	if(souket_struct.num_of_souket+1 >= souket_struct.size ) // if there is need to make some more room in the array
	{
		souket_struct.souket_array  =(int *)realloc(souket_struct.souket_array,(souket_struct.size+5)*sizeof(int)); // malloc new space
		souket_struct.size = souket_struct.size+5;

		for(i=souket_struct.num_of_souket;i<souket_struct.size;i++) //
			souket_struct.souket_array[i] = 0;
	}

	//find empty space

	for(i=0;i<souket_struct.size ;i++)
		if(souket_struct.souket_array[i] == 0)
			break; 


	souket_struct.num_of_souket = souket_struct.num_of_souket+1;       //incremant num of souket 
	souket_struct.souket_array[i] = newSocket;	           //place the new souket in the empty place
	//FD_SET(newSocket,&readfds);

	//print_soukets();
	//LS_iteam();

}//add



void rmv_souket(int newSocket){
	int i;
	int *new,*temp;

	max &=~newSocket;
	remove_ls(newSocket);  // cheak if there is a file


	if(souket_struct.num_of_souket>0)
	{
		for(i=0;i<souket_struct.size ;i++) //find the souket to rmv
			if(souket_struct.souket_array[i] == newSocket)
			{
				souket_struct.souket_array[i] = 0;
				close(newSocket);
				souket_struct.num_of_souket = souket_struct.num_of_souket-1;

				//if the num of souket is less then the size-10 then shrink the size by 5
				if(souket_struct.num_of_souket < souket_struct.size-5)
				{
					new  =(int *)calloc(souket_struct.size-5,sizeof(int));

					temp = new;
					for(i=0;i<souket_struct.size ;i++) //do hard copy to the new location
						if(souket_struct.souket_array[i]!=0)
							*temp++ = souket_struct.souket_array[i];

					free(souket_struct.souket_array); //free the old location
					souket_struct.souket_array = new; //point to the new location

					souket_struct.size = souket_struct.size-5;
				}//if
			}//if
	}//if	
	else printf("no argument to remove\n");
	//print_soukets();
	DEBUG(" \n");
	//LS_iteam();


}//rmv_souket


void print_soukets(){
	int i;

	printf("client\n");
	printf("------\n");
	for(i=1;i<souket_struct.size;i++){
		if(souket_struct.souket_array[i]!=0)
		printf("index:	%d	num:	%d	\n",i,souket_struct.souket_array[i]);
	}
	printf("number of client: %d\n",souket_struct.num_of_souket -1);

}


//init wite size 5 and num of station 5;
void init_station_struct (){

stations.size = 5;
stations.num_of_station = 0;
stations.song_name = (char **)calloc(stations.size,sizeof(char*));

}//init_station


//thi methode get name of song and reurn the station number
void add_station(char* song_name , unsigned char name_size )
{
int i;

DEBUG("add_station 1 \n");

if(stations.num_of_station +1 >= stations.size){ //realloc some more room

	stations.song_name =(char **)realloc((int)stations.size+5,sizeof(char*));
	stations.size += 5; }

DEBUG("add_station 2 \n");

stations.song_name[stations.num_of_station] = (char *)malloc(name_size*sizeof(char*));

DEBUG("add_station 3 \n");


for ( i=0;i<name_size;i++)
	stations.song_name[stations.num_of_station][i] = song_name[i];

DEBUG("add_station 4 %s \n",stations.song_name[stations.num_of_station]);


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

void print_station(){
	int i;

	printf("station\n");
	printf("-------\n");
	for(i=0;i<stations.num_of_station;i++)
		printf("index %d    name: %s \n",i,stations.song_name[i]);

	printf("number of station:%d ",stations.num_of_station);
	printf("\n");

}



/* handle the number of client uploading song
---------------------------------------------

to use this struct use the fallowing function:

	void init_Linkls();
	int add_next(int souket,char* name, int num_of_byte);
	int remove_ls(int souket);
	void LS_size();
*/

void init_Linkls(){

	Linkls.next = NULL;
    Linkls.souket = 0;
	Linkls.fd = 0;
	Linkls.num_of_byte = 0;
	Linkls.name_size = 0;
}

int add_next(int souket,unsigned char song_name_size,char* name, int num_of_byte){

char name_buf[200];
char buf[200];
FILE *temp;
linkls *current,*add;
int i;


     add = (linkls*)calloc(1,sizeof(linkls)); //case fiald to calloc
     if(add == NULL )
    	 return -1;

snprintf(add->name, sizeof(add->name), "%s",name); // copy the name to station struct

snprintf(buf, sizeof(buf), "MP3_FILE/%s",name); // open new file with the name of the station
DEBUG("%s",buf);
temp = fopen( buf,"w"); //case fiald to open file
	if(temp == NULL){
		free(add);
		perror("file");
		return -1;}


add->fd = temp ;
add->num_of_byte = num_of_byte;
add->souket = souket;
add->next = NULL;
add->name_size = song_name_size;



current = &Linkls;

while(current->next != NULL)
	current = current->next;

current->next = add;

//print_soukets();

//LS_iteam();
return 1;
}//add_next

linkls * find(int souket){
linkls *current;

DEBUG("souket %d\n",souket);

//LS_iteam();
current = &Linkls;

while(current != NULL){
DEBUG("current.souket %d\n",current->souket);

if(current->souket == souket)
{
return current;
}

current = current->next;

}
return NULL;

}//ind_Linkls

int remove_ls(int souket){

DEBUG("1\n");
linkls *current,*next;

current = &Linkls;
next = current->next;

if(next == NULL){
DEBUG("2\n");
	goto END;
}




while(next != NULL){

DEBUG("4\n");

if(next->souket == souket)
{
	DEBUG("5\n");
	fclose(next->fd); //close file
	current->next = next->next;
	free(next);
	LS_size();
	printf("remove souket\n");
	//print_soukets();
	//LS_iteam();


	return 1; }

current = current->next;
next = next->next;
}

END:
printf("no such souket\n");
return -1;

}//remove_ls

//for debug
void LS_size(){
	int i;
	linkls *current;
	current = &Linkls;

	i=0;
	while(current->next != NULL){
		current = current->next;
		i++;
	}

	printf("LS_size 0 %d\n",i);

}//num_of_ls


void LS_iteam(){
	int i;
	linkls *current;
	current = &Linkls;

	i=0;
	printf("\n");
	while(current != NULL){

		printf("index:	%d	fd:	%d	num_of_byte:	%d	souket:	%d	next:	%p\n",i,current->fd,current->num_of_byte,current->souket,current->next);
		current = current->next;
		i++;
	}

	printf("LS_size %d\n",i-1);

}//num_of_ls

int find_s(char *buf){

	int i;

	for(i=0;i<stations.num_of_station;i++)
		if(strcmp(buf,stations.song_name[i])==0)
			return 1;

	return 0;
}

void send_inv(int Socket,char *invM){
	unsigned char buffer[20];


	buffer[0] = INVALIDCOMMAND;
	buffer[1] = strlen(invM);
	snprintf(buffer+2,(int)buffer[1],"%s",invM);

	send(Socket,buffer,2+buffer[1],0);

}

