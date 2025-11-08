#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/wait.h>
#include "LineParser.h"
#include <signal.h>
#include <fcntl.h>

//globals
int debug_mode = 0; //add supurting for the parameter -d (task 1a)
#define BUFFER_SIZE 2048

//part 4
#define HISTLEN 20 //"HISTLEN is a constant with a value of 20 as a default."

typedef struct historyNode{ // the history list is a linked list of historyNode structs
    char *command; // inside each node in the history list, we save the command (input from the user). (save before parseCmdLines).
    struct historyNode *next; // field to the next node (define it as a linked list! (connection between node to the next node)) 
} historyNode;

// gloabals part 4
historyNode *history_list = NULL; // head of the history list
historyNode *history_tail = NULL; // tail of the history list
int history_count = 0; // counter to know if we can add more command as regular or we need to remove / delete the oldest command in the history list (the head of the list)


//given struct process (3.a - process List)
typedef struct process{
    cmdLine* cmd;                         /* the parsed command line*/
    pid_t pid; 		                  /* the process id that is running the command*/
    int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next;	                  /* next process in chain */
} process;

//function declarations
void execute(cmdLine* pCmdLine);
void executePipe(cmdLine* pCmdLine);
void addProcess(process** process_list, cmdLine* cmd, pid_t pid);
void printProcessList(process** process_list);
void freeProcessList(process* process_list);
void updateProcessList(process **process_list);
void updateProcessStatus(process* process_list, int pid, int status);
void printHistoryList(historyNode* history_node);
void addtoHistoryList(const char* command);
char* getCommandFromHistoryList(int index);


process* process_list = NULL; // global variable to hold the head of the process list (needs to be under the struct process)

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

void execute(cmdLine* pCmdLine){
    pid_t pid = fork(); // creates a new process (Fork actually duplicates a running process) (PID == Process ID)

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
        
        addProcess(&process_list, pCmdLine, pid);

        // if the command blocks (without &), wait fot the child to finish (and in the command in the background, let it run and the shell will continue)
        // 1c -> use waitpid() only if the command doees NOT end with '&' (blocking == 1)
        if (pCmdLine->blocking) waitpid(pid, NULL, 0);
        
    } 
}

void executePipe(cmdLine* pCmdLine){
    int pipefd[2];

    if(pipe(pipefd) == -1){
        perror("pipe failed");
        exit(1);
    }

    if (pCmdLine->outputRedirect != NULL) {
        fprintf(stderr, "Error: output redirection on left side (first command) of pipe is not allowed.\n");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }
    if (pCmdLine->next->inputRedirect != NULL) {
        fprintf(stderr, "Error: input redirection on right side (second command) of pipe is not allowed.\n");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }
    
    //from task1 - mypipeline.c
    fprintf(stderr, "(parent_process>forking…)\n");

    pid_t pid_child1 = fork(); 

    if (pid_child1 == -1){
        perror("fork failed");
        exit(1);
    }

    if(pid_child1 == 0){ // child 1 - dup stdout to pipefd[1] and execute ls -ls (so this child will write and not read)
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe…)\n");

        if (pCmdLine->inputRedirect != NULL) {
            close(STDIN_FILENO);
            if (open(pCmdLine->inputRedirect, O_RDONLY) == -1) {
                perror("open failed");
                _exit(1);
            }
        }
        
        close(STDOUT_FILENO); // Close the standard output.

        dup(pipefd[1]); // new pipefd[1] will be the new standard output. so the output of ls -ls will be redirected to the pipe instead of the screen.
        
        close(pipefd[1]); // Close the write end of the pipe because now we duplicated it to the stdout so we dont need the original also open.
        //child2 needs to read from the pipe so we DO NOT WRITE: close(pipefd[0]); 
        //until here from task 1 
        close(pipefd[0]);

        fprintf(stderr, "(child1>executing command: %s)\n", pCmdLine->arguments[0]);

        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
            perror("execvp failed");
            _exit(1);
        }
    }

    //Back to parent
    //from task1 again. mypipeline.c
    
    fprintf(stderr, "parent_process>created process with id: "  "%d\n", pid_child1);
    fprintf(stderr, "(parent_process>forking…)\n");

    pid_t pid_child2 = fork();
    
    if (pid_child2 == -1){
        perror("fork failed");
        exit(1);
    }

    if(pid_child2 == 0){ // child 2 - dup stdin to pipefd[0] and execute wc

        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe…)\n");
        close (STDIN_FILENO); // close the standard input.

        dup(pipefd[0]);

        close(pipefd[0]); // close the read end of the pipe because now we duplicated it to the stdin so we dont need the original also open.
        close(pipefd[1]); // close write end because this child is not going to write to the pipe
        //until here code from task1

        if (pCmdLine->next->outputRedirect != NULL) {
            close(STDOUT_FILENO);
            if (open(pCmdLine->next->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0666) == -1) {
                perror("open failed");
                _exit(1);
            }
        }
        
        fprintf(stderr, "(child2>executing command: %s)\n", pCmdLine->next->arguments[0]);

        if (execvp(pCmdLine->next->arguments[0], pCmdLine->next->arguments) == -1) {
            perror("execvp failed");
            _exit(1);
        }
    }

    //Back to parent once again
    //Both childs receivced whats thy need (child1 -> its stdout is connected to pipefd[1], child2 -> its stdin is connected to pipefd[0]). 
    //so parent process no longer need the pipe (because the childs use pipe(pipe[0], pipe[1]))
    //from task1 again
    fprintf(stderr, "parent_process>created process with id: "  "%d\n", pid_child2);
    fprintf(stderr, "(parent_process>closing the read end & write end of the pipe…)\n");
    close(pipefd[0]); 
    close(pipefd[1]);

    fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");
    waitpid(pid_child1, NULL, 0); // parent process waits for child 1 to terminate before continuing to child 2
    waitpid(pid_child2, NULL, 0); // parent process waits for child 2 to finish (and then the parent continue and finish the function).

    fprintf(stderr, "(parent_process>exiting…)\n");
}

