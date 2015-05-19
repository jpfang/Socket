#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>	/* Added for the nonblocking socket */

#define MAXPENDING 10 /* Max connection requests */
#define BUFFSIZE 1024 /* The BUFFSIZE constant limits the data sent per loop */
#define MAXCONNECTS 3 /* Max connection at one time */

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
int HandleClient(int SourceSock, char *MessageBuffer, int *ConnectTable, int ServerSock ,
				 int MaxSockNumber, int Delay)
{
	char TempMessageBuffer[BUFFSIZE]; /* Temp for message buffer */
	int MessageLen;	/* Message Lenth */
	int SockIndex; /* Sock Index */

	sleep(Delay);
#if DEBUG
	sleep(1);
#endif
	
	/* Check Sock is server socket or disconnect socket*/
	if (SourceSock == ServerSock || !ConnectTable[SourceSock - ServerSock])
	{
		fprintf(stdout, "\n===================\n%s===================\n", MessageBuffer);
		sprintf(TempMessageBuffer, "\n===================\n%s===================\n", MessageBuffer);
	}
	else
	{
		/* connect socket socket */
		/* Who saying? */
		fprintf(stdout, "Sock%d say:%s", SourceSock, MessageBuffer);
		sprintf(TempMessageBuffer,"Sock%d say:%s", SourceSock, MessageBuffer);
	}

	MessageLen = strlen(TempMessageBuffer);

	/* Send to connect members, expect server and null sockets */
	for (SockIndex = ServerSock+1 ; SockIndex < MaxSockNumber+1; SockIndex++)
	{
		/* Check the socket if exist? */
		if (ConnectTable[SockIndex - ServerSock])
		{
			/* Send back received data */
			if (SockIndex != SourceSock)
			{
				if (send(SockIndex, TempMessageBuffer, MessageLen, 0) != MessageLen)
				{
					Die("Failed to send bytes to client");
				}
			}
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ServerSock, SourceSock;	/* Client & Server socket */
	struct sockaddr_in ServerAddrInfo;	/* Server address information */
	struct sockaddr_in SourceAddrInfo;	/* Source address information */
	
	/* Size of client Address information */
	unsigned int SourceAddrInfoSize = sizeof(SourceAddrInfo);

	char MessageBuffer[BUFFSIZE];	/* Send or Received internet message buffer */
	int ReceivedDataBytes = -1;	/* Size of received MessageBuffer */
	int on = 1; /* To release port immediately when process shutdown */

	fd_set SocksList; /* SocksList file descriptor list */
	fd_set ReadSocks; /* temp file descriptor list for select() */
	
	/* Max Socket number and Socket index */
	int MaxSockNumber, SockIndex;
	unsigned int NumConnect = 0; /* Current number of connects */
	int ConnectTable[MAXCONNECTS]; /* Connect Table */
	int MessageLen; /* Message lenth */

	if (argc != 3)
	{
		fprintf(stderr, "USAGE:%s <port> <delay_echo_time>\n",argv[0]);
		exit(1);
	}

	/* Inintail connect table */
	memset(ConnectTable, 0, sizeof(int)*MAXCONNECTS);

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

	/* Offset ServerSock to zero index */
	ConnectTable[ServerSock - ServerSock] = 1;

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
	fprintf(stdout, "The time delay ehco is %d secs\n",atoi(argv[2]));

	/* Until Server close */
	while (1)
	{
		ReadSocks = SocksList; /* For temp */
		
#if DEBUG
		fprintf(stdout, "select\n");
		PAUSE
#endif

		/* select */
		/* Check which filedescriptor need to do I/O */
		if (select(MaxSockNumber+1, &ReadSocks, NULL, NULL, NULL) < 0)
		{
			Die("select");
		}

		/* Run through the exising connections */
		for (SockIndex = ServerSock; SockIndex < (MaxSockNumber+1); SockIndex++)
		{
#if DEBUG
			fprintf(stdout, "SockIndex=%d\n",SockIndex);
			PAUSE
#endif
			if (FD_ISSET(SockIndex, &ReadSocks)) /* Check which one need to do I/O */
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
#if DEBUG
					fprintf(stdout, "SourceSock=%d\n",SourceSock);
					PAUSE
#endif

					/* Check connect number */
					if (NumConnect > MAXCONNECTS - 1)
					{
						sprintf(MessageBuffer,"Connect %d/%d Overflow\n", NumConnect + 1, MAXCONNECTS);
						
						MessageLen = strlen(MessageBuffer) ;

						/* Send Back to source */
						if (send(SourceSock, MessageBuffer, MessageLen, 0) != MessageLen)
						{
							Die("Failed to send bytes to client");
						}

						close(SourceSock);
						continue;
					}

					/* Keep track of the max */
					if (SourceSock > MaxSockNumber)
					{
						MaxSockNumber = SourceSock;
					}

					/* Add New Sock to SocksList list */
					FD_SET(SourceSock, &SocksList);

					/* Setting status to connect */
					ConnectTable[SourceSock - ServerSock] = 1;

					NumConnect++; /* Number of Current Connect */

					sprintf(MessageBuffer, "Sock%d from connected: %s\n\
Current Connects=%d\n", SourceSock, inet_ntoa(SourceAddrInfo.sin_addr), NumConnect);

					/* Handle exising connects */
					HandleClient(SockIndex, MessageBuffer, ConnectTable, ServerSock,
								 MaxSockNumber, atoi(argv[2]));
#if DEBUG
					fprintf(stdout, "MaxSockNumber=%d\n",MaxSockNumber);
					fprintf(stdout, "Watting\n");
					PAUSE
#endif
				}
				/* Handle exising connections */
				else
				{
					/* Received data from client */
					if ((ReceivedDataBytes = recv(SockIndex, MessageBuffer, BUFFSIZE, 0)) < 1)
					{
						/* Disconnect */
#if DEBUG
						fprintf(stdout, "Disconnect\n");
#endif				
						NumConnect--;

						sprintf(MessageBuffer, "Sock%d Disconnect\n\
Current Connects=%d\n",SockIndex,NumConnect);

						close(SockIndex);

						/* Remove Socket in SocksList */
						FD_CLR(SockIndex, &SocksList);

						/* Setting status to disconnect */
						ConnectTable[SockIndex - ServerSock] = 0;

						if (SockIndex >= MaxSockNumber)
						{
							MaxSockNumber = MaxSockNumber - 1;
						}
					}
					else
					{
						/* Setting Ending char. */
						MessageBuffer[ReceivedDataBytes] = '\0';
					}

					/* Handle data from a client */
					HandleClient(SockIndex, MessageBuffer, ConnectTable, ServerSock, 
								 MaxSockNumber, atoi(argv[2]));
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
