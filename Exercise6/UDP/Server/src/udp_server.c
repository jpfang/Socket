#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define BUFFSIZE 1024 /* Max Message buffer size */

/* Sock Error */
void Die(char *Message)
{
	perror(Message);
	exit(1);
}

int main(int argc, char *argv[])
{
	int ServerSock;	/* Server socket */
	struct sockaddr_in ServerAddrInfo;	/* Server address information */
	struct sockaddr_in SourceAddrInfo;	/* Client address information */
	char MessageBuffer[BUFFSIZE];	/* Send or Received internet message buffer */
	
	/* Size of Server & Client Address information */
	unsigned int SourceAddrInfoSize;
	
	int ReceivedDataBytes = 0;	/* Size of received MessageBuffer */

	if (argc != 2)
	{
		fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
		exit(1);
	}

	/* Create the UDP socket */
	if ((ServerSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		Die("Failed to create socket");
	}

	/* Construct the server sockaddr_in structure */
	memset(&ServerAddrInfo, 0, sizeof(ServerAddrInfo));	/* Clear struct */
	ServerAddrInfo.sin_family = AF_INET;				/* Internet/IP */
	ServerAddrInfo.sin_addr.s_addr = htonl(INADDR_ANY);	/* IP address */
	ServerAddrInfo.sin_port = htons(atoi(argv[1]));		/* Server port */

	/* Bind the socket */
	if (bind(ServerSock, (struct sockaddr *)&ServerAddrInfo, sizeof(ServerAddrInfo)) < 0)
	{
		Die("Failed to bind server socket");
	}
	
	fprintf(stdout, "Listen on the server socket\n");

	/* Run until cancelled */
	while (1)
	{
		SourceAddrInfoSize = sizeof(SourceAddrInfo);
		
		/* Receive a message from the client */
		if ((ReceivedDataBytes =
			 recvfrom(ServerSock, MessageBuffer, BUFFSIZE, 0, (struct sockaddr *) &SourceAddrInfo,
					  &SourceAddrInfoSize)) <0 )
		{
			Die("Failed to receive message");
		}

		fprintf(stdout, "\nSource connect: %s\n", inet_ntoa(SourceAddrInfo.sin_addr));


		fprintf(stdout, "Source port: %d\n", ntohs(SourceAddrInfo.sin_port));

		ServerAddrInfo.sin_port = SourceAddrInfo.sin_port;

		/* Send back */
		if (sendto(ServerSock, MessageBuffer, ReceivedDataBytes, 0,
				   (struct sockaddr *) &SourceAddrInfo, sizeof(SourceAddrInfo))
			!= ReceivedDataBytes)
		{
			Die("Mismatch in number of echo'd bytes");
		}
	}
}
