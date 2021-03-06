//                                          compiling - gcc -g UDP_R.o -o UDP_R

/* Receiver/client multicast Datagram example. */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


 #define PORT 6000

struct sockaddr_in localSock;
struct ip_mreq group;
int sd,i = 100000;
int datalen,read = 0;
int optval = 1;
char databuf[1024];
FILE *fd;
const char* str;



int main(int argc, char *argv[]){
	/*---- Open data logging txt file : ----*/
     fd = fopen("myFile.txt", "w");
   	 if(fd ==NULL){
		 perror("file open error");
		 exit(1);
   	 }
	 else
		 printf("file open succeed");



	/* Create a datagram socket on which to receive. */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0){
		perror("Opening datagram socket error");
		exit(1);
	}
	else
		printf("Opening datagram socket....OK.\n");


	/* Enable SO_REUSEADDR to allow multiple instances of this */
	/* application to receive copies of the multicast datagrams. */
	//SOL_SOCKET(level) - have to be here.     SO_REUSEADDR(optname) - let the socket use the same ip:port eventhough its busy
	//optval = shuld be > 0 to enable setsocketopt ??????
	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) < 0){
		perror("Setting SO_REUSEADDR error");
		close(sd);
		exit(1);
	}
	else
		printf("Setting SO_REUSEADDR...OK.\n");


	/* Bind to the proper port number with the IP address */
	/* specified as INADDR_ANY. */
	memset((char *) &localSock, 0, sizeof(localSock));    //zero localsock
	localSock.sin_family = AF_INET;
	localSock.sin_port = htons(PORT);
	localSock.sin_addr.s_addr = INADDR_ANY;   //INADDR_ANY-bind the socket to all available interfaces.
	if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock)))
	{
		perror("Binding datagram socket error");
		close(sd);
		exit(1);
	}
	else
		printf("Binding datagram socket...OK.\n");

	/* Join the multicast group 239.0.0.1 on the local 203.106.93.94 */
	/* interface. Note that this IP_ADD_MEMBERSHIP option must be */
	/* called for each local interface over which the multicast */
	/* datagrams are to be received. */
	group.imr_multiaddr.s_addr = inet_addr("239.0.0.1");
	group.imr_interface.s_addr = inet_addr("192.2.2.1");   //ip of receiver
	if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
	{
		perror("Adding multicast group error");
		close(sd);
		exit(1);
	}
	else
		printf("Adding multicast group...OK.\n");
	read = 1;
	while(i){

		/* Read from the socket. */
		datalen = sizeof(databuf);
		i--;
		read = recvfrom(sd, databuf, datalen,0,0,0);//instead of the two 0's at the end shuld be INADDR_ANY and the second one his size
		if(read < 0)
		{
			perror("Reading datagram message error");
			close(sd);
			//exit(1);
		}
		else
		{
			printf("%s",databuf);
			if(fprintf(fd,"%s",databuf)==-1){
				perror("file error");
				close(sd);			
				exit(1);
			}
				
		}
		bzero(databuf , 1024);
	return 0;

	}
}






