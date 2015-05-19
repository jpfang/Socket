#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>	//Added for the nonblocking socket

#define MAXPENDING 10 //Max connection requests
#define BUFFSIZE 32 //The BUFFSIZE constant limits the data sent per loop

#define DEBUG 0

#if DEBUG
	#define PAUSE printf("Press Enter key to continue..."); fgetc(stdin);
#endif

/* Sock Error */
void Die(char *Message)
{
	perror(Message);
	exit(1);
}

/* Handle client process */
int HandleClient(int SourceSock)
{
	char MessageBuffer[BUFFSIZE];	/* Send or Received internet message buffer */
	int ReceivedDataBytes = -1;	/* Size of received MessageBuffer */
	
	/* Received message */
	if ((ReceivedDataBytes = recv(SourceSock, MessageBuffer, BUFFSIZE, 0)) < 1)
	{
		/* Disconnect */
		return 1;
	}

	/* Setting Ending char */
	MessageBuffer[ReceivedDataBytes] = '\0';

#if DEBUG
	sleep(1);
#endif

	fprintf(stdout,"Sock%d say:%s", SourceSock, MessageBuffer);

	/* Send back received data */
	if (send(SourceSock, MessageBuffer, ReceivedDataBytes, 0) != ReceivedDataBytes)
	{
		Die("Failed to send bytes to client");
	}
	
	return 0;
}

int main(int argc, char *argv[])
{
	int ServerSock, SourceSock;	/* Client & Server socket */
	struct sockaddr_in ServerAddrInfo;	/* Server address information */
	struct sockaddr_in SourceAddrInfo;	/* Client address information */
	/* Size of client Address information */
	unsigned int SourceAddrInfoSize = sizeof(SourceAddrInfo);
	
	int on = 1; /* To release port immediately when process shutdown */
	
	fd_set SocksList; /* SocksList file descriptor list */
	fd_set ReadSocks; /* temp file descriptor list for select() */
	
	/* Max Socket number and Socket index */
	int MaxSockNumber, SockIndex;
	unsigned int NumConnect = 0; /* Current number of connects */

	if (argc != 2)
	{
		fprintf(stderr, "USAGE:%s <port>\n",argv[0]);
		exit(1);
	}

	/* Clear the SocksList and temp sets for non-blocking select */
	FD_ZERO(&SocksList);
	FD_ZERO(&ReadSocks);

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
	memset(&ServerAddrInfo, 0, sizeof(ServerAddrInfo)); /* Clear struct */
	ServerAddrInfo.sin_family = AF_INET;				/* Internet/IP */
	ServerAddrInfo.sin_addr.s_addr = htonl(INADDR_ANY);	/* Incoming addr */
	ServerAddrInfo.sin_port = htons(atoi(argv[1]));		/* server port */

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

	/* Add the listener to the SocksList set */
	FD_SET(ServerSock, &SocksList);
	
	/* Keep track of the biggest file descriptor */
	MaxSockNumber = ServerSock;

#if DEBUG
	fprintf(stdout, "ServerSock=%d\n",ServerSock);
	fprintf(stdout, "MaxSockNumber=%d\n",MaxSockNumber);
	PAUSE
#endif

	fprintf(stdout, "Listen on the server socket\n");

	/* Until Server close */
	while (1)
	{
		ReadSocks = SocksList; /* For temp */
		
#if DEBUG
		fprintf(stdout, "select\n");
		PAUSE
#endif

		/* select */
		if (select(MaxSockNumber+1, &ReadSocks, NULL, NULL, NULL) < 0)
		{
			Die("select");
		}

		/* Run through the exising connections looking for data to read */
		for (SockIndex = 0; SockIndex < (MaxSockNumber+1); SockIndex++)
		{
#if DEBUG
			fprintf(stdout, "SockIndex=%d\n",SockIndex);
			PAUSE
#endif
			if (FD_ISSET(SockIndex, &ReadSocks)) /* Check SockIndex */
			{
				/* Handle new connections */
				if (SockIndex == ServerSock)
				{		
#if DEBUG
					fprintf(stdout, "New Connect\n");
					PAUSE
#endif
					/* Connect */
					if ((SourceSock = accept(ServerSock, (struct sockaddr *) &SourceAddrInfo,
											 &SourceAddrInfoSize)) < 0)
					{
						Die("Failed to accpet client connection");
					}

					/* Add New Sock to SocksList list */
					FD_SET(SourceSock, &SocksList);
#if DEBUG
					fprintf(stdout, "SourceSock=%d\n",SourceSock);
					PAUSE
#endif
					/* Keep track of the max */
					if (SourceSock > MaxSockNumber)
					{
						MaxSockNumber = SourceSock;
					}
					
					fprintf(stdout, "\n*****Sock%d from connected: %s*****\n",
							SourceSock,inet_ntoa(SourceAddrInfo.sin_addr));

					NumConnect++; /* Number of Current Connect */

					fprintf(stdout, "*****Current Connects=%d*****\n",NumConnect);
#if DEBUG
					fprintf(stdout, "MaxSockNumber=%d\n",MaxSockNumber);
					fprintf(stdout, "Watting\n");
					PAUSE
#endif
				}
				/* Handle exising connections */
				else
				{
					/* Handle data from a client */
					if (HandleClient(SockIndex))
					{
#if DEBUG
						fprintf(stdout, "Disconnect\n");
#endif
						/* Disconnect process */
						close(SockIndex);

						FD_CLR(SockIndex, &SocksList);

						if (SockIndex >= MaxSockNumber)
						{
							MaxSockNumber = MaxSockNumber - 1;
						}
						
						NumConnect--;

						fprintf(stdout, "\n*****Sock%d Disconnect*****\n",SockIndex);

						fprintf(stdout, "*****Current Connects=%d*****\n",NumConnect);
					}
#if DEBUG
					fprintf(stdout, "Finish\n");
					fprintf(stdout, "MaxSockNumber=%d\n",MaxSockNumber);
					fprintf(stdout, "Watting\n");
					PAUSE
#endif	
				}
			}
		}
	}
}
