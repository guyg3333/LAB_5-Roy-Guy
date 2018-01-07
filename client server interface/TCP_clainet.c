/****************** CLIENT CODE ****************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>

#define PORT_NUM 3490
#define IP_ADDR "192.2.2.1"
#define DEBUG printf



/* -----------struct----------- */

typedef union _group_16{

	unsigned char u8[2];
	short u16;

} group_16;


typedef union _group_32{

	unsigned char u8[4];
	int u32;

} group_32;




int main(){
  int clientSocket,i,port_num,rcv_f,stram,sret;
  int Num_of_Frame;
  char buffer[1024];
  const char* str;
  struct sockaddr_in serverAddr;
  socklen_t addr_size;
  FILE *fd;
  group_16 num_of_station, m_port_num;
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

  printf("send hello \n");                  //Sand hello

  FD_ZERO(&readfds);
  FD_SET(clientSocket,&readfds);

  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  sret = select(3,&readfds, NULL,NULL,&timeout);     // Wait for Welcome massage

  if(sret < 0 ){
  printf("timeout");
  goto CLOSE;

  }//if

  else
  {
	 rcv_f = recv(clientSocket, buffer, 200, 0);
	 if(rcv_f == -1)

	 	 {
	 	   perror("receive error");
	 	   goto START;
	 	 }


	 num_of_station.u8[0] = buffer[1];
	 num_of_station.u8[1] = buffer[2];

	 multicast_addr.u8[0] = buffer[3];
	 multicast_addr.u8[1] = buffer[4];
	 multicast_addr.u8[2] = buffer[5];
	 multicast_addr.u8[3] = buffer[6];

	 m_port_num.u8[0] = buffer[7];
	 m_port_num.u8[1] = buffer[8];

	 printf("num_of_station: %d \n",num_of_station.u16);
	 printf("multicast_addr: %d.%d.%d.%d \n",multicast_addr.u8[0],multicast_addr.u8[1],multicast_addr.u8[2],multicast_addr.u8[3]);
	 printf("m_port_num: %d \n",m_port_num.u16);

  }//else




  // <---------------- Start --  assign to stream service  -------------> //



  // <----------------  End  --  assign to stream service  -------------> //



  stram = 0;

   FD_ZERO(&readfds);
   FD_SET(stram,&readfds);

   timeout.tv_sec = 5;
   timeout.tv_usec = 0;

   printf("enter here\n");
   sret = select(3,&readfds, NULL,NULL,NULL);     // Wait for Welcome massage

   memset((void*) buffer,0,21);
   read(stram,(void*)buffer,20);


   printf("%s\n",buffer);









  CLOSE:

  close(clientSocket);


  printf("end session \n");
//guy
  return 0;
}