// psrt 3a

/* "Receive a process list (process_list), a command (cmd), and the process id (pid) of the process running the command. 
Note that process_list is a pointer to a pointer so that we can insert at the beginning of the list if we wish." */
void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* new_process = (process*)malloc(sizeof(process)); 
    if (new_process == NULL) {
        perror("(malloc) failed to allocate memory for new_process");
        exit(1);
    }

    new_process->cmd = cmd; 
    new_process->pid = pid;
    new_process->status =  RUNNING; // status of the process: RUNNING/SUSPENDED/TERMINATED
    new_process->next = *process_list; // add the new process to the beginning of the list

    *process_list = new_process; // update process list to point to the new process
}

// "print the processes."
void printProcessList(process** process_list){
    updateProcessList(process_list); // update the process list before printing it

    printf("PID\t\tCommand\t\tSTATUS\n"); // headline (\t = tab)

    process* curr_proccess = *process_list; // start from the head of the list
    process* prev = NULL;

    while (curr_proccess != NULL){
        printf("%d\t\t%s\t\t", curr_proccess->pid, curr_proccess->cmd->arguments[0]); // print the pid and the command

        if (curr_proccess->status == RUNNING) {
            printf("RUNNING\n");
        } else if (curr_proccess->status == SUSPENDED) {
            printf("SUSPENDED\n");
        } else if (curr_proccess->status == TERMINATED) {
            printf("TERMINATED\n");
        }

        if(curr_proccess->status == TERMINATED){
            if(prev == NULL){ //curr_process is the head of the list
                *process_list = curr_proccess->next; 
                freeCmdLines(curr_proccess->cmd); // free the cmdLine struct
                free(curr_proccess); 
                curr_proccess = *process_list; // move to the new head :)
            } else { // curr_process is not the head of the list
                prev->next = curr_proccess->next; 
                freeCmdLines(curr_proccess->cmd);
                free(curr_proccess);
                curr_proccess = prev->next; 
            }
        } else { // curr_proccess->status != TERMINATED
            prev = curr_proccess;
            curr_proccess = curr_proccess->next; 
        }
    }
}

// part 3b - updating the process list
//free all memory allocated for the process list.
void freeProcessList(process* process_list){
    process* curr_process = process_list; // start from the head of the list
    while (curr_process != NULL){
        process* next_process = curr_process->next; // save the next process
        freeCmdLines(curr_process->cmd); // free the cmdLine struct
        free(curr_process); // free the process struct
        curr_process = next_process; 
    }
}

//go over the process list, and for each process check if it is done
void updateProcessList(process **process_list){
    process* curr_process = *process_list;
    int status;
    pid_t result_pid;

    while(curr_process != NULL){
    // WNOHANG = Non-blocking: waitpid returns immediately even if there is no change in the child process state
    // WUNTRACED = Return if the child process was stopped (received SIGSTOP signal)
    // WCONTINUED = Return if the child process was resumed (received SIGCONT signal)

        result_pid = waitpid(curr_process->pid, &status, WNOHANG | WUNTRACED | WCONTINUED); 
        if (result_pid == -1) {
            curr_process->status = TERMINATED;
            curr_process = curr_process->next;
            continue;
        }

		if (result_pid != 0)
		{
			if (WIFSIGNALED(status) || WIFEXITED(status))
				curr_process->status = TERMINATED;
			
			else if (WIFSTOPPED(status))
				curr_process->status = SUSPENDED;
			
			else if (WIFCONTINUED(status))
				curr_process->status = RUNNING;
		}
		curr_process = curr_process->next;
    }
}

