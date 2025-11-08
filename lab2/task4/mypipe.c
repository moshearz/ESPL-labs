#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>     
#include <sys/types.h>


int main(int argc, char *argv[]){

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <message>\n", argv[0]);
        return 1;
    }
    
    int pipefd[2]; // fd[0] - read. fd[1] - write
    char buffer[2024]; //allocare array of 2024 bytes to store the message that will read form PIPE (thats where read() will pouring the data to pipe)

    if(pipe(pipefd) == -1) {
        printf("An error occured with opening the pipe!\n");
        return 1; 
    }

    pid_t processID = fork(); //fork splite the process into 2 processes: Parent and Child (The child is a copy of the parent)

    if (processID == -1){ // fork == -1 means that the fork failed
        perror("fork failed");
        return 1;
    }
    // the parent recieve argv[1] and wirte it to the pipe (argv[1] == "hello world" for example). then the child will read from the pipe and print it to the screen.
    if (processID == 0){ // Child
        close(pipefd[1]); // dont need to write, so close fd[1].
        read(pipefd[0], buffer, sizeof(buffer));
        printf("child process: %s\n", buffer); 
        close(pipefd[0]); //finish to read 
    }
    
    else{
        //Parent
        close(pipefd[0]); // dont need to read.
        write(pipefd[1], argv[1], strlen(argv[1]) + 1); // +1 for the null terminator(\0) . which mark the end of the string.
        close(pipefd[1]); // finish write
    }

    return 0;
}