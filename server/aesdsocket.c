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
#include <stdbool.h>
#include <pthread.h>
#include "queue.h"  // Use implemented linked list

#define NUM_THREADS 10
#define TIME_STAMP_INTERVAL 10

// create SLIST (singly-linked list)
typedef struct slist_data_s slist_data_t;
struct slist_data_s {
	pthread_t thread_id;
	bool complete;
	SLIST_ENTRY(slist_data_s) entries;
};



//// SLIST
//
//slist_data_t *thread_stat_p;
//
//slist_data_s* slist_create_head() {
//
//	SLIST_HEAD(slisthead, slist_data_s) head;
//	SLIST_INIT(&head);	
//	return head;
//}
//
//slist_data_s* slist_add_data(slist_data_t head, slist_data_t* datap) {
//	
//	datap = malloc(sizeof(slist_data_t));
//	datap->complete = true;
//	SLIST_INSERT_HEAD(&head, datap, entries);
//	return head;
//}
//
//
//bool check_any_completed_threads(slist_data_t* datap, slist_data_t* head, slist_data_t* entries) {	
//	SLIST_FOREACH(datap, &head, entries) {
//		if (datap->complete == false) {
//			all_complete = false;
//		}
//	}
//	return all_complete;
//}
//
//void free_any_completed_threads(slist_data_t* datap, slist_data_t* head, slist_data_t* entries) {	
//	SLIST_FOREACH(datap, &head, entries) {
//		if (datap->complete == true) {	
//			pthread_join(datap->thread_id); // end and free the completed thread and don't let them hang there
//			// Better alternative to detached thread
//		}
//	}
//}

// Global variables
FILE* file;
char* bytes_buffer;
char* timer_buffer;

// Thread function for recv and send, wrapped here
/**
 * This structure should be dynamically allocated and passed as
 * an argument to your thread using pthread_create.
 * It should be returned by your thread so it can be freed by
 * the joiner thread.
 */
struct thread_data{
    /*
     * TODO: add other values your thread will need to manage
     * into this structure, use this structure to communicate
     * between the start_thread_obtaining_mutex function and
     * your thread implementation.
     */	
	pthread_mutex_t *mutex;
	int acceptedfd; 		
    /**
     * Set to true if the thread completed with success, false
     * if an error occurred.
     */
    bool thread_complete_success;
};