//find the process with the given id in the process_list and change its status to the received status.
void updateProcessStatus(process* process_list, int pid, int status){
    process* curr = process_list;
    while(curr){
        if (curr->pid == pid){
            curr->status = status;
            return;
        }
        curr = curr->next;
    }
}

// part 4 - history list
void printHistoryList(historyNode* history_list){
    historyNode* curr = history_list; // starts from the head of the history linked list
    int index = 1; 
    for (int i = 0; i < history_count; i++){
       printf("%d: %s\n", index++, curr->command); // print the index and the command. (index++ will increase the index by 1 after printing it)
       curr = curr->next;
    }
 }

 void addtoHistoryList(const char* command){
    historyNode* new_node = (historyNode*)malloc(sizeof(historyNode)); // allocate memory for the new node (this noce will save the command)
    if (new_node == NULL) {
        perror("malloc failed");
        exit(1);
    }
    new_node->command = strdup(command); // copy the command the user entered to the new node 
    new_node->next = NULL; // set the next pointer to NULL

    history_tail == NULL ? (history_list = history_tail = new_node) : (history_tail = history_tail->next = new_node); // if the history list is empty --> set the head and tail to the new node. else --> add the new node to the end of the list

    history_count++; // increase the count of commands in the history list

    if (history_count > HISTLEN) { // if the history list is longer than HISTLEN, remove the oldest command (the head of the list)
        historyNode* temp = history_list;
        history_list = history_list->next; // move the head to the next node
        free(temp->command); // free the command string
        free(temp); // free the old head node
        history_count--;
    }
 }

 char* getCommandFromHistoryList(int index){
    if (index < 1 || index > history_count) return NULL;
    historyNode* curr = history_list;
    for(int i = 1; i < index; i++) // go to the node at the given index
        curr = curr->next;
    return curr->command; 
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

        if(strcmp(input, "hist") == 0){
            printHistoryList(history_list); // print the history list
            continue; // continue to the next iteration of the loop
        }

        addtoHistoryList(input);

        //part 4 - history - HANDLE HISTORY COMMAND BEDORE PARSING
        if (strcmp(input, "!!") == 0){
            if(history_tail == NULL){
                printf("No commands in history.\n");
                continue;
            }
            strcpy(input, history_tail->command);
            printf("%s\n", input);
        }
        else if(input[0] == '!' && isdigit(input[1])){ // if the first char is '!' and the second char is a digit
            int index = atoi(&input[1]); // convert the string to an integer
            char* command = getCommandFromHistoryList(index); 
            if (command == NULL) {
                printf("No command with index %d in history list.\n", index);
                continue;
            }
            strcpy(input, command); // copy the command to the input string
            printf("%s\n", input);
        }

        //parseCmdLines pass the input string and return a pointer to the cmdLine struct (which is a linked list of cmdLine structs (THE STRUCT CONTAINS FILED "next" so its component of linked list)) that contains the parsed command line
        cmdLine* parsedCmd = parseCmdLines(input); 

        if (parsedCmd == NULL) continue; // if the input is empty(the user prees just enter so "input" will be empty string or just spaces), we will continue to the next iteration of the loop

        // identify ths command: "procs" (task 3a)
        if (strcmp(parsedCmd->arguments[0], "procs") == 0) {
            printProcessList(&process_list); // pass the address
            freeCmdLines(parsedCmd); 
            continue; 
        }

        // // identify the command: cd (task 1b)
        if (strcmp(parsedCmd->arguments[0], "cd") == 0) {
            //checks if the user did not provide a path to cd (a cd to move to)
            if(parsedCmd->argCount < 2){
                fprintf(stderr, "cd: missising argument \n");
            } else {
                if (chdir(parsedCmd->arguments[1]) == -1)  //chdir() tries to change the argument (folder) of the current process (the shell) acording to the path the user provided.
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
            else {
                if (signal_num == SIGSTOP)
                    updateProcessStatus(process_list, target_pid, SUSPENDED);
                else if (signal_num == SIGCONT)
                    updateProcessStatus(process_list, target_pid, RUNNING);
                else if (signal_num == SIGINT)
                    updateProcessStatus(process_list, target_pid, TERMINATED);
            }
                
            freeCmdLines(parsedCmd); // avoid memory leak because we are not going to execute the command and skip freeCmdLines at the end of the loop
            continue; // exit the loop and continue to the next iteration
        }

        if (parsedCmd->next != NULL) {
            executePipe(parsedCmd);
        } else {
            execute(parsedCmd);
        }
        
    }
    // free the history list
    historyNode* curr = history_list;
    while (curr != NULL) {
        historyNode* temp = curr;
        curr = curr->next;
        free(temp->command);
        free(temp);
    }


    return 0;
    
}