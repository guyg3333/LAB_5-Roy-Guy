/****************** CLIENT CONTROL CODE ****************/

/*---------------------Libraries and Defines----------------------*/
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/select.h>

#define UPDELAY 800
#define PORT_NUM 3490
#define IP_ADDR "192.8.2.1"
#define DEBUG printf
#define WELCOME 0
#define SONGSIZE 1
#define NOT_OPENED -1
#define BUFFERSIZE 1024
#define MAXSONGSIZE 10485760 // =1024*1024*10
#define MINSONGSIZE 2000

#define WELCOME 0
#define ANNOUNCE 1
#define PERMIT_SONG 2
#define INVALID_COMMAND 3
#define NEW_STATION 4
/*--------------------------Structs-------------------------------*/

//Client to Server messages
typedef struct HelloMsg{
	uint8_t commandType;
	uint16_t reserved;
}HelloMsg;
typedef struct AskSongMsg{
	uint8_t commandType;
	uint16_t stationNumber;
}AskSongMsg;
typedef struct UpSongMsg{
uint8_t commandType;
uint32_t songSize; //in bytes
uint8_t songNameSize;
char songName[260];
}UpSongMsg;
//Server to Client messages
typedef struct WelcomeMsg{
uint8_t replyType;//0
uint16_t numOfStations;
uint32_t multicastGroup;
uint16_t portNumber;
}WelcomeMsg;
typedef struct AnnounceMsg{
uint8_t replyType ;//1
uint8_t songNameSize;
char songName[260];
}AnnounceMsg;
typedef struct PermitSongMsg{
uint8_t replyType ;//2
uint8_t permit;
}PermitSongMsg;
typedef struct InvalidCommandMsg{
uint8_t replyType ;//3
uint8_t replyStringSize;
char replyString[];//replyStringSize
}InvalidCommandMsg;
typedef struct NewStationsMsg{
uint8_t replyType ;//4
uint16_t stationNumber;
}NewStationsMsg;

typedef union _group_16{
	unsigned char u8[2];
	short u16;
} group_16;
typedef union _group_32{
	unsigned char u8[4];
	int u32;
} group_32;


/*---------------------Declarations---------------------*/
void clrBuf(char*,int);
void upSong();
void resetTimer();
void uploadSong();
void closeSockets(char*);
void helloMsg();
void welcomeMsg();
void askSong();
void newStation();
/*------------------Global Variables---------------------*/

static int serverTCPsock;
unsigned char buffer[BUFFERSIZE]={0};
short newStationNum;
struct sockaddr_in serverAddr;
fd_set readfds;
FILE * fileP;
int connected;
struct timeval timeout;
WelcomeMsg welMsg;
AnnounceMsg announceMsg;
NewStationsMsg stationMsg;
AskSongMsg askMsg;
UpSongMsg upMsg;
PermitSongMsg permitMsg;

