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
	char MessageBuffer[BUFFSIZE];	/* Send or Received internet message buffer */
	
	/* Size of MessageBufferSize & Client Address information */
	unsigned int MessageBufferSize, MyAddrInfoSize;
	
	int ReceivedDataBytes = 0;	/* Size of received MessageBuffer */
	int BroadcastPermission; /* Socket opt to set permission to broadcast */

	if (argc != 4)
	{
		fprintf(stderr, "USAGE: %s <broadcast_ip> <server_port> <local_port>\n", argv[0]);
		exit(1);
	}

	/* Creat the UDP socket */
	if ((ServerSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		Die("Failed to creat socket");
		exit(1);
	}

	/* Enable broadcast */
	BroadcastPermission = 1;
	
	/* Set socket to allow broadcast */
	if (setsockopt(ServerSock, SOL_SOCKET, SO_BROADCAST, (void *) &BroadcastPermission, 
          		   sizeof(BroadcastPermission)) < 0)

	/* Construct the server sockaddr_in structure */
	memset(&ServerAddrInfo, 0, sizeof(ServerAddrInfo));	/* Clear struct */
	ServerAddrInfo.sin_family = AF_INET;				/* Internet/IP */
	ServerAddrInfo.sin_addr.s_addr = inet_addr(argv[1]);/* Server IP address */
	ServerAddrInfo.sin_port = htons(atoi(argv[2]));		/* Server port */

	memset(&MyAddrInfo, 0, sizeof(MyAddrInfo));	/* Clear struct */
	MyAddrInfo.sin_family = AF_INET;				/* Internet/IP */
	MyAddrInfo.sin_addr.s_addr = htonl(INADDR_ANY);	/* my IP address */
	MyAddrInfo.sin_port = htons(atoi(argv[3]));		/* my port */
	
	/* Bind the socket */
	if (bind(ServerSock, (struct sockaddr *)&MyAddrInfo, sizeof(ServerAddrInfo)) < 0)
	{
		Die("Failed to bind server socket");
	}

	/* Send the my local port to the server */
	MessageBufferSize = strlen(argv[3]);

	if (sendto(ServerSock, argv[3], MessageBufferSize, 0, (struct sockaddr *) &ServerAddrInfo,
		sizeof(ServerAddrInfo)) != MessageBufferSize)
	{
		Die("Mismatch in number of sent bytes");
	}

	int done = 0;

	/* Until find server */
	while (!done)
	{
		MyAddrInfoSize = sizeof(MyAddrInfo);
		
		/* Receive a message from the client */
		if ((ReceivedDataBytes =
			 recvfrom(ServerSock, MessageBuffer, BUFFSIZE, 0, (struct sockaddr *) &MyAddrInfo,
					  &MyAddrInfoSize)) <0 )
		{
			Die("Failed to receive message");
		}

		MessageBuffer[ReceivedDataBytes] = '\0';
		
		fprintf(stdout, "\nServer connect: %s\n", inet_ntoa(MyAddrInfo.sin_addr));

		fprintf(stdout, "Server port: %d\n", ntohs(MyAddrInfo.sin_port));
		
		done = 1;
	}
	
	close(ServerSock);

	exit(0);
}