void* threadfunc(void* thread_param)
{

    //// TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    //// hint: use a cast like the one below to obtain thread arguments from your parameter
    ////struct thread_data* thread_func_args = (struct thread_data *) thread_param;
	struct thread_data* thread_func_args = (struct thread_data *) thread_param; // cast input thread_param pointer to a thread_data type
	//usleep(thread_func_args->wait_to_obtain_ms*1000); //usleep is for micro second not milisecond	
	////DEBUG_LOG("Waited %d ms BEFORE OBTAINING THE LOCKED MUTEX from arg. \n", thread_func_args->wait_to_obtain_ms);
	//	
	////	
	//usleep(thread_func_args->wait_to_release_ms*1000); 	
	// TODO: Logs message to the syslog
	//syslog(LOG_DEBUG, "Accepted connection from %s", client_addr.sa_data);
	
	// Once the connection is done, do recv and send using acceptedfd
	// use /var/tmp/aesdsocketdata as the buffer	
		
	size_t buffer_len=100000;// 1000000000; too large	
	bytes_buffer = (char*) malloc(sizeof(char)*buffer_len);
	recv(thread_func_args->acceptedfd, bytes_buffer, buffer_len, 0);	
	
	// Find the new line break
	char* packet_head = bytes_buffer;
	char* line_break;	
	line_break = strchr(bytes_buffer, '\n');
	int valid_packet = 0;
	if (line_break != NULL) {
		valid_packet = 1;	
	}
	// get the locked mutex from arg for unlock later
	pthread_mutex_t* thrd_mutex = thread_func_args->mutex;	
	pthread_mutex_lock(thrd_mutex); // perfrom mutex lock so other threads can't work
	if (valid_packet) {		
		line_break[1]='\0'; // Replace the breakline with null	
		// write the packet to file
		file = fopen("/var/tmp/aesdsocketdata", "a+");// use append mode	
		//if (file == NULL) {
		//	perror("fopen failed");
		//	return 1;
		//}	
		fprintf(file, "%s",packet_head);
		fclose(file);	
	}	
	pthread_mutex_unlock(thrd_mutex); // release mutex lock so other threads may work
	// Load full content of /var/tmp/aesdsocketdata to client, and send back to client
	
	file = fopen("/var/tmp/aesdsocketdata", "rb");// use append mode	
	if (fseek(file,0, SEEK_END)	 != 0) {
		fclose(file);
		//return -1;
	}
	
	long file_size = ftell(file);
	if (file_size == -1) {
		fclose(file);
		//return -1;
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
	send(thread_func_args->acceptedfd, bytes_buffer, buffer_len, 0);	
	if (bytes_buffer != NULL) {	
		free(bytes_buffer);
		bytes_buffer = NULL;	
	}
	//syslog(LOG_DEBUG, "Closed connection from %s", client_addr.sa_data);
	// Label the thread complete
    return thread_param;
}


struct timer_thread_data{
    /*
     * TODO: add other values your thread will need to manage
     * into this structure, use this structure to communicate
     * between the start_thread_obtaining_mutex function and
     * your thread implementation.
     */	
	pthread_mutex_t *mutex;	
	time_t* time_start; 
};

void* timer_threadfunc(void* thread_param)
{

    //// TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    //// hint: use a cast like the one below to obtain thread arguments from your parameter
    ////struct thread_data* thread_func_args = (struct thread_data *) thread_param;
	struct timer_thread_data* thread_func_args = (struct timer_thread_data *) thread_param; // cast input thread_param pointer to a thread_data type
	//usleep(thread_func_args->wait_to_obtain_ms*1000); //usleep is for micro second not milisecond	
	////DEBUG_LOG("Waited %d ms BEFORE OBTAINING THE LOCKED MUTEX from arg. \n", thread_func_args->wait_to_obtain_ms);
	//	
	////	
	//usleep(thread_func_args->wait_to_release_ms*1000); 	
	// TODO: Logs message to the syslog
	//syslog(LOG_DEBUG, "Accepted connection from %s", client_addr.sa_data);
	
	// Once the connection is done, do recv and send using acceptedfd
	// use /var/tmp/aesdsocketdata as the buffer	
	
	// get the locked mutex from arg for lock later
	pthread_mutex_t* thrd_mutex = thread_func_args->mutex;

	// start the part for recv and send
	size_t buffer_len=1024;// 1000000000; too large	
	timer_buffer = (char*) malloc(sizeof(char)*buffer_len);

	while (1) {		

		time_t now;
		time(&now);

		struct tm *current_time = localtime(&now);	

		if ((now - *(thread_func_args->time_start)) > TIME_STAMP_INTERVAL) { 
			pthread_mutex_lock(thrd_mutex); // perfrom mutex lock so other threads can't work
				
				
			// write the packet to file
			file = fopen("/var/tmp/aesdsocketdata", "a+");// use append mode	
			//if (file == NULL) {
			//	perror("fopen failed");
			//	return 1;
			//}	
			strftime(timer_buffer, buffer_len, "%Y-%m-%d %H:%M:%S\n", current_time);
			fprintf(file, "%s",timer_buffer);
			fclose(file);	
			
			pthread_mutex_unlock(thrd_mutex); // release mutex lock so other threads may work
		}
	}
	// Load full content of /var/tmp/aesdsocketdata to client, and send back to client
    return thread_param;
}

void signal_handler(int sig) {
	if ((sig == SIGINT) || (sig == SIGTERM) ) {
		syslog(LOG_DEBUG, "Caught signal, exiting");
		if (bytes_buffer != NULL) {
			free(bytes_buffer);
		}	
		if (timer_buffer != NULL) {
			free(timer_buffer);
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
	struct addrinfo hints;

	//hints = (struct addrinfo*) malloc(sizeof(struct addrinfo));
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	const char* node;
	node = NULL;
	const char service[] = "9000";
		
	getaddrinfo(node, service, &hints, &res); // WARNING, malloc happens inside for res
	//free(hints);	
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

	// now start more threads for dealing with recv and send	

	// Create a linked list of thread status
	slist_data_t *datap=NULL;
	SLIST_HEAD(slisthead, slist_data_s) head;
	SLIST_INIT(&head);
	// Set up the mutex	
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);
	while(1) {
		int acceptedfd; // TODO: return -1 if any of the connect steps fail
		acceptedfd = accept(server_fd, &client_addr, &addrlen); // Use accpt_fd to read and write for our socket
		if (acceptedfd < 0) {
			return -1;
		}	
		// Now a new connection has been set up
		// create a new list element to record the status
		datap = malloc(sizeof(slist_data_t));
		datap->complete=false; //initialize it to be not completed
		SLIST_INSERT_HEAD(&head, datap, entries);
		// Set up thread_data
		struct thread_data* thrd_data = (struct thread_data*) malloc(sizeof(struct thread_data));
		thrd_data->acceptedfd = acceptedfd;
		 
		thrd_data->mutex = &mutex;
		pthread_create(&(datap->thread_id), NULL, threadfunc, thrd_data); // start a new thread to do this recv and send
		
		// required infomation are in thrd_data which are passed to the threadfunc as the arguement
	
		// TODO: update this information in the thread status linked list
		// From main thread, check if any existing thread is done with their work so that they can be freed by pthread_join(...)
		SLIST_FOREACH(datap, &head, entries) {
			if (datap->complete) {
				pthread_join(datap->thread_id, NULL); // end the thread
				SLIST_REMOVE(&head, datap, slist_data_s, entries); // remove the thread from the linked list
				free(datap); // free the memory for the node
			}
		}
		
	}	

	return 0;
}
