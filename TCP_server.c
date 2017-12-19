/****************** SERVER CODE ****************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#define DEBUG	  printf //uncomment to remove all printf's
#define FRAME_SIZE 1024

int main(){
  int welcomeSocket, newSocket,Num_of_byte,i;
  char buffer[FRAME_SIZE];
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;
  FILE *fd;

  DEBUG("Start \n");








  /*---- open file to sand: ----*/
  fd = fopen("Alice.txt", "r");

  	 if(fd ==NULL)
  	 {
  	    	 perror("file open");
  	    	 exit(1);
  	 }

  /*---- Create the socket. The three arguments are: ----*/
  /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */

  welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

  //atoi

  /*---- Configure settings of the server address struct ----*/
  /* Address family = Internet */
  serverAddr.sin_family = AF_INET;
  /* Set port number, using htons function to use proper byte order */
  serverAddr.sin_port = htons(3490);
  /* Set IP address to localhost */
  serverAddr.sin_addr.s_addr = inet_addr("192.2.2.1");
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

  /*---- Send message to the socket of the incoming connection ----*/
  while(Num_of_byte)
  {
	  for(i=0;i<FRAME_SIZE;i++)
	  	 {
	  	 buffer[i] = fgetc(fd);
	  	 if( feof(fd) ) {
	  		 	  Num_of_byte = 0;
	  	          break ;
	  	       }
	  	 }

  //strcpy(buffer,"Hello World\n");
  send(newSocket,buffer,1024,0);
  DEBUG("send \n");
  Num_of_byte--;
  }
  close(newSocket);

  return 0;
}
