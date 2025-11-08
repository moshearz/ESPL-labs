#include <stdio.h> 
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

//ls -l (ls lists the files in the current directory, -l shows the list in long format (permissions, size, owner, time..),-s adds the size (in disk blocks) to each line):  lists all files, with full details and shows size.
//wc (Word Count): reads that output and counts: How many lines, words, characters were in the listing

int main(int argc, char *argv[]){

    int pipefd[2];  // pipefd[0] -> for using read end. pipefd[1] -> for using write end
    
    if(pipe(pipefd) == -1) { // create a pipe. if failed, print error and exit.
        perror("pipe failed");
        return 1; 
    }

    fprintf(stderr, "parent_process>forking…\n");

    pid_t pid_child1 = fork(); 

    if (pid_child1 == -1){
        perror("fork failed");
        return 1; // (nearly) does the same as exit(1) because we inside main().
    }

    if(pid_child1 == 0){ // child 1 - dup stdout to pipefd[1] and execute ls -l (so this child will write and not read)
        fprintf(stderr, "child1>redirecting stdout to the write end of the pipe…\n");

        close(STDOUT_FILENO); // Close the standard output.

        dup(pipefd[1]); // new pipefd[1] will be the new standard output. so the output of ls -l will be redirected to the pipe instead of the screen.
        
        close(pipefd[1]); // Close the write end of the pipe because now we duplicated it to the stdout so we dont need the original also open.
        close(pipefd[0]); // close read end because this child is not going to read from the pipe

        fprintf(stderr, "child1>going to execute cmd: …\n");
        char * ls_args[] = { "ls" , "-l", NULL};
        execvp(ls_args[0], ls_args); // execute ls -ls. the output will be redirected to the pipe.

        perror("execvp failed"); 
        return 1; 
    }
    // parent process
    fprintf(stderr, "parent_process>created process with id: %d\n", pid_child1);
    fprintf(stderr, "parent_process>closing the write end of the pipe…\n");
    close(pipefd[1]); // close the write end of the pipe (because this parent is not going to write to the pipe) after forking child 1.

    fprintf(stderr, "parent_process>forking…\n");

    pid_t pid_child2 = fork();
    
    if (pid_child2 == -1){
        perror("fork failed");
        return 1;
    }

    if(pid_child2 == 0){ // child 2 - dup stdin to pipefd[0] and execute wc

        fprintf(stderr, "child2>redirecting stdin to the read end of the pipe…\n");
        close (STDIN_FILENO); // close the standard input.

        dup(pipefd[0]);

        close(pipefd[0]); // close the read end of the pipe because now we duplicated it to the stdin so we dont need the original also open.
        close(pipefd[1]); // close write end because this child is not going to write to the pipe

        fprintf(stderr, "child2>going to execute cmd: …\n");
        char * wc_args[] = { "wc", NULL}; 
        execvp(wc_args[0], wc_args); // execute wc. the input will be read from the pipe.

        perror("execvp failed"); 
        return 1; 
    }
    // parent process
    fprintf(stderr, "parent_process>created process with id: %d\n", pid_child2);
    fprintf(stderr, "parent_process>closing the read end of the pipe…\n");
    close(pipefd[0]); // close the read end of the pipe (because this parent is not going to read from the pipe) after forking child 2.

    fprintf(stderr, "parent_process>waiting for child processes to terminate…\n");
    waitpid(pid_child1, NULL, 0); // wait for child 1 to terminate before continuing to child 2
    waitpid(pid_child2, NULL, 0); // wait for child 2 to finish.

    fprintf(stderr, "parent_process>exiting…\n");

    return 0;
}
