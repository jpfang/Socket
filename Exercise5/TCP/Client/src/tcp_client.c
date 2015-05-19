#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define BUFFSIZE 1024 /* Max message buffer size */

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
	char MessageBuffer[BUFFSIZE];	/* Send or Received internet message buffer */
	unsigned int MessageBufferSize;	/* Size of MessageBuffer */
	int ReceivedDataBytes = 0;	/* Size of received MessageBuffer */
	struct timeval TimeOut;	/* Time out */

	if (argc != 4)
	{
		fprintf(stderr, "USAGE: %s <server_ip> <port> <time_out>\n", argv[0]);
		exit(1);
	}

	/* Setting Time out sec */
	TimeOut.tv_sec = atoi(argv[3]);
	TimeOut.tv_usec = 0;

	/* Create the TCP socket */
	if ((ServerSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		Die("Failed to creat socket");
	}

	/* Setting Time out for socket send*/
	if (setsockopt(ServerSock, SOL_SOCKET, SO_SNDTIMEO,
				   (struct timeval *)&TimeOut,sizeof(struct timeval)) < 0)
	{
		Die("Failed setsockopt SO_SNDTIMEO");
	}

	/* Setting Time out for socket recived*/
	if (setsockopt(ServerSock, SOL_SOCKET, SO_RCVTIMEO,
				   (struct timeval *)&TimeOut,sizeof(struct timeval)) < 0)
	{
		Die("Failed setsockopt SO_RCVTIMEO");
	}

	/* Construct the server sockaddr_in structure */
	memset(&ServerAddrInfo, 0, sizeof(ServerAddrInfo)); /* Clear struct */
	ServerAddrInfo.sin_family = AF_INET;				/* Internet/IP */
	ServerAddrInfo.sin_addr.s_addr = inet_addr(argv[1]);/* IP address */
	ServerAddrInfo.sin_port = htons(atoi(argv[2]));		/* Server port */
	
	/* Establish connection */
	if (connect(ServerSock, (struct sockaddr *) &ServerAddrInfo,
				sizeof(ServerAddrInfo)) < 0)
	{
		Die("Failed to connect with server");
	}

	/* Until you don't want say anything */
	while(1)
	{
		/* Key in */
		fprintf(stdout, "You say:");
		if (fgets(MessageBuffer, sizeof(MessageBuffer), stdin) == NULL)
		{
			fprintf(stdout, "Failed to get input text data\n");
		}
	
		/* Check Input txt data */
		if ((MessageBufferSize = strlen(MessageBuffer)) > BUFFSIZE)
		{
			fprintf(stderr, "Key in total word: %d\nMax buffer size is %d\n"
					, MessageBufferSize, BUFFSIZE);

			exit(1);
		}

		/* Send the word to the server */
		if (send(ServerSock, MessageBuffer, MessageBufferSize, 0) != MessageBufferSize)
		{
			Die("Mismatch in number of sent bytes");
		}

		/* Receive the word back from the server */
		if ((ReceivedDataBytes = recv(ServerSock, MessageBuffer, BUFFSIZE-1, 0)) < 1)
		{
			Die("Failed to receive bytes from server");
		}

		/*MessageBuffer[ReceivedDataBytes] = '\0';	//Assure null terminated string
		fprintf(stdout,"%s\n", MessageBuffer);*/
	}
	
	close(ServerSock);
	return 0;
}
