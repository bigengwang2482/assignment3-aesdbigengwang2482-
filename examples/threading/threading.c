#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
	struct thread_data* thread_func_args = (struct thread_data *) thread_param; // cast input thread_param pointer to a thread_data type
	wait(thread_func_args->wait_to_obtain_ms);
	int rc = pthread_mutex_lock(thread_func_args->mutex);
	if (rc != 0) {
		printf("pthread_mutex_lock failed with %d\n", rc);
		thread_func_args->thread_complete_success = false;
	} else {
		wait(thread_func_args->wait_to_release_ms);
		rc = pthread_mutex_unlock(thread_func_args->mutex);
		if (rc != 0 ) {
			printf("pthread_mutex_unlock failed with %d\n", rc);
			thread_func_args->thread_complete_success = false;
		} else {
			thread_func_args->thread_complete_success = true;
		}
	}	
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
	// Step 1 allocate memory for thread_data
	struct thread_data* thrd_data = (struct thread_data*) malloc(sizeof(struct thread_data));
	// Step 2 setup mutex and wait arguments
	thrd_data->mutex = mutex;
	thrd_data->wait_to_obtain_ms = wait_to_obtain_ms;
	thrd_data->wait_to_release_ms = wait_to_release_ms;
	// pass thread_data to created thread using threadfunc() as entry point.
	pthread_create(thread, NULL, threadfunc, thrd_data);
	if (thrd_data->thread_complete_success) {
		return true;
	}
	// return true if successful

    return false;
}

