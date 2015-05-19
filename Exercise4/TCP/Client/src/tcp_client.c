#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFFSIZE 1024 /* Max message buffer size */

/* Sock Error */
void Die(char *Message)
{
	perror(Message);
	exit(1);
}

/* This is our thread function.  It is like main(), but for a thread*/
void *Thread_SockReceive(void *ServerSock)
{
	char MessageBuffer[BUFFSIZE];	/* Send or Received internet message buffer */
	int ReceivedDataBytes = 0;	/* Size of received MessageBuffer */

	while (1)
	{
		/* Receive the word back from the server */
		if ((ReceivedDataBytes = recv((*(int*)ServerSock), MessageBuffer, BUFFSIZE-1, 0)) > 0 )
		{
			MessageBuffer[ReceivedDataBytes] = '\0';	//Assure null terminated string
			fprintf(stdout,"%s", MessageBuffer);
		}
	}
}

int main(int argc, char *argv[])
{
	int ServerSock;	/* Server socket */
	struct sockaddr_in ServerAddrInfo;	/* Server address information */
	char MessageBuffer[BUFFSIZE];	/* Send or Received internet message buffer */	
	unsigned int MessageLen;	/* Size of MessageBuffer */
	pthread_t pth;	// this is our thread identifier
	
	if (argc != 3)
	{
		fprintf(stderr, "USAGE: TCPecho <server_ip> <port>\n");
		exit(1);
	}

	/* Create the TCP socket */
	if ((ServerSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		Die("Failed to creat socket");
	}

	/* Construct the server sockaddr_in structure */
	memset(&ServerAddrInfo, 0, sizeof(ServerAddrInfo)); 	/* Clear struct */
	ServerAddrInfo.sin_family = AF_INET;				/* Internet/IP */
	ServerAddrInfo.sin_addr.s_addr = inet_addr(argv[1]);/* IP address */
	ServerAddrInfo.sin_port = htons(atoi(argv[2]));		/* Server port */
	
	/* Establish connection */
	if (connect(ServerSock, (struct sockaddr *) &ServerAddrInfo,
				sizeof(ServerAddrInfo)) < 0)
	{
		Die("Failed to connect with server");
	}

	fprintf(stdout, "Begin Talk\n");

	/* Creat a thread to recevied message from server */
	pthread_create(&pth, NULL, Thread_SockReceive, &ServerSock);

	/* Until you don't want say anything */	
	while(1)
	{
		/* Key in */
		if (fgets(MessageBuffer, sizeof(MessageBuffer), stdin) == NULL)
		{
			fprintf(stdout, "Failed to get input text data\n");
		}
	
		/* Check Input txt data */
		if ((MessageLen = strlen(MessageBuffer)) > BUFFSIZE)
		{
			fprintf(stderr, "Key in total word: %d\nMax buffer size is %d\n"
					, MessageLen, BUFFSIZE);

			exit(1);
		}

		/* Send the word to the server */
		if (send(ServerSock, MessageBuffer, MessageLen, 0) != MessageLen)
		{
			Die("Mismatch in number of sent bytes");
		}
	}
	
	close(ServerSock);

	return 0;
}