/*------------------------Main----------------------------*/

	
int main(int argc, char*argv[]) {

	if(argc != 3){
		printf("usage: ./radio_control <servername> <serverport>\n");
		return 0;
	}

	int i,port_num,rcv_f,stream,sret;
	int Num_of_Frame;
	const char* str;
	socklen_t addr_size;
	FILE *fd;
	group_16 num_of_stations, m_port_num; ;
	group_32 multicast_addr;


	port_num= atoi(argv[2]); 	//Set port number, using htons function to use proper byte order
	
	printf("Initializing a TCP session with the server..\n");
			   	
	//Create the socket
	if((serverTCPsock = socket(PF_INET, SOCK_STREAM, 0))==-1) //The arguments are: 1)Internet domain. 2)stream socket. 3) Default protocol. 
			closeSockets("Error while trying making the socket..\n");
			
	//Configure settings of the server address struct
	serverAddr.sin_family = AF_INET; //family type IPv4
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);//->h_name); //server IP address/local host
	serverAddr.sin_port = htons(port_num); //server port
	
	//memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  //Set IP address to localhost. Set all bits of the padding to 0
	
	// Connect the socket to the server using the address struct
	
	if(connect(serverTCPsock,(struct sockaddr*)&serverAddr,sizeof(serverAddr))==-1)  
			closeSockets("Error while trying to connect to the server..\n");
	
	memset((void*) buffer,0,20); //Initialize memory
	
	helloMsg(); //Send an Hello Message	
	printf("Sending Hello Message..\n");


	welcomeMsg(); //Waiting for the welcome message and check correctness
	printf("Welcome message arrived successfully..\n\n");

	connected=1;
	 
	// <---------------- Start --  assign to stream service  -------------> //

	// <----------------  End  --  assign to stream service  -------------> //

	//Connection established. Wait for input from the USER
	while(1){

	stream = 0;
	FD_ZERO(&readfds);
	FD_SET(stream,&readfds);
	FD_SET(serverTCPsock,&readfds);

	fflush(stdout);
	printf(" Please enter a command:\n 1.Station number between 0 to %d\n 2.'q' to quit\n 3.'s' to upload song\n",welMsg.numOfStations);
    if(select(serverTCPsock+1,&readfds, NULL,NULL,NULL)<0) // Wait for input from user
			closeSockets(" Failed to use Select() !!\n"); 

			
    if(FD_ISSET(stream,&readfds)){ //User input     

		memset((void*) buffer,0,21);
		read(stream,(void*)buffer,20);

		switch(buffer[0])
		{
			case 'q':	closeSockets("Bye Bye..\n");
			case 'Q':   closeSockets("Bye Bye..\n");
			case 's':   upSong(serverTCPsock);//Upload a song
						break;
			case 'S':	upSong(serverTCPsock);//Upload a song
						break;
			default: 	askSong();
						break;

		}//switch
    }//if
    else{     //if server message receive

	    if(recv(serverTCPsock, buffer, 200, 0)<=0)  
				closeSockets("Error while using recv function! The program will now close..\n");
	   	 	 
		if(buffer[0]==NEW_STATION) //4
				newStation();
		else
				closeSockets("Invalid message! The program will now close..\n");
	
	}//else
  }//while
  closeSockets("end session \n");
}

void newStation()
{
	newStationNum=buffer[1];
	printf("\n There is a new station! the number of the station is %d\n",newStationNum);
	FD_ZERO(&readfds);
	FD_SET(serverTCPsock, &readfds);
}

void welcomeMsg()
{
	group_32 multicast_addr;
	group_16 m_port_num;
	group_16 num_of_stations;
	int recBytes=0;
	resetTimer();
	if(select(serverTCPsock+1,&readfds, NULL,NULL,&timeout) == 0 ) // Wait for Welcome massage 
		closeSockets("Welcome message timeout!"); 
	//select(serverTCPsock+1,&readfds, NULL,NULL,&timeout);
	if(FD_ISSET(serverTCPsock,&readfds))
	{
	recBytes = recv(serverTCPsock ,buffer ,BUFFERSIZE,0);
		
		if(recBytes==9)//TODO checkcorrectness() //to check if its well come maybe add condition if(buffer[0]==WELCOME), WELCOME=0.
		{
			
		num_of_stations.u8[0] = buffer[1];
		num_of_stations.u8[1] = buffer[2];

		multicast_addr.u8[0] = buffer[3];
		multicast_addr.u8[1] = buffer[4];
		multicast_addr.u8[2] = buffer[5];
		multicast_addr.u8[3] = buffer[6];

		m_port_num.u8[0] = buffer[7];
		m_port_num.u8[1] = buffer[8];
			
		welMsg.numOfStations = num_of_stations.u16;	
		welMsg.multicastGroup = multicast_addr.u32;
		welMsg.portNumber = m_port_num.u16;	
			
		printf("Number of Stations: %d\nMulticast Group: %d.%d.%d.%d\nPort number: %d\n",welMsg.numOfStations,multicast_addr.u8[0],multicast_addr.u8[1],multicast_addr.u8[2],multicast_addr.u8[3],welMsg.portNumber);
		return;
		}
		else if(recBytes>0)
			closeSockets("ERROR! number of bytes in welcome message is incorrect..\n");
	}//fd_isset_tcp
	else
	{
			closeSockets("Message timeout! The program will now close..\n");
	}
	resetTimer();
	return;
}//waitforfunc

