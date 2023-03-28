/* time_client.c - main */

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>                                                                            
#include <netinet/in.h>
#include <arpa/inet.h>
                                                                                
#include <netdb.h>

#define	BUFSIZE 64

#define	MSG		"Any Message \n"

/*------------------------------------------------------------------------
 * main - UDP client for TIME service that prints the resulting time
 *------------------------------------------------------------------------
 */
int
main(int argc, char **argv)
{
	char	*host = "localhost";
	int	port = 3000;
	char	now[100];		/* 32-bit integer to hold time	*/ 
	struct hostent	*phe;	/* pointer to host information entry	*/
	struct sockaddr_in sin;	/* an Internet endpoint address		*/
	int	s, n, type;	/* socket descriptor and socket type	*/

	switch (argc) {
	case 1:
		break;
	case 2:
		host = argv[1];
	case 3:
		host = argv[1];
		port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "usage: UDPtime [host [port]]\n");
		exit(1);
	}

	memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;                                                                
        sin.sin_port = htons(port);
                                                                                        
    /* Map host name to IP address, allowing for dotted decimal */
        if ( phe = gethostbyname(host) ){
                memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
        }
        else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
		fprintf(stderr, "Can't get host entry \n");
                                                                                
    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "Can't create socket \n");
	
                                                                                
    /* Connect the socket */
        if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "Can't connect to %s %s \n", host, "Time");

	// ---------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------

	#define BBLU "\e[1;34m"
	#define GRN "\e[1;32m"
	#define RED "\e[1;31m"
	#define reset "\e[0m"

	typedef struct pdu {
		char type;
		char data[100];
	} PDU;
	
	int input;
	char buffer[20];
	FILE *fptr;

	/* Send IP Address to Server */
	(void) write(s, MSG, strlen(MSG));

	/* Read the Time */
	n = read(s, (char *)&now, sizeof(now));
	if (n < 0)
	{
		fprintf(stderr, "Read failed\n");
	}
	else
	{
		write(1, now, n);
	}

	/* Loop */
	while (1)
	{
		/* User Interface */
		printf("\n%sUDP File Download Service%s\n", BBLU, reset);
		printf("[0] Download File\n");
		printf("[1] Exit Program\n\n");
		printf("%sEnter Input:%s ", GRN , reset);
		scanf("%d", &input);

		/* Manage Input */
		switch (input)
		{
			case 0:
				printf("%sEnter File Name: %s", GRN, reset);
				scanf("%s", buffer);
				fptr = fopen("download.txt", "w");
				break;

			case 1:
				printf("%sTerminating Program%s\n\n", RED, reset);
				exit(0);
				break;
		}


		/* Send Data to Server*/
		PDU *data = (PDU*)malloc(sizeof(PDU));	
		strcpy(data->data, buffer);
		data->data[strlen(data->data)] = '\0'; 
		data->type = 'C';
		(void) write(s, data, (int)strlen(data->data)+1);

		/* Holds Received Packet */
		PDU try;
		try.type = 'Z';

		/* Get Data and Write to File*/
		while (try.type != 'F')
		{	
			/* Empty */
			for (int i=0; i<100; i++)
			{
				try.data[i] = '\0';
			}

			/* Get */
			n = recv(s, &try, sizeof(try.data)+1, 0);
			try.data[n] = '\n';

			/* Error */
			if (try.type == 'E')
			{
				printf("%s", try.data);
				printf("%sTerminating Program%s\n\n", RED, reset);
				exit(0);
			}

			/* Write */
			fprintf(fptr, "%s", try.data);
			fflush(fptr);
		}

		/* Output */
		printf("File Sucessfuly Downloaded\n");
	}

	exit(0);
}
