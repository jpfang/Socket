#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define MAXPENDING 5 /* Max connection requests */
#define BUFFSIZE 1024 /* The BUFFSIZE constant limits the data sent per loop */

/* Sock Error */
void Die(char *Message)
{
	perror(Message);
	exit(1);
}

/* Handle Client Porcess */
int HandleClient(int SourceSock)
{
	char MessageBuffer[BUFFSIZE];	/* Send or Received internet message buffer */
	int ReceivedDataBytes = -1;	/* Size of received MessageBuffer */

	/* Received message */
	if ((ReceivedDataBytes = recv(SourceSock, MessageBuffer, BUFFSIZE, 0)) <= 0)
	{
		close(SourceSock);
		return -1;
	}

	MessageBuffer[ReceivedDataBytes] = '\0';

	fprintf(stdout,"%s",MessageBuffer);

	/* Send back received data */
	if (send(SourceSock, MessageBuffer, ReceivedDataBytes, 0) != ReceivedDataBytes)
	{
		Die("Failed to send bytes to client");
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ServerSock, SourceSock; /* Source and Server socket */
	int SockIndex;	/* Socket Index */
	struct sockaddr_in ServerAddrInfo;	/* Server address information */
	struct sockaddr_in SourceAddrInfo;	/* Client address information */
	
	/* Size of Server&Client Address information */
	unsigned int SourceAddrInfoSize = sizeof(SourceAddrInfo);

	int on=1; /* To release port immediately when process shutdown */

	if (argc != 2)
	{
		fprintf(stderr, "USAGE:%s <port>\n",argv[0]);
		exit(1);
	}

	/* Creat the TCP socket */
	if ((ServerSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		Die("Failed to create socket");
	}
	
	/* Setting sock parameter for release port */
	if ((setsockopt(ServerSock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
    {  
        perror("setsockopt failed");  
        exit(EXIT_FAILURE);  
    }  

	/* Construct the sever sockaddr_in structure */
	memset(&ServerAddrInfo, 0, sizeof(ServerAddrInfo)); 	//Clear struct
	ServerAddrInfo.sin_family = AF_INET;				//Internet/IP
	ServerAddrInfo.sin_addr.s_addr = htonl(INADDR_ANY);	//Incoming addr
	ServerAddrInfo.sin_port = htons(atoi(argv[1]));		//server port

	/* Bind the server socket */
	if (bind(ServerSock, (struct sockaddr *) &ServerAddrInfo,
			 sizeof(ServerAddrInfo)) < 0)
	{
		Die("Failed to bind the server socket");
	}

	/* Listen on the server socket */
	if (listen(ServerSock, MAXPENDING) < 0)
	{
		Die("Failed to listen on server socket");
	}

	fprintf(stdout, "Listen on the server socket\n");
	
	SockIndex = ServerSock;

	/* Run until cancelled */
	while (1)
	{
		/* Switch socket process */
		if (SockIndex == ServerSock)
		{
			/* Wait for clinet connection */
			if ((SourceSock = accept(ServerSock, (struct sockaddr *) &SourceAddrInfo,
				&SourceAddrInfoSize)) < 0)
			{
				Die("Failed to accpet client connection");
			}

			fprintf(stdout, "Client connected: %s\n",
					inet_ntoa(SourceAddrInfo.sin_addr));

			SockIndex++;
		}
		else
		{
			/* Handle Client */
			SockIndex += HandleClient(SockIndex);
		}
	}
}
