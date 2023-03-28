/* A simple echo client using TCP */
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

#include <string.h>
#include <fcntl.h>

#define SERVER_TCP_PORT 3000	/* well-known port */
#define BUFLEN		256	/* buffer length */

int main(int argc, char **argv)
{
	int 	n, i, bytes_to_read;
	int 	sd, port;
	struct	hostent		*hp;
	struct	sockaddr_in server;
	char	*host, *bp, rbuf[BUFLEN], sbuf[BUFLEN];

	switch(argc){
	case 2:
		host = argv[1];
		port = SERVER_TCP_PORT;
		break;
	case 3:
		host = argv[1];
		port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
		exit(1);
	}

	/* Create a stream socket	*/	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (hp = gethostbyname(host)) 
	  bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	else if ( inet_aton(host, (struct in_addr *) &server.sin_addr) ){
	  fprintf(stderr, "Can't get server's address\n");
	  exit(1);
	}

	/* Connecting to the server */
	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
	  fprintf(stderr, "Can't connect \n");
	  exit(1);
	}
	
	/*----------------------------------------------------------------------------------*/
	/*-----------------------Communicating With The Server-----------------------------*/
	/*--------------------------------------------------------------------------------*/

	int length = -1;
	char buffer[BUFLEN];

	int packet_size = 100;

	int flag_size = 8;
	char error_flag[] = "10110110\0";

	int header_size = 5;
	int message_size = 0;

	// get message from terminal
	printf("Transmit: \n");
	length = read(0, buffer, BUFLEN);

	// send to server
	write(sd, buffer, length);

	// get header
	length = read(sd, buffer, header_size);
	message_size = atoi(buffer);
	printf("BUFFER: %d\n", message_size);

	// get data
	length = read(sd, buffer, message_size);

	// check for error flag
	char check[flag_size+1];
	strncpy(check, buffer, 8);
	check[8] = '\0';

	// exit if error
	if (strcmp(check, error_flag) == 0)
	{
		char *error_message = &buffer[8];
		printf("%s", error_message);

		close(sd);
		return(0);
	}

	// get file size
	int file_size = atoi(buffer);
	printf("FILE SIZE: %d\n", file_size);

	// read data
	int remaining_data = file_size;
	
	// create file
	FILE *fptr;
	fptr = fopen("downloaded_file.txt", "w");

	while (remaining_data > 0)
	{
		// get header
		length = read(sd, buffer, header_size);
		message_size = atoi(buffer);

		// get data and print to file
		remaining_data -= read(sd, buffer, message_size);
		fprintf(fptr, "%s", buffer);
	}

	close(sd);
	return(0);
}


/* 		bp = rbuf;
	bytes_to_read = n;
	while ((i = read(sd, bp, bytes_to_read)) > 0)
	{
		bp += i;
		bytes_to_read -=i;
	}

	write(1, rbuf, n);
	printf("Transmit: \n"); */