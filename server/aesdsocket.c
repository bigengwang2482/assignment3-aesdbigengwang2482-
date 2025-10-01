// This is a file for using socket server written in C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


int main(int argc, char* argv[]) {
	// Initialize the socket
	int server_fd; // the server socket file descriptor

	server_fd = socket(PF_INET, SOCK_STREAM, 0);	

	// Use getaddrinfo to setup the addrinfo
	struct addrinfo* hints;
	hints = (struct addrinfo*) malloc(sizeof(struct addrinfo));
	hints->ai_flags = AI_PASSIVE;
	const char* node;
	node = NULL;
	const char service[] = "9000";
	
	struct addrinfo* res;
	getaddrinfo(node, service, hints, &res); // WARNING, malloc happens inside for res
	
	// Binding here: now use the results from getaddrinfo to serve as the socket address	
	bind(server_fd, res->ai_addr,sizeof(struct sockaddr));
	freeaddrinfo(res); // WARNING, must have it here to free res
	free(hints);

	// Start listening
	listen(server_fd, 10); //temporarily set 10 as the backlog number

	// Try accept and set up the connection
	struct sockaddr client_addr;	
	socklen_t addrlen = sizeof(struct sockaddr);
	int acceptedfd; // TODO: return -1 if any of the connect steps fail
	acceptedfd = accept(server_fd, &client_addr, &addrlen); // Use accpt_fd to read and write for our socket	
	// TODO: Logs message to the syslog

	// Once the connection is done, do recv and send using acceptedfd
	// use /var/tmp/aesdsocketdata as the buffer

	FILE* file = fopen("/var/tmp/aesdsocketdata", "a+");// use append mode

	if (file == NULL) {
		perror("fopen failed");
		return 1;
	}
		
	size_t buffer_len=15;
	char* bytes_buffer = (char*) malloc(sizeof(char)*buffer_len);
	recv(acceptedfd, bytes_buffer, buffer_len, 0);
	send(acceptedfd, bytes_buffer, buffer_len, 0);
	

	fclose(file);	
	return 0;
}
