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
	int ServerSock;	/* Server socket */
	struct sockaddr_in ServerAddrInfo;	/* Server address information */
	struct sockaddr_in MyAddrInfo;	/* My address information */
	char MessageBuffer[BUFFSIZE];	/* Send or Received internet message MessageBuffer */
	
	/* Size of Client address information and MessageBuffer */
	unsigned int MessageLen, ServerAddrInfoLen;
	int ReceivedDataBytes = 0;

	if (argc != 4)
	{
		fprintf(stderr, "USAGE: %s <server_ip> <server_port> <local_port>\n", argv[0]);
		exit(1);
	}

	//Creat the UDP socket
	if ((ServerSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		Die("Failed to creat socket");
		exit(1);
	}

	/* Construct the server sockaddr_in structure */
	memset(&ServerAddrInfo, 0, sizeof(ServerAddrInfo));	/* Clear struct */
	ServerAddrInfo.sin_family = AF_INET;				/* Internet/IP */
	ServerAddrInfo.sin_addr.s_addr = inet_addr(argv[1]);/* Server IP address */
	ServerAddrInfo.sin_port = htons(atoi(argv[2]));		/* Server port */

	memset(&MyAddrInfo, 0, sizeof(MyAddrInfo));		/* Clear struct */
	MyAddrInfo.sin_family = AF_INET;				/* Internet/IP */
	MyAddrInfo.sin_addr.s_addr = htonl(INADDR_ANY);	/* My IP address */
	MyAddrInfo.sin_port = htons(atoi(argv[3]));		/* My port */

	/* Bind the socket */
	if (bind(ServerSock, (struct sockaddr *)&MyAddrInfo, sizeof(MyAddrInfo)) < 0)
	{
		Die("Failed to bind server socket");
	}

	MessageLen = strlen(argv[3]);

	/* Send the word to the server */
	if (sendto(ServerSock, argv[3], MessageLen, 0, (struct sockaddr *) &ServerAddrInfo,
		sizeof(ServerAddrInfo)) != MessageLen)
	{
		Die("Mismatch in number of sent bytes");
	}

	int done = 0;

	/* Run until Server ehco */
	while (!done)
	{
		ServerAddrInfoLen = sizeof(ServerAddrInfo);

		/* Receive a message from the client */
		if ((ReceivedDataBytes =
			 recvfrom(ServerSock, MessageBuffer, BUFFSIZE, 0, (struct sockaddr *) &ServerAddrInfo,
					  &ServerAddrInfoLen)) <0 )
		{
			Die("Failed to receive message");
		}

		MessageBuffer[ReceivedDataBytes] = '\0';
		
		fprintf(stdout, "\nServer connect: %s\n", inet_ntoa(ServerAddrInfo.sin_addr));

		fprintf(stdout, "Server port: %d\n", ntohs(ServerAddrInfo.sin_port));
		
		done = 1;
	}
	
	close(ServerSock);

	exit(0);
}
