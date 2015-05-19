#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>

#define BUFFSIZE 255

/* Sock Error */
void Die(char *Message)
{
	perror(Message);
	exit(1);
}

int main(int argc, char *argv[])
{
	int ServerSock;	/* Source socket */
	struct sockaddr_in SourceAddrInfo;	/* ServerSock address information */
	char MessageBuffer[BUFFSIZE];	/* Send or Received internet message buffer */
	
	/* Size of Server & Client Address information */
	unsigned int SourceAddrInfoSize;
	
	int ReceivedDataBytes = 0;	/* Size of received MessageBuffer */
	
	time_t ServerTime; /* Server time */

	if (argc != 3)
	{
		fprintf(stderr, "USAGE: %s <Multicast_IP> <PORT>\n", argv[0]);
		exit(1);
	}

	if (atoi(argv[1]) < 224 || atoi(argv[1]) > 239)
	{
		fprintf(stderr, "The Multicast_IP Range is:\n");
		fprintf(stderr, "224.0.0.0~239.255.255.255\n");
		exit(1);
	}

	/* Create the UDP socket */
	if ((ServerSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		Die("Failed to create socket");
	}

	/* Construct the server sockaddr_in structure */
	memset(&SourceAddrInfo, 0, sizeof(SourceAddrInfo));	/*Clear struct */
	SourceAddrInfo.sin_family = AF_INET;				/* Internet/IP */
	SourceAddrInfo.sin_addr.s_addr = inet_addr(argv[1]);/* IP address */
	SourceAddrInfo.sin_port = htons(atoi(argv[2]));		/* Source port */
	
	fprintf(stdout, "Send the server time to multicast group per 5 secs.\n");

	SourceAddrInfoSize = sizeof(SourceAddrInfo);

	/* Run until cancelled */
	while (1)
	{
		ServerTime = time(0); /* Get current time */

		sprintf(MessageBuffer, "%-24.24s", ctime(&ServerTime));
		
		fprintf(stdout, "Sending Time: %s\n",MessageBuffer);

		/* Send to Group */
		if ((ReceivedDataBytes =
			sendto(ServerSock, MessageBuffer, BUFFSIZE, 0, (struct sockaddr *) &SourceAddrInfo,
				   SourceAddrInfoSize) < 0))
		{
			Die("Failed sending Message");
		}
		sleep(5);
	}
}
