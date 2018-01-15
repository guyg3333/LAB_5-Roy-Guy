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
#include <sys/types.h>
#include <pthread.h>

#define UPDELAY 1200//16400
//#define portTCP 3490
#define IP_ADDR "192.8.2.1"
#define DEBUG printf
#define WELCOME 0
#define SONGSIZE 1
#define NOT_OPENED -1
#define BUFFERSIZE 1024
#define MAXSONGSIZE 10485760 // =1024*1024*10 BYTES
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
	uint16_t newStationNumber;
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
uint16_t newStationNumber;
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
void sendSong();
void closeSockets(char*);
void helloMsg();
void welcomeMsg();
void askSong();
void newStation();
void invalidCommandReceived();
void *udpListener();
/*------------------Global Variables---------------------*/

group_32 multicast_addr;
uint32_t myStationAddress;
static int serverTCPsock,udpSock; //Static or not??
unsigned char buffer[BUFFERSIZE]={0};
short newStationNum;
struct sockaddr_in serverAddr;
fd_set readfds;
FILE * fp;
FILE *fileP;
struct timeval timeout;
struct sockaddr_in saddr;
struct ip_mreq imreq;
char udpBuffer[1024];
int stationFlag=0;
u_int yes=1;

WelcomeMsg welMsg;
AnnounceMsg announceMsg;
NewStationsMsg stationMsg;
AskSongMsg askMsg;
UpSongMsg upMsg;
PermitSongMsg permitMsg;
InvalidCommandMsg invMsg;

/*------------------------Main----------------------------*/

	
int main(int argc, char*argv[]) {

	if(argc != 3){
		printf(" usage: ./radio_control <servername> <serverport>\n");
		return 0;
	}
	
	pthread_t controlThread;
	int i,portTCP,rcv_f,stream,sret;
	const char* str;
	socklen_t addr_size;
	FILE *fd;
	group_16 num_of_stations, udp_Port; ;

	udpSock=NOT_OPENED;
	serverTCPsock=NOT_OPENED;
	
	portTCP= atoi(argv[2]); 	//Set port number, using htons function to use proper byte order
	
	printf("\n Initializing a TCP session with the server..\n");
			   	
	//Create the socket
	if((serverTCPsock = socket(PF_INET, SOCK_STREAM, 0))==-1) //The arguments are: 1)Internet domain. 2)stream socket. 3) Default protocol. 
			closeSockets("\n Error while trying making the socket..\n");
			
	//Configure settings of the server address struct
	serverAddr.sin_family = AF_INET; //family type IPv4
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
	serverAddr.sin_port = htons(portTCP); //server port
	
	//memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  //Set IP address to localhost. Set all bits of the padding to 0
	
	// Connect the socket to the server using the address struct
	
	if(connect(serverTCPsock,(struct sockaddr*)&serverAddr,sizeof(serverAddr))==-1)  
			closeSockets("\n Error while trying to connect to the server..\n");
	
	memset((void*) buffer,0,20); //Initialize memory
	
	helloMsg(); //Send an Hello Message	
	printf("\n Sending Hello Message..\n");


	welcomeMsg(); //Waiting for the welcome message and check correctness
	printf("\n Welcome message arrived successfully..\n\n");
	
	
	//I think Here i put the UDP connoction + the UDP Streaming
	pthread_create(&controlThread,NULL,udpListener,NULL); 


	//Connection established. Wait for input from the USER
	while(1){

	stream = 0;
	FD_ZERO(&readfds);
	FD_SET(stream,&readfds);
	FD_SET(serverTCPsock,&readfds);

	fflush(stdout);
	printf("\n Please enter a command:\n 1.Station number between 0 to %d\n 2.'s' to send song\n 3.'q' to quit\n",welMsg.numOfStations);
    if(select(serverTCPsock+1,&readfds, NULL,NULL,NULL)<0) // Wait for input from user
			closeSockets("\n ERROR :)Failed to use Select(). The program will now close..\n"); 

			
    if(FD_ISSET(stream,&readfds)){ //User input     

		memset((void*) buffer,0,21);
		read(stream,(void*)buffer,20);

		switch(buffer[0])
		{
			case 'q':	closeSockets(" Bye Bye..\n");
			case 'Q':   closeSockets(" Bye Bye..\n");
			case 's':   upSong(serverTCPsock);//send a song
						break;
			case 'S':	upSong(serverTCPsock);//send a song
						break;
			default: 	askSong();
						break;

		}//switch
    }//if
    else{     //if server message receive

	    if(recv(serverTCPsock, buffer, 200, 0)<=0)  
				closeSockets("\n Error while using recv function! The program will now close..\n");
	   	 	 
		if(buffer[0]==NEW_STATION) //4
				newStation();
		else if(buffer[0]==INVALID_COMMAND)//3
				invalidCommandReceived();
		else
				closeSockets("\n Invalid message! The program will now close..\n");
	}//else
  }//while

	//for back up, shouldn't reach here
	close(serverTCPsock);
	close(udpSock);	   
	return EXIT_SUCCESS;
}

