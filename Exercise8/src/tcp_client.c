#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h> /* hostent struct, gethostbyname() */
#include <arpa/inet.h> /* inet_ntoa() to format IP address */
#include <netinet/in.h> /* in_addr structure */

#define BUFFSIZE 1024 /* Max message buffer size */
#define HOSTNAME "checkip.dyndns.org" /* Target Host ID */
#define HTTPPORT 80 /* HTTP Port */

/* Find Word in string */
int FindWord(const char *String, char *Pattern, unsigned int PatternSize)
{
	/* Char index & PatternSize loop index */
	int CharIndex = 0, LoopIndex;
	int Match = 0; /* Match? */

	/* Until pattern match */
	while (!Match)
	{
		/* In the end */
		if (String[CharIndex] == '\0')
		{
			/* Failed */
			fprintf(stderr, "Failed to find %s\n", Pattern) ;
			return -1;
		}
		else
		{
			/* compare char. by char. */
			for (LoopIndex = 0; LoopIndex < PatternSize; LoopIndex++)
			{
				Match = 1;

				/* dismatch */
				if (String[CharIndex+LoopIndex] != Pattern[LoopIndex])
				{
					Match = 0;
				}

				if (!Match)
				{
					break;
				}
			}
		}

		CharIndex++;
	}

	/* Return Char index before target position */
	return (CharIndex = CharIndex - 1);
}

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
	unsigned int MessageBufferSize;	/* Size of MessageBufferSize */
	int ReceivedDataBytes;	/* Size of received MessageBuffer */
	const char HostNameID[] = HOSTNAME; /* Host Name */

	struct hostent *HostInfo; /* host */
	char PublicIP[BUFFSIZE]; /* PublicIP */

	/* Get Host IP */
	if ((HostInfo = gethostbyname(HostNameID)) == NULL)
	{
		Die("Failed to get host");
	}

	/* Create the TCP socket */
	if ((ServerSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		Die("Failed to create socket");
	}

	/* Construct the server sockaddr_in structure */
	memset(&ServerAddrInfo, 0, sizeof(ServerAddrInfo)); /* Clear */
	ServerAddrInfo.sin_family = AF_INET;				/* Internet */
	ServerAddrInfo.sin_addr.s_addr = *((unsigned long *)HostInfo->h_addr_list[0]); /* Host IP */
	ServerAddrInfo.sin_port = htons(HTTPPORT); 			/* HTTP Port 80 */

	/* Establish connect */
	if (connect(ServerSock, (struct sockaddr *) &ServerAddrInfo, sizeof(ServerAddrInfo)) < 0)
	{
		Die("Failed to connect with server");
	}

	/* Sending some thing to the server */
	sprintf(MessageBuffer,"\n");

	MessageBufferSize = strlen(MessageBuffer);

	fprintf(stdout, "Send to %s, Bytes:%d\n",inet_ntoa(ServerAddrInfo.sin_addr), MessageBufferSize);

	if (send(ServerSock, MessageBuffer, MessageBufferSize, 0) != MessageBufferSize)
	{
		Die("MisMatch in number of sent bytes");
	}
	
	/* Recived data from server */
	if ((ReceivedDataBytes = recv(ServerSock, MessageBuffer, BUFFSIZE, 0)) < 0)
	{
		Die("Failed to receive initial bytes from server");
	}

	fprintf(stdout,"Received:\n%s\n",MessageBuffer);

	/* Find public IP position in string */
	int BeginePosition, EndPosition;

	/* Position=FindWord(const char *String, char *Pattern, unsigned int PatternSize)  */
	BeginePosition = FindWord(MessageBuffer,"IP Address: ",12);
	EndPosition = FindWord(&MessageBuffer[BeginePosition],"<",1);

	memset(PublicIP,0,BUFFSIZE); /* Clear memory */

	/* Copy Public IP */
	memcpy(PublicIP,&MessageBuffer[BeginePosition+12],EndPosition-12);
	
	fprintf(stdout, "Public IP: %s\n",PublicIP);

	return 0;
}
