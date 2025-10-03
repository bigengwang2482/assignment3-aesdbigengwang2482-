// This is a file for using socket server written in C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>


FILE* file;
char* bytes_buffer;
void signal_handler(int sig) {
	if ((sig == SIGINT) || (sig == SIGTERM) ) {
		syslog(LOG_DEBUG, "Caught signal, exiting");
		if (bytes_buffer != NULL) {
			free(bytes_buffer);
		}	
		remove("/var/tmp/aesdsocketdata");	
		exit(0);
	}

}

int main(int argc, char* argv[]) {
	// Register a signal handler
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		perror("signal");
		exit(1);
	}
	if (signal(SIGTERM, signal_handler) == SIG_ERR) {
		perror("signal");
		exit(1);
	}

	// Check if runnig daemon mode
	int do_daemon = 0;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-d") == 0 ) {
			do_daemon = 1;
		}
	}

	// Set up the syslog
	openlog(NULL, 0, LOG_USER);

	// Initialize the socket
	int server_fd; // the server socket file descriptor

	server_fd = socket(PF_INET, SOCK_STREAM, 0);	

	// Use getaddrinfo to setup the addrinfo	
	struct addrinfo* res;
	struct addrinfo* hints;

	hints = (struct addrinfo*) malloc(sizeof(struct addrinfo));
	hints->ai_flags = AI_PASSIVE;
	const char* node;
	node = NULL;
	const char service[] = "9000";
	
	
	getaddrinfo(node, service, hints, &res); // WARNING, malloc happens inside for res
	free(hints);	
	// Binding here: now use the results from getaddrinfo to serve as the socket address	
	bind(server_fd, res->ai_addr,sizeof(struct sockaddr));
	freeaddrinfo(res); // WARNING, must have it here to free res	

	if (do_daemon) {
		pid_t pid;


		// Create a new process for daemon
		pid = fork();

		if (pid < 0) {
			perror("fork");
			exit(1);
		}

		// parent exit
		if (pid > 0) {
			printf("Daemonized process with PID %d\n", pid);
			exit(0);
		}

		// child proceeds here and  become the session leader
		if (setsid() < 0) {
			perror("setsid");
			exit(1);
		}

	}
	
	//
	// Start listening
	listen(server_fd, 10); //temporarily set 10 as the backlog number

	// Try accept and set up the connection
	struct sockaddr client_addr;	
	socklen_t addrlen = sizeof(struct sockaddr);

	while(1) {
		int acceptedfd; // TODO: return -1 if any of the connect steps fail
		acceptedfd = accept(server_fd, &client_addr, &addrlen); // Use accpt_fd to read and write for our socket
		if (acceptedfd < 0) {
			return -1;
		}	
		// TODO: Logs message to the syslog
		syslog(LOG_DEBUG, "Accepted connection from %s", client_addr.sa_data);
	
		// Once the connection is done, do recv and send using acceptedfd
		// use /var/tmp/aesdsocketdata as the buffer
	
		
			
		size_t buffer_len=1000000; // 1000000000; too large	
		bytes_buffer = (char*) malloc(sizeof(char)*buffer_len);
		recv(acceptedfd, bytes_buffer, buffer_len, 0);	
	
		// Find the new line break
		char* packet_head = bytes_buffer;
		char* line_break;	
		line_break = strchr(bytes_buffer, '\n');
		if (line_break == NULL) {
			//printf("No breakline found.\n");	
		}
		else {
			while (line_break != NULL) {
				size_t packet_size;
				packet_size = (line_break - packet_head) / sizeof(char) + 1;
				printf("Found breakline, Get packet_size %ld \n", packet_size);
				//line_break[0]='\0'; // Replace the breakline with null
				printf("Write %s to file \n", packet_head); 
				// write the packet to file
				file = fopen("/var/tmp/aesdsocketdata", "a+");// use append mode
			
				if (file == NULL) {
					perror("fopen failed");
					return 1;
				}	
				fprintf(file, "%s",packet_head);
				fclose(file);
				// update the packet_head and line break
				packet_head = line_break +  sizeof(char);
				line_break = strchr(packet_head, '\n');
			}
		}	
		
		// Load full content of /var/tmp/aesdsocketdata to client, and send back to client
		
		file = fopen("/var/tmp/aesdsocketdata", "rb");// use append mode	
		if (fseek(file,0, SEEK_END)	 != 0) {
			fclose(file);
			return -1;
		}
		
		long file_size = ftell(file);
		if (file_size == -1) {
			fclose(file);
			return -1;
		}
		buffer_len = file_size;
		if (bytes_buffer != NULL) {	
			free(bytes_buffer);
			bytes_buffer = NULL;	
		}
		bytes_buffer = (char*) malloc(sizeof(char) * buffer_len);
		fseek(file, 0, SEEK_SET);
		fread(bytes_buffer, sizeof(char), file_size, file);
		fclose(file);
						
		// Send the buffer to client
		send(acceptedfd, bytes_buffer, buffer_len, 0);	
		if (bytes_buffer != NULL) {	
			free(bytes_buffer);
			bytes_buffer = NULL;	
		}
		syslog(LOG_DEBUG, "Closed connection from %s", client_addr.sa_data);
	}	

	return 0;
}
