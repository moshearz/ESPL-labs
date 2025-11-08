#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/wait.h>
#include "LineParser.h"

//#include "linux/limits.h"
#include <signal.h>
#include <fcntl.h>

//globals
int debug_mode = 0; //add supurting for the parameter -d (task 1a)
#define BUFFER_SIZE 2048


void execute(cmdLine* pCmdLine){
    pid_t pid = fork(); // creates a new process (Fork actually duplicates a running process)

    if (pid == -1){ // fork == -1 means that the fork failed
        perror("fork failed");
        exit(1); // In all other cases (not child) we need to use exit (and not _exit)
    }

    if (pid == 0){//To the new process, the child, fork returns 0. (pid == 0 means that we are in the CHILD process)

        // Handle input/output redirection (task 3: Redirection)
        //Input
        if(pCmdLine->inputRedirect != NULL){ //check if the user insert : "<" in the command line (with the name of the file to read from) (if dont -> command will be executed with the default input (the keyboard))
            close(STDIN_FILENO); // the default standard input file descriptor number which is 0. (its 0 because the first file descriptor is 0 (stdin) and the second is 1 (stdout) and the third is 2 (stderr))
            // now the command will read from the file instead of the standard input (the keyboard) . "cat < in.txt" will read from the file in.txt instead of the keyboard
            if (open(pCmdLine->inputRedirect, O_RDONLY) == -1 ) { // O_RDONLY -> open the file in read only mode. open()  returns the first unused file decscroptor number.generally 3 because 0, 1, 2 are used for stdin, stdout and stderr, but in our case we close stdin so it will be 0.
                perror("open failed");
                _exit(1); // abort the child program when the open fails
            }
        }

        //Output
        if(pCmdLine->outputRedirect != NULL){ //check if the user insert : ">" in the command line (with the name of the file to write to) (if dont -> command will be executed with the default output (the screen))
            close(STDOUT_FILENO); // close the default standard output file descriptor number which is 1. 
            if(open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0666) == -1){ // O_WRONLY = READ ONLY, O_CREAT = CREATE THE FILE IF IT DOES NOT EXIST, O_TRUNC = IF THE FILE EXIST RESET IT, 0666 = PREMISSONS FOR READ & WRITE FOR EVERYONE
                perror("open failed");
                _exit(1); 
            }
        }

        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) { //execvp swaps the current process with a new process. Activates the command
            perror("execvp failed");
            _exit(1); //abort the child program when the exec fails
        }
        
    }


    
    else { // pid > 0 means that we are in the PARENT process
        if (debug_mode)  //"with the "-d" flag, your shell will also print the debug output to stderr"
            fprintf(stderr, "pid: %d\nExecuting command: %s\n", pid, pCmdLine->arguments[0]);
            
        // if the command blocks (without &), wait fot the child to finish (and in the command in the background, let it run and the shell will continue)
        // 1c -> use waitpid() only if the command doees NOT end with '&' (blocking == 1)
        if (pCmdLine->blocking) waitpid(pid, NULL, 0);
        
    } 
}

int main(int argc, char* argv[]) {
    char buffer[PATH_MAX];
    char input[BUFFER_SIZE];
    
    //When executed with the "-d" flag, your shell will also print the debug output to stderr
    // we will check if the -d flag is in the argv[i]
    if (argc > 1)
        for (int i = 0; i < argc; i++) 
            if (strcmp(argv[i], "-d") == 0) 
                debug_mode = 1;
    
    while (1) {
        if (getcwd(buffer, PATH_MAX) != NULL) {
            printf("%s> ", buffer);
        } else {
            perror("getcwd failed");
            exit(1);
        }

        if (fgets(input, BUFFER_SIZE, stdin) == NULL){
            perror("fgets failed");
            exit(1);
        }

        input[strcspn(input, "\n")] = '\0';

        // we do this line before parsing the input, so we can quit the shell without parsing it so no need to free the cmdLine and we use break to exit the loop and then the return 0 will exit the program (instead use exit(0))
        if(strcmp(input, "quit") == 0) break; 

        //parseCmdLines pass the input string and return a pointer to the cmdLine struct (which is a linked list of cmdLine structs (THE STRUCT CONTAINS FILED "next" so its component of linked list)) that contains the parsed command line
        cmdLine* parsedCmd = parseCmdLines(input); 

        if (parsedCmd == NULL) continue; // if the input is empty(the user prees just enter so "input" will be empty string or just spaces), we will continue to the next iteration of the loop

        // // identify the command: cd (task 1b)
        if (strcmp(parsedCmd->arguments[0], "cd") == 0) {
            //checks if the user did not provide a path to cd (a cd to move to)
            if(parsedCmd->argCount < 2){
                fprintf(stderr, "cd: missising argument ");
            } else if (chdir(parsedCmd->arguments[1]) != 0) { //chdir() tries to change the argument (folder) of the current process (the shell) acording to the path the user provided.
                perror("cd failed");
            }
            freeCmdLines(parsedCmd); // avoid memory leak because we are not going to execute the command and skip freeCmdLines at the end of the loop
            continue;
        };

        // 2 - signals
        if (strcmp(parsedCmd->arguments[0], "halt") == 0 || strcmp(parsedCmd->arguments[0], "wakeup") == 0 || strcmp(parsedCmd->arguments[0], "ice") == 0) {
            
            if(parsedCmd->argCount < 2){
                fprintf(stderr, "%s: missising argument ", parsedCmd->arguments[0]);
                freeCmdLines(parsedCmd); // avoid memory leak because we are not going to execute the command and skip freeCmdLines at the end of the loop
                continue; // exit the loop and continue to the next iteration
            }

             int target_pid = atoi(parsedCmd->arguments[1]); // convert: string to int
             int signal_num = 0;

             if (strcmp(parsedCmd->arguments[0], "halt") == 0) {
                signal_num = SIGSTOP; // halt <process id> - Send a SIGSTOP signal to a process to make it "sleep".
             } else if (strcmp(parsedCmd->arguments[0], "wakeup") == 0) {
                signal_num = SIGCONT; // wakeup <process id> - Wake up a sleeping (stopped) process (SIGCONT)
             } else if (strcmp(parsedCmd->arguments[0], "ice") == 0) {
                signal_num = SIGINT; // ice <process id> - Terminate a running/sleeping process (SIGINT).
             }

            if (kill(target_pid, signal_num) == -1) {perror("kill failed");} // "In all cases, use the kill( ) system call wrapper, see man 2 kill, to send the relevant signal to the given process id." 
                
            freeCmdLines(parsedCmd); // avoid memory leak because we are not going to execute the command and skip freeCmdLines at the end of the loop
            continue; // exit the loop and continue to the next iteration
        }
            
        execute(parsedCmd);

        freeCmdLines(parsedCmd);

    }

    return 0;
    
}