void helloMsg()
{
	HelloMsg hMsg;
	hMsg.commandType=0;
	hMsg.reserved=0;
	buffer[0]=htons(hMsg.commandType);
	buffer[1]=htons(hMsg.reserved);
	if(send(serverTCPsock,buffer,3,0)<0)
			closeSockets("Error while trying to send an Hello message..\n");
}

void upSong()
{
	int i,recBytes=0;
	group_32 song_size;
	printf("Please enter the song you would like to upload to the server:\n");
	clrBuf(upMsg.songName,260);
	scanf("%s",upMsg.songName);
	upMsg.commandType=2; //first byte is 2
	upMsg.songNameSize=strlen(upMsg.songName);
	if((fileP=fopen(upMsg.songName,"r"))==NULL)
	{
		printf("INVALID song name\n\n");
		return;
	}
	fseek(fileP, 0, SEEK_END);
	upMsg.songSize=ftell(fileP);
	DEBUG("upMsg.songSize %d\n",upMsg.songSize);
	
	rewind(fileP);
	if((upMsg.songSize>=MINSONGSIZE) && (upMsg.songSize<=MAXSONGSIZE))  //songsize has to be between 2000 to 10485760 Bytes
	{
		resetTimer();		
		buffer[0]=upMsg.commandType;
		song_size.u32 = upMsg.songSize;
	//	*(uint32_t*)&buffer[1]=htonl(upMsg.songSize);
		for(i = 0 ;i<4;i++)
			buffer[i+SONGSIZE] = song_size.u8[i];
		
		buffer[5]=upMsg.songNameSize;
		
	

		memcpy(buffer+6,upMsg.songName,upMsg.songNameSize);
		if(send(serverTCPsock,buffer,6+upMsg.songNameSize,0)<0)
		{
			closeSockets("failed to Send upSong message\n");
		}
		clrBuf(buffer,6+upMsg.songNameSize);
		//if(select(serverTCPsock+1,&readfds,NULL,NULL,&timeout)==0)
		if(select(serverTCPsock+1,&readfds,NULL,NULL,NULL)==0)
			closeSockets("Time out passed, quitting...\n");
		
		else if(FD_ISSET(serverTCPsock,&readfds))
		{
			recBytes = recv(serverTCPsock ,buffer ,BUFFERSIZE,0);
			DEBUG("permitMsg.replyType: %d\npermitMsg.permit: %d\n",buffer[0],buffer[1]);
			if(recBytes==2)
			{
				permitMsg.replyType=buffer[0];
				permitMsg.permit=buffer[1];
				if(permitMsg.replyType==2)
				{
					if(permitMsg.permit==1)
					{
						printf("Permit message received! upload the song..\n");
						uploadSong();
					}
					else
					{
						closeSockets("The server denied your request,try later!\n");
					}
				}
				else
				{
					closeSockets("Reply type does not match the permitSong type\n");
				}
			}//recBytes==2
			else
			{
				closeSockets("Server sent invalid msg, wrong number of bytes\n");
			}
		}
	}
	else
	{
		closeSockets("The song size chosen is out of bounds\n");
	}
}

