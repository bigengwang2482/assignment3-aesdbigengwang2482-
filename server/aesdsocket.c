// This is a file for using socket server written in C
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>



int main(int argc, char* argv[]) {
	// Initialize the socket
	int server_fd; // the server socket file descriptor

	server_fd = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr socket_addr;

	// Use getaddrinfo to setup the addrinfo
	struct addrinfo hints;
	addinfo_hints.ai_flags = AI_PASSIVE;
	const char* node;
	node = NULL;
	const char service[20]
	service="1234";
	struct addrinfo* res;
	getaddrinfo(node, service, &hints, &res); // WARNING, malloc happens inside for res
	
	// Binding here: now use the results from getaddrinfo to serve as the socket address	
	bind(server_fd, res->ai_addr,sizeof(struct sockaddr));
	freeaddrinfo(res); // WARNING, must have it here to free res


	// Start listening
	listen(server_fd, 10); //temporarily set 10 as the backlog number

	// Try accept and set up the connection
	struct addinfo client_addr;
	socklen_t addrlen = sizeof(struct sockaddr);
	int acceptedfd;
	acceptedfd = accept(server_fd, &client_addr, &addrlen); // Use accpt_fd to read and write for our socket	


	// Once the connection is done, do recv and send using acceptedfd
	size_t len;
	recv(acceptedfd, &buf, len, 0);
	send(acceptedfd, bytes_buffer, len, 0);
	

	return 0;
}
