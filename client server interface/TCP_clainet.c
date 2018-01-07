/****************** CLIENT CODE ****************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>

#define PORT_NUM 3490
#define IP_ADDR "192.2.2.1"
#define DEBUG printf

#define WELCOME 0
#define NEW_STATION 4

/* -----------struct----------- */

typedef union _group_16{

	unsigned char u8[2];
	short u16;

} group_16;


typedef union _group_32{

	unsigned char u8[4];
	int u32;

} group_32;

static int clientSocket;


int main(){
  int i,port_num,rcv_f,stram,sret;
  int Num_of_Frame;
  char buffer[1024];
  const char* str;
  struct sockaddr_in serverAddr;
  socklen_t addr_size;
  FILE *fd;
  group_16 num_of_stations, m_port_num; ;
  group_32 multicast_addr;


  fd_set readfds;
  struct timeval timeout;


  START:



    //-----DEBUG

    port_num = 2500;

    //
    printf("start\n");


    //
    /*---- Open data logging txt file : ----*/
     fd = fopen("myFile.txt", "w");
   	 if(fd ==NULL)
   	 {
   	    	 perror("file open");
   	    	 exit(1);
   	 }

   	serverAddr.sin_addr.s_addr = inet_addr("192.168.1.2");


  //

  /*---- Create the socket. The three arguments are: ----*/
  /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
  clientSocket = socket(PF_INET, SOCK_STREAM, 0);

  /*---- Configure settings of the server address struct ----*/
  /* Address family = Internet */
  serverAddr.sin_family = AF_INET;
  /* Set port number, using htons function to use proper byte order */
  serverAddr.sin_port = htons(port_num);
  /* Set IP address to localhost */
  //serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
  /* Set all bits of the padding field to 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  /*---- Connect the socket to the server using the address struct ----*/
  addr_size = sizeof serverAddr;
  if(connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size) == -1) {
	 perror("connect");
     exit(1);
  }

  printf("connect to server \n");



  //-----init
  memset((void*) buffer,0,20);
  i = send(clientSocket,buffer,1,0);
   if(i == -1)
  {
	  perror("receive error");
	  goto START;

  }

  printf("send hello \n");                  		//Sand hello

  FD_ZERO(&readfds);
  FD_SET(clientSocket,&readfds);

  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  sret = select(3,&readfds, NULL,NULL,&timeout);     // Wait for Welcome massage

  if(sret < 0 ){
  printf("timeout");
  goto CLOSE;
  }//if


	 if(recv(clientSocket, buffer, 200, 0)<0){

	 	   perror("receive error");
	 	   goto CLOSE;
	 	 }

	 if(buffer[0] == WELCOME) //0
	 {

	 num_of_stations.u8[0] = buffer[1];
	 num_of_stations.u8[1] = buffer[2];

	 multicast_addr.u8[0] = buffer[3];
	 multicast_addr.u8[1] = buffer[4];
	 multicast_addr.u8[2] = buffer[5];
	 multicast_addr.u8[3] = buffer[6];

	 m_port_num.u8[0] = buffer[7];
	 m_port_num.u8[1] = buffer[8];

	 printf("connect\n");									// connect
	 printf("num_of_station: %d \n",num_of_stations.u16);
	 printf("multicast_addr: %d.%d.%d.%d \n",multicast_addr.u8[0],multicast_addr.u8[1],multicast_addr.u8[2],multicast_addr.u8[3]);
	 printf("m_port_num: %d \n",m_port_num.u16);
	 }//if

	 else{
		 printf("invalid hand shake \n");
		 goto CLOSE; }



  // <---------------- Start --  assign to stream service  -------------> //



  // <----------------  End  --  assign to stream service  -------------> //

  while(1){

   stram = 0;

   //FD_ZERO(&readfds);
   FD_SET(stram,&readfds);



   fflush(stdout);
   printf("enter here\n");

   if(select(3,&readfds, NULL,NULL,NULL)<0){     // Wait for Welcome massage

   perror("select");
   goto CLOSE;
  }

   if(FD_ISSET(stram,&readfds)){                //if the user type input

   memset((void*) buffer,0,21);
   read(stram,(void*)buffer,20);


   printf("%s\n",buffer);

   switch(buffer[0]){

   case 'q':
   case 'Q':    goto CLOSE;

   case 's':
   case 'S':
	            //upSong(clientSocket);
	            break;

  // default: goTostation(buffer);


   }//switch
   }//if

   else{                                         //if server masseage recive

	   if(recv(clientSocket, buffer, 200, 0)<0){

	   	 	   perror("receive error");
	   	 	   goto CLOSE;
	   	 	 }



	     switch(buffer[0]){

	     case NEW_STATION: //4

	    	 num_of_stations.u8[0] = buffer[1];
	    	 num_of_stations.u8[1] = buffer[2];
	    	 break;


	     default:

	    	 printf("invalid massage \n");
	     	 goto CLOSE;


	     }//switch
	}//else

  }//while





  CLOSE:

  close(clientSocket);


  printf("end session \n");
//guy
  return 0;
}
