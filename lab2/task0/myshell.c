#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/wait.h>
#include "LineParser.h"

//#include "linux/limits.h"
//#include <signal.h>
//#include <fcntl.h>

#define BUFFER_SIZE 2048

void execute(cmdLine* pCmdLine){
    pid_t pid = fork(); // creates a new process (Fork actually duplicates a running process)

    if (pid == -1){ // fork == -1 means that the fork failed
        perror("fork failed");
        exit(1);
    }

    if (pid == 0){//To the new process, the child, fork returns 0.
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
            perror("execvp failed");
            _exit(1);
        }
    } else {
        if (pCmdLine->blocking) {
            waitpid(pid, NULL, 0);
        }
    } 
}

int main(){
    char buffer[PATH_MAX];
    char input[BUFFER_SIZE];

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

        if(strcmp(input, "quit") == 0) break;

        cmdLine* parsedCmd = parseCmdLines(input);

        if (parsedCmd == NULL) continue;

        execute(parsedCmd);

        freeCmdLines(parsedCmd);

    }

    return 0;
    
}