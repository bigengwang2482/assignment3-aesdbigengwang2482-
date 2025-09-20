#include "systemcalls.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    int res;
    res = system(cmd);

    if (res == -1) {
	return false;
    }
    return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/
    pid_t pid = fork(); // Use fork to create a child process, with pid holding the status
    if (pid == -1) {
	    perror("fork failed");
	    exit(1);
    }

	int status;
    // execute for child or wait for parent
    if (pid == 0) {
	    // I'm the child
	    execv(command[0], command);
	    perror("execv failed."); 
	    exit(1);
    } else {
	    // I'ms the parent
	    int child_pid = wait(&status); // get info about child status	
		printf("Heard back from child pid=%d with status %d, if exited normally? %d\n", child_pid, WEXITSTATUS(status), WIFEXITED(status));
		if (WIFEXITED(status)) { // true if exited normally
			// If exited normally
			if (WEXITSTATUS(status) == 0) {
				printf("The child procss executed succesfully. \n");
				return true;
			} else {
				printf("The child process exited with a non-zero status: %d \n", WEXITSTATUS(status));
				return false;
			}	
		} else {
			printf("The child process is not returning properly. \n");
			if (WIFSIGNALED(status)) {
				printf("The child process is terminated by signal: %d. \n", WIFSIGNALED(status));	
			}
			return false;
		}
    }

    va_end(args);

    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/ 
	// Use the last command to hold the redirect info
	//const int tail_size = 100;
	//printf("DEBUGGING: outputfile is: %s \n", outputfile);	
	//command[count] = (char *)malloc(tail_size * sizeof(char)); // alloc mem first
	//sprintf(command[count], " > %s", outputfile);
	//printf("DEBUGGING: out command is %s \n", command[count]);

	// Create redirected file
	int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);	
	if (fd < 0) { perror("open"); abort(); } // abort if failed openning.

 
	pid_t pid = fork(); // Use fork to create a child process, with pid holding the status
    if (pid == -1) {
	    perror("fork failed");
	    exit(1);
    }

	int status;
    // execute for child or wait for parent
    if (pid == 0) {
	    // I'm the child
		if (dup2(fd, 1) < 0) { perror("dup2"); abort(); } // redirect stdout (1) to the file
		close(fd); // Close the original file descriptor as it's no longer needed	
	    execv(command[0], command);
	    perror("execv failed."); 
	    exit(1);
    } else {
	    // I'ms the parent
		close(fd); // Close the original file descriptor as it's no longer needed
	    int child_pid = wait(&status); // get info about child status	
		printf("Heard back from child pid=%d with status %d, if exited normally? %d\n", child_pid, WEXITSTATUS(status), WIFEXITED(status));
		if (WIFEXITED(status)) { // true if exited normally
			// If exited normally
			if (WEXITSTATUS(status) == 0) {
				printf("The child procss executed succesfully. \n");
				return true;
			} else {
				printf("The child process exited with a non-zero status: %d \n", WEXITSTATUS(status));
				return false;
			}	
		} else {
			printf("The child process is not returning properly. \n");
			if (WIFSIGNALED(status)) {
				printf("The child process is terminated by signal: %d. \n", WIFSIGNALED(status));	
			}
			return false;
		}
    } 

    va_end(args);

    return true;
}