void uploadSong() //First byte in our upsong msg type
{
	char *temp;
	int loopnum=0; //debug
	useconds_t usec=UPDELAY;
	float progress;
	int sentBytesTotal=0,sentBytes=0,recBytes=0;
	
	while(sentBytesTotal<upMsg.songSize && EOF!=fscanf(fileP,"%1024c",buffer)) 
	{
		if(connected==0)
			closeSockets("The server shutdown the connection while uploading a song!  The program will now close..\n");

		loopnum++; //debug
		DEBUG("loopnum %d\n",loopnum); //debug
		if((upMsg.songSize-sentBytesTotal)>BUFFERSIZE)
			sentBytes=send(serverTCPsock,buffer,BUFFERSIZE,0);
		else
			sentBytes=send(serverTCPsock,buffer,(upMsg.songSize-sentBytesTotal),0);
		if(sentBytes<0)	
			closeSockets("ERROR! failed to send. The program will now close..\n");
		
		sentBytesTotal+=sentBytes;

		DEBUG("sentBytesTotal %d\n",sentBytesTotal);
		
		progress=(float)((sentBytesTotal*100)/upMsg.songSize);
		printf("Uploading song! progress: %.2f%%\n",progress);
		fflush(stdout);
	usleep(usec);
	}//while(bytes...)
	if(select(serverTCPsock+1,&readfds,NULL,NULL,&timeout)==0)
		closeSockets("\n Timeout occoured while waiting to the NewStations message from the server\n  The program will now close..\n");
	else if(FD_ISSET(serverTCPsock,&readfds))
	{
		recBytes = recv(serverTCPsock ,buffer ,BUFFERSIZE,0);
		if(buffer[0]!=NEW_STATION)
			closeSockets("\n Wrong reply type! The program will now close..\n");

		if(recBytes==3)
		{
			welMsg.numOfStations++;
			newStation();
			return;
		}
		else
			closeSockets("\n Wrong number of bytes received while waiting for new station after uploading song\n The program will now close..\n");
					
	}

}//uploadSong()

void resetTimer()
{
	FD_ZERO(&readfds);
	FD_SET(0, &readfds);
	FD_SET(serverTCPsock, &readfds);
	timeout.tv_sec = 0;
	timeout.tv_usec = 1000*100;  		//100 ms
	return;
}
void closeSockets(char* s)
{
	//close all opened sockets
	if(serverTCPsock!=NOT_OPENED) close(serverTCPsock);
	perror(s);
	exit(1);
}
void clrBuf(char* buffer,int bufsize) //clear the given buffer,input buffer pointer,buffer size to clear
{
	int i;
	for(i=0;i<bufsize;i++)
		buffer[i]=0;
	return;
}
void askSong()
{
	group_16 station_num;
	int input, bytesRec=0;
	char userInput;
	userInput=buffer[0];
	input=userInput-'0';
	DEBUG("input: %d\n",input);
	if(userInput=='0' || (input>0 && input<=welMsg.numOfStations))
	{
		askMsg.commandType=1;
		askMsg.stationNumber=input;
	}
	else
		closeSockets("Wrong input, the program will now close..\n");
	
	station_num.u16 = askMsg.stationNumber;
	clrBuf(buffer,1024);
	buffer[0]=askMsg.commandType;
	buffer[1]=station_num.u8[0];
	buffer[2]=station_num.u8[1];
	if(send(serverTCPsock,buffer,3,0)<0)
		closeSockets("ERROR while trying to send an AskSong message! The program will now close..\n");
	
	clrBuf(buffer,3);
	resetTimer();
	if(select(serverTCPsock+1,&readfds,NULL,NULL,&timeout)==0)
		closeSockets("Announce message timeout! The program will now close..\n");
		
	if(FD_ISSET(serverTCPsock,&readfds)) //Announce message arrived
	{
		bytesRec = recv(serverTCPsock ,buffer ,BUFFERSIZE,0);
		if(bytesRec>0)
		{
			announceMsg.replyType=buffer[0];
			announceMsg.songNameSize=buffer[1];
			strcpy(announceMsg.songName,buffer+2);
			if(announceMsg.replyType==1)
			{
				printf("reply type: %d,songNameSize: %d songName: %s\n",announceMsg.replyType,announceMsg.songNameSize,announceMsg.songName);
				if(announceMsg.songNameSize==strlen(announceMsg.songName)){
				printf("Announce msg received!\n");
				return;
				}
				else
					closeSockets("Announce message has INVALID song size or name! The program will now close..\n");

			}
			else
			{
				closeSockets("Wrong reply from the server! The program will now close..\n");
			}
		}
		else if(bytesRec==0)
		{
			closeSockets("Server send an INVALID message! The program will now close..\n");
		}
		else
		{
			closeSockets("Server send an INVALID message! The program will now close..\n");
		}
		clrBuf(buffer,BUFFERSIZE);
		resetTimer();
	}

}