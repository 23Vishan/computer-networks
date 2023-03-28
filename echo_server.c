/* A simple echo server using TCP */
#include <stdio.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SERVER_TCP_PORT 3000 // well-known port
#define BUFLEN 256           // buffer length

int echod(int);
void reaper(int);

int main(int argc, char **argv)
{
	int sd, new_sd, client_len, port;
	struct sockaddr_in server, client;

	// get arguments
	switch(argc)
	{
		case 1:
			port = SERVER_TCP_PORT;
			break;
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
	}

	// create a socket that uses TCP
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	// assign an IP address and port to the socket
	// places socket in server (passive) mode
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	// queue up a maximum of 5 TCP requests
	listen(sd, 5);

	(void)signal(SIGCHLD, reaper);

	while(1)
	{
		// accept an incoming connection request
		// blocks return until a TCP connection request by the client is received
		// when it arrives it returns a new socket descriptor
		client_len = sizeof(client);
		new_sd = accept(sd, (struct sockaddr *)&client, &client_len);

		// TCP connection not established
		if(new_sd < 0)
		{
			fprintf(stderr, "Can't accept client \n");
			exit(1);
		}

		switch (fork())
		{
			// child
			case 0:
				(void) close(sd);    // closes old socket
				exit(echod(new_sd)); // uses new socket to communicate with the client

			// parent
			default:
				// close new socket and listen with old socket for more connection requests
				(void) close(new_sd);
				break;

			// error
			case -1:
				fprintf(stderr, "fork: error\n");
		}
	}
}

// sends a message to the client
int echod(int sd)
{
	char header[5];
	int header_size = 5;
	FILE *file;
	char buf[BUFLEN] = "";
	int message_length;
	int fptr;
	int char_count;
	int packet_size = 100;

	 // get filename
	message_length = read(sd, buf, BUFLEN);

	// variable to hold name
	char file_name[message_length];
	strcpy(file_name, buf);

	// remove '\n' from input
	file_name[message_length-1] = '\0';

	// open
	file = fopen(file_name, "r");

	// error
	if (file == NULL)
	{
		printf("could not open\n");
		strcpy(buf, "10110110error opening file\n");

		// send header
		sprintf(header, "%d", (int)strlen(buf));
		write(sd, header, header_size);
		printf("HEADER: %s\n", header);

		// send data
		write(sd, buf, strlen(buf));

		close(sd);
		return(0);
	}

	// calculate file size
	fseek(file, 0L, SEEK_END);    // go to end of file
	int file_size = ftell(file);  // get location (size)
	rewind(file); 				  // go back to beginning
	sprintf(buf, "%d", file_size);// convert to string

	// send header
	sprintf(header, "%d", (int)strlen(buf));
	write(sd, header, header_size);
	printf("HEADER: %s\n", header);

	// send data
	write(sd, buf, header_size);
	printf("SIZE: %d\n", file_size);

	int remaining_data = file_size;
	char packet[100];

	while (remaining_data > 0)
	{
		for (int i=0; i<packet_size; i++)
		{
			char c = fgetc(file);
			if (c == EOF)
			{
				break;
			}
			packet[i] = c;
		}

		// send header
		sprintf(header, "%d", (int)strlen(packet));
		write(sd, header, header_size);

		// send data
		remaining_data -= write(sd, packet, (int)strlen(packet));
	}
	
	// close socket
	close(sd);

	return(0);
}

// reaper?
void reaper(int sig)
{
	int	status;
	while(wait3(&status, WNOHANG, (struct rusage *)0) >= 0);
}
