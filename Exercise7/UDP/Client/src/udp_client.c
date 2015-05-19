#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define BUFFSIZE 255

/* Sock Error */
void Die(char *Message)
{
	perror(Message);
	exit(1);
}

int main(int argc, char *argv[])
{
	int ServerSock;	/* Client socket */
	struct sockaddr_in MyAddrInfoSize;	/* Client address information */
	char MessageBuffer[BUFFSIZE];	/* Send or Received internet message buffer */
	
	/* Size of My Address information */
	unsigned int MyAddrInfoSizeSize;
	
	int ReceivedDataBytes = 0;	/* Size of received MessageBuffer */

	struct ip_mreq IPMreq; /* MREQ for Multicast */

	if (argc != 3)
	{
		fprintf(stderr, "USAGE: %s <Multicast_IP> <Local_port>\n", argv[0]);
		exit(1);
	}

	/* Creat the UDP socket */
	if ((ServerSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		Die("Failed to creat socket");
		exit(1);
	}

	/* Construct the server sockaddr_in structure */
	memset(&MyAddrInfoSize, 0, sizeof(MyAddrInfoSize));	/* Clear struct */
	MyAddrInfoSize.sin_family = AF_INET;				/* Internet/IP */
	MyAddrInfoSize.sin_addr.s_addr = htonl(INADDR_ANY); /* my IP address */
	MyAddrInfoSize.sin_port = htons(atoi(argv[2]));		/* my port */

	MyAddrInfoSizeSize = sizeof(MyAddrInfoSize);

	/* Bind the socket */
	if (bind(ServerSock, (struct sockaddr *)&MyAddrInfoSize, MyAddrInfoSizeSize) < 0)
	{
		Die("Failed to bind server socket");
	}

	fprintf(stdout, "To Correct time per 5 secs by Server\n");

	/* Setting Multicast */
	IPMreq.imr_multiaddr.s_addr = inet_addr(argv[1]);
	IPMreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(ServerSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &IPMreq, sizeof(IPMreq)) < 0)
	{
		Die("Failed to setsockopt");
	}

	/* Run until cancelled */
	while (1)
	{
		/* Receive a message from the client */
		if ((ReceivedDataBytes =
			 recvfrom(ServerSock, MessageBuffer, BUFFSIZE, 0, (struct sockaddr *) &MyAddrInfoSize,
					  &MyAddrInfoSizeSize)) <0 )
		{
			Die("Failed to receive message");
		}

		MessageBuffer[ReceivedDataBytes] = '\0';
		
		fprintf(stdout, "Correct Time: %s\n", MessageBuffer);
	}
	
	close(ServerSock);

	exit(0);
}
