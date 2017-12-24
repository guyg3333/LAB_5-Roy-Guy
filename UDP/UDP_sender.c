#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
//                                          compiling - gcc -g UDP_S.o -o UDP_S
#define MCAST_IP "239.0.0.1"
#define PORT 6000
struct in_addr localInterface;
struct sockaddr_in groupSock;
int sd;
char databuf[1024] = "Multicast test message lol!";
int datalen = sizeof(databuf);

int main (int argc, char *argv[ ])
{
/* Create a datagram socket on which to send. */
sd = socket(AF_INET, SOCK_DGRAM, 0);
if(sd < 0){
  perror("Opening datagram socket error");
  exit(1);
}
else
  printf("Opening the datagram socket...OK.\n");
  /* Initialize the group sockaddr structure with a
	group address of 239.0.0.1 and port 6000. */
  memset((char *) &groupSock, 0, sizeof(groupSock));  //zero groupSock
  groupSock.sin_family = AF_INET;                     //IPV4
  groupSock.sin_addr.s_addr = inet_addr(MCAST_IP); 	  //define mcast group IP
  groupSock.sin_port = htons(PORT);                   //port

                     //missing num_parts,file_name
					 //need to keep the interface section?
//=================================================== is--it--needed?   ============================
/* Set local interface for outbound multicast datagrams. */
/* The IP address specified must be associated with a local, */
/* multicast capable interface. */
localInterface.s_addr = inet_addr("192.2.1.1");
if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
{
  perror("Setting local interface error");
  exit(1);
}
else
  printf("Setting the local interface...OK\n");
//======================================================================================================

/* Send a message to the multicast group specified by the*/
/* groupSock sockaddr structure. */
/*int datalen = 1024;*/

if(sendto(sd, databuf, datalen, 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0)
	perror("Sending datagram message error");
else
	printf("Sending datagram message...OK\n");

//Finishing
close(sd);

return 0;

}
