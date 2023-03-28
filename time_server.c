/* time_server.c - main */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>


/*------------------------------------------------------------------------
 * main - Iterative UDP server for TIME service
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	struct  sockaddr_in fsin;	/* the from address of a client	*/
	char	buf[100];		/* "input" buffer; any size > 0	*/
	char    *pts;
	int	sock;			/* server socket		*/
	time_t	now;			/* current time			*/
	int	alen;			/* from-address length		*/
	struct  sockaddr_in sin; /* an Internet endpoint address         */
        int     s, type;        /* socket descriptor and socket type    */
	int 	port=3000;
                                                                                

	switch(argc){
		case 1:
			break;
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
	}

        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port);
                                                                                                 
    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "can't creat socket\n");
                                                                                
    /* Bind the socket */
        if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "can't bind to %d port\n",port);
        listen(s, 5);	
	alen = sizeof(fsin);

	// ---------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------

	typedef struct pdu {
		char type;
		char data[100];
	} PDU;

	FILE *fptr;

	while (1)
	{
		/* Initialize */
		PDU data;

		/* Get Message and IP Address of Sender*/
		if (recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&fsin, &alen) < 0)
		{
			fprintf(stderr, "recvfrom error\n");
		}
		
		/* Get Time */
		(void) time(&now);
        pts = ctime(&now);

		/* Send to Client */
		(void) sendto(s, pts, strlen(pts), 0, (struct sockaddr *)&fsin, sizeof(fsin));

		/* Get Data */
		if (recvfrom(s, &data, sizeof(data.data)+1, 0, (struct sockaddr *)&fsin, &alen) < 0)
		{
			fprintf(stderr, "recvfrom error\n");
		}
		
		/* Debugging */
		printf("filename: %s\n", data.data);
		fflush(stdout);
		
		/* Open File */
		fptr = fopen(data.data, "r");
		if (fptr == NULL)
		{
			/* Send to Client */
			strcpy(data.data, "error opening file \n\0");
			data.type = 'E';
			(void) sendto(s, &data, (int)strlen(data.data)+1, 0, (struct sockaddr *)&fsin, sizeof(fsin));

			/* Debugging */
			printf("error opening file\n");
		}
		else
		{
			/* Debugging */
			printf("file opened\n");
		}

		/* Get File Size */
		fseek(fptr, 0L, SEEK_END);
		int remaining_data = ftell(fptr);
		rewind(fptr);

		/* Debugging */
		printf("filelength: %d\n", remaining_data);
		fflush(stdout);

		/* Send Data */
		while (remaining_data > 0)
		{
			/* Empty */
			for (int i=0; i<100; i++)
			{
				buf[i] = '\0';
				data.data[i] = '\0';
			}

			/* Get One Packet of Data at a Time */
			for (int i=0; i<100; i++)
			{
				char c = fgetc(fptr);
				if (c == EOF)
				{
					break;
				}
				buf[i] = c;
				remaining_data--;
			}

			/* End of File Reached */
			if (remaining_data == 0)
			{
				data.type = 'F';
			}
			else
			{
				data.type = 'D';
			}

			/* Send Packet */
			strcpy(data.data, buf);
			(void) sendto(s, &data, (int)strlen(data.data)+1, 0, (struct sockaddr *)&fsin, sizeof(fsin));

			/* Debugging */
			printf("%s", buf);
			fflush(stdout);
		}
	}
}
