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


#define COM_TYPE	0
#define STAEION_NUM	1
#define SONG_SIZE	1
#define SONG_NAME_SIZE	5
#define SONG_NAME	6	


#define WELCOME_SIZE 9

#define  REPLY_TYPE 	0
#define  NAME_STATION	1
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



static int num_of_client;
static group_32 mulyicastGroup;
static group_16 port_num;

void make_Wellcom_p(char *buffer);
int make_Song_p(char *buffer, short station);
char getSongName(char *name,short station);
	short getnumStations();



int main( ){
	
  int welcomeSocket, newSocket,Num_of_Frame,i,j;
  char buffer[FRAME_SIZE];
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;
  FILE *fd;

  DEBUG("Start \n");

    
  welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

  //atoi

  /*---- Configure settings of the server address struct ----*/
  /* Address family = Internet */
  serverAddr.sin_family = AF_INET;
  /* Set port number, using htons function to use proper byte order */
  serverAddr.sin_port = htons(PORT);
  /* Set IP address to localhost */
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  /* Set all bits of the padding field to 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  /*---- Bind the address struct to the socket ----*/
  bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  /*---- Listen on the socket, with 5 max connection requests queued ----*/
  if(listen(welcomeSocket,5)==0)
    printf("Listening\n");
  else
    printf("Error\n");

  /*---- Accept call creates a new socket for the incoming connection ----*/
  addr_size = sizeof serverStorage;
  newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);

  
  
/*-----Enter Apllication ----*/  
  
 
  
  
  
  
  
  Apllication_function(newSocket);
   
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
	  buffer[i+NAME_STATION] = num_16.u8[i]; // 1
  
	  for(i = 0 ;i<4;i++)
	  buffer[i+M_GROUP] = mulyicastGroup.u8[i]; //3
	  
	  for(i = 0 ;i<2;i++) //7
	  buffer[i+PORT_NUM] = port_num.u8[i];
	    
  } 
  
  

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
  
  
  
  
  
  
  Apllication_function(int newSocket){
  
  char buffer[200];
  char song_name[200];
  char song_name_size;
  group_16 num_16;
  group_32 song_size;
  int i,rcv_f;

  while(1)
  
  {
	  
  
  rcv_f = recv(newSocket, buffer, 200, 0);
  if(rcv_f == -1)
  {
	  perror("receive error");
	  break;
  }
	  
	  switch(buffer[COM_TYPE])
	  {
		  
		  case HELLO:										  //Hello
		  
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
			  
			  for(i = 0 ;i<4;i++)
			  song_size.u8[i] = buffer[i+SONG_SIZE];
		  
			  for(i = 0 ;i<1;i++)
			  song_name_size = buffer[i+SONG_NAME_SIZE];
			  
			  for(i = 0 ;i<song_name_size;i++)
			  song_name[i] = buffer[i + SONG_NAME];
		  
			 // Upload_song(song_size.u32,song_name_size,song_name);	
		  
			  break;

			default:

				printf("unone massege type  closing TCP connection");
				close(newSocket);
		  
	  }//switch
	}//while(1);
  }//Apllication_function
  
  
  



