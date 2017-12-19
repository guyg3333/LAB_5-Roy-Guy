/****************** CLIENT CODE ****************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>

#define PORT_NUM 3490
#define IP_ADDR "192.2.2.1"
#define DEBUG printf

int main(int argc, char *argv[]){
  int clientSocket,i,port_num;
  int Num_of_Frame;
  char buffer[1024];
  const char* str;
  struct sockaddr_in serverAddr;
  socklen_t addr_size;
  FILE *fd;


  //
    if (argc != 4) {
            printf("./TCP_clainet <IP address> <Port> <Num of Frame >\n");
            exit(1);
        }

    port_num = atoi(argv[2]);
    Num_of_Frame = atoi(argv[3]);

    printf("%s  %d %d \n",argv[1],port_num,Num_of_Frame);



    //
    /*---- Open data logging txt file : ----*/
     fd = fopen("myFile.txt", "w");
   	 if(fd ==NULL)
   	 {
   	    	 perror("file open");
   	    	 exit(1);
   	 }




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
  serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
  /* Set all bits of the padding field to 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  /*---- Connect the socket to the server using the address struct ----*/
  addr_size = sizeof serverAddr;
  if(connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size) == -1) {
	 perror("connect");
     exit(1);
  }

  while(Num_of_Frame)
  {

  /*---- Read the message from the server into the buffer ----*/
  recv(clientSocket, buffer, 1024, 0);

  /*---- Print the received message ----*/

  printf("Data received: %s",buffer);

  if(fprintf(fd,"%s",buffer)==-1)
  	  {
  		  perror("file");
  		      exit(1);
  	  }
  bzero(buffer , 1024);

  /*---- Clear buffer ----*/

  //for(i=0;i<1024;i++)

	//  buffer[i] = 0;
  Num_of_Frame = Num_of_Frame-1;
  }
  close(clientSocket);
  fclose(fd);

  printf("end session \n");

  return 0;
}