void *udpListener()
{
	int recv_len,len;
	
	while(1)
	{
		//Set content of struct saddr and imreq to zero
		memset((char*)&saddr, 0, sizeof(struct sockaddr_in));
		memset(&imreq, 0, sizeof(struct ip_mreq));
		
			/*sprintf(mIP,"%d.%d.%d.%d",(int)buffer[1],(int)buffer[2],(int)buffer[3],(int)buffer[4]);
			mPort=(buffer[5]<<8)+buffer[6];
			*/ // ~~~~~~~~~~~to use this code
			
		//Open a udp socket 
		if((udpSock = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
			closeSockets("\n Opening datagram socket error. The program will now close..\n");
		
		
		printf("\n Opening datagram socket....OK.\n");
		//Bind
		saddr.sin_family = PF_INET;
		saddr.sin_port = htons(welMsg.portNumber); // listen to udp PORT
		//saddr.sin_addr.s_addr = inet_addr("192.8.5.1");
	
		saddr.sin_addr.s_addr = htonl(INADDR_ANY); // bind socket to any interface

		printf("%d\n",(uint16_t)saddr.sin_port);
		printf("%d\n",(uint32_t)saddr.sin_addr.s_addr);
		
		if(bind(udpSock,(struct sockaddr*)&saddr, sizeof(struct sockaddr_in)) == -1)
			closeSockets("\n ERROR :) Can't bind the UDP socket. The program will now close..");
		
		printf("\n UDP socket successfully binded..\n");
	
		//Mcast group setting	
		//imreq.imr_multiaddr.s_addr = inet_addr(gethostbyname(myStationAddress));  //not sure about this  //the group mcast ip
		imreq.imr_multiaddr.s_addr = inet_addr("239.0.0.1");
		imreq.imr_interface.s_addr = htonl(INADDR_ANY); // use DEFAULT interface//delete htonl
		
		//JOIN multicast group on default interface
		setsockopt(udpSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,&imreq, sizeof(imreq));  //////////////////////////////////
		 if (setsockopt(udpSock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
		perror("Reusing ADDR failed");
		exit(1);
		}
		//Read from the socket
		len = sizeof(saddr);
		printf("\n UDP listener is waiting..\n");
		
		fp = popen("play -t mp3 -> /dev/null 2>&1", "w"); //open a pipe. output is sent to dev/null

		while(1){
			if(stationFlag) //if stationFlag=1, The station have changed
			{
				stationFlag=0;
				break;
			}
									printf("\n\n 4   \n\n");

			if((recv_len = recvfrom(udpSock,udpBuffer,BUFFERSIZE,0,(struct sockaddr*) &saddr,&len))< 0)
				printf("\n Error while receiving information. The program will now close..");		
			
			else
			{
				printf("%s",udpBuffer);
				fwrite(udpBuffer,sizeof(char),BUFFERSIZE,fp);
				memset((void *)udpBuffer,0,BUFFERSIZE);
			}
			printf("\n\n while   \n\n");
		}
	}
}

void newStation()
{
	newStationNum=buffer[1];
	welMsg.numOfStations++;
	printf("\n There is a new station! the number of the station is %d\n",newStationNum);
	FD_ZERO(&readfds);
	FD_SET(serverTCPsock, &readfds);
}

void welcomeMsg()
{
	group_16 udp_Port;
	group_16 num_of_stations;
	int recBytes=0;
	resetTimer();
	if(select(serverTCPsock+1,&readfds, NULL,NULL,&timeout) == 0 ) // Wait for Welcome massage 
		closeSockets("\n Welcome message timeout! The program will now close..\n"); 
		
	if(FD_ISSET(serverTCPsock,&readfds))
	{
		recBytes = recv(serverTCPsock ,buffer ,BUFFERSIZE,0);
		
		if(buffer[0]==INVALID_COMMAND)	invalidCommandReceived();
		if(buffer[0]!=WELCOME) closeSockets("\n ERROR :) Invalid replytype. The program will now close..\n"); 
		
		if(recBytes==9)
		{
			
		num_of_stations.u8[0] = buffer[1];
		num_of_stations.u8[1] = buffer[2];

		multicast_addr.u8[0] = buffer[3];
		multicast_addr.u8[1] = buffer[4];
		multicast_addr.u8[2] = buffer[5];
		multicast_addr.u8[3] = buffer[6];

		udp_Port.u8[0] = buffer[7];
		udp_Port.u8[1] = buffer[8];
			
		welMsg.numOfStations = num_of_stations.u16;	
		welMsg.multicastGroup = multicast_addr.u32;
		welMsg.portNumber = udp_Port.u16;	
		myStationAddress=welMsg.multicastGroup;
			
		printf(" Number of Stations: %d\n Multicast Group: %d.%d.%d.%d\n Port number: %d\n",welMsg.numOfStations,multicast_addr.u8[0],multicast_addr.u8[1],multicast_addr.u8[2],multicast_addr.u8[3],welMsg.portNumber);
		return;
		}
		else if(recBytes>0)
			closeSockets("ERROR:) the number of bytes in welcome message is incorrect..\n");
	}//fd_isset_tcp
	else
		closeSockets("Message timeout! The program will now close..\n");

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
	printf(" Please enter the song you would like to send to the server:\n");
	clrBuf(upMsg.songName,260);
	scanf("%s",upMsg.songName);
	upMsg.commandType=2; //first byte is 2
	upMsg.songNameSize=strlen(upMsg.songName);
	if((fileP=fopen(upMsg.songName,"r"))==NULL)
	{
		printf(" INVALID song name\n\n");
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
			closeSockets("\n ERROR :) Failed to Send upSong message. The program will now close..\n");
		
		clrBuf(buffer,6+upMsg.songNameSize);
		if(select(serverTCPsock+1,&readfds,NULL,NULL,NULL)==0)
			closeSockets("\n ERROR :) Permit message Time out. The program will now close..\n\n");
		
		else if(FD_ISSET(serverTCPsock,&readfds))
		{
			recBytes = recv(serverTCPsock ,buffer ,BUFFERSIZE,0);
			DEBUG("permitMsg.replyType: %d\npermitMsg.permit: %d\n",buffer[0],buffer[1]);
			if(buffer[0]==INVALID_COMMAND) invalidCommandReceived();
			if(buffer[0]!=PERMIT_SONG) closeSockets("ERROR :) Invalid replytype. The program will now close..\n");	
			if(recBytes==2)
			{
				permitMsg.replyType=buffer[0];
				permitMsg.permit=buffer[1];
				if(permitMsg.replyType==2)
				{
					if(permitMsg.permit==1)
					{
						printf("Permit message received! sending the song to the server..\n");
						sendSong();
					}
					else
					{
						closeSockets("The server has denied your request,try again later!\n");
					}
				}
				else
				{
					closeSockets("ERROR :) Invalid replytype. The program will now close..\n");
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

void sendSong() //First byte in our upsong msg type
{
	char *temp;
	int loopnum=0; //debug
	useconds_t usec=UPDELAY;
	float progress;
	int sentBytesTotal=0,sentBytes=0,recBytes=0;
	
	while(sentBytesTotal<upMsg.songSize && EOF!=fscanf(fileP,"%1024c",buffer)) 
	{
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
		printf("sending song! progress: %.2f%%\n",progress);
		fflush(stdout);
	usleep(usec);
	}//while(bytes...)
	resetTimer();
	if(select(serverTCPsock+1,&readfds,NULL,NULL,&timeout)==0)
		closeSockets("\n Timeout occoured while waiting to the NewStations message from the server\n  The program will now close..\n");
	else if(FD_ISSET(serverTCPsock,&readfds))
	{
		recBytes = recv(serverTCPsock ,buffer ,BUFFERSIZE,0);
		if(buffer[0]!=NEW_STATION)
			closeSockets("\n Wrong reply type! The program will now close..\n");

		if(recBytes==3)
		{
			newStation();
			return;
		}
		else
			closeSockets("\n Wrong number of bytes received while waiting for new station after sending song\n The program will now close..\n");
					
	}

}//sendSong()

void resetTimer()
{
	FD_ZERO(&readfds);
	FD_SET(0, &readfds);
	FD_SET(serverTCPsock, &readfds);
	timeout.tv_sec = 0;
	timeout.tv_usec = 1000*100;  		//100 ms
	return;
}

void closeSockets(char* str)
{
	//close all opened sockets
	if(serverTCPsock!=NOT_OPENED) close(serverTCPsock);
	if(udpSock!=NOT_OPENED) close(udpSock);
	if(str!=0)
		perror(str);

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
		askMsg.newStationNumber=input;
	}
	else
	{
		printf("\n No such station, try again..\n");
		return;
	}
	
	station_num.u16 = askMsg.newStationNumber;
	clrBuf(buffer,BUFFERSIZE);
	buffer[0]=askMsg.commandType;
	buffer[1]=station_num.u8[0];
	buffer[2]=station_num.u8[1];
	if(send(serverTCPsock,buffer,3,0)<0)
		closeSockets("\n ERROR while trying to send an AskSong message! The program will now close..\n");
	
	clrBuf(buffer,3);
	resetTimer();
	if(select(serverTCPsock+1,&readfds,NULL,NULL,&timeout)==0)
		closeSockets("\n Announce message timeout! The program will now close..\n");
		
	if(FD_ISSET(serverTCPsock,&readfds)) //Announce message arrived
	{
		bytesRec = recv(serverTCPsock ,buffer ,BUFFERSIZE,0);
		
		if(buffer[0]==INVALID_COMMAND)	invalidCommandReceived();
		if(buffer[0]!=ANNOUNCE) closeSockets("\n ERROR :) Invalid replytype. The program will now close..\n");
		
		if(bytesRec>0)
		{
			announceMsg.replyType=buffer[0];
			announceMsg.songNameSize=buffer[1];
			strcpy(announceMsg.songName,buffer+2);
			if(announceMsg.replyType==1)
			{
				printf("\n Reply type: %d,songNameSize: %d songName: %s\n",announceMsg.replyType,announceMsg.songNameSize,announceMsg.songName);
				if(announceMsg.songNameSize==strlen(announceMsg.songName)){
				printf("\n Announce message was received successfully received!\n");
				multicast_addr.u8[3]=input+1;
				myStationAddress=multicast_addr.u32;
				stationFlag=1;
				return;
				}
				else
					closeSockets("Announce message has INVALID song size or name! The program will now close..\n");

			}		
			else
			{
				closeSockets("ERROR :) Invalid replytype. The program will now close..\n");
			}
		}
		else if(bytesRec==0)
			closeSockets("Server sent an INVALID message! The program will now close..\n");
		else
			closeSockets("Server sent an INVALID message! The program will now close..\n");
		clrBuf(buffer,BUFFERSIZE);
		resetTimer();
	}

}

void invalidCommandReceived()
{
	strcpy(invMsg.replyString,buffer+2);
	closeSockets(invMsg.replyString);
	return;
}
