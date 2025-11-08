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

extern int system_call();

struct linux_dirent {
    unsigned long  d_ino;     /* inode number - unique ID of the file */
    unsigned long  d_off;     /* offset to next directory entry       */
    unsigned short d_reclen;  /* length of this directory record      */
    char           d_name[];  /* filename (null-terminated string)    */
};

int main (int argc , char* argv[], char* envp[])
{
    if (argc > 1 && (argv[1][0] != '-' || argv[1][1] != 'a')) {
        char *error_str = "Error: Exiting with 0x66.\n";
        system_call(SYS_WRITE, STDOUT, error_str, strlen(error_str));
        return 0x66;
    }

    /* "." means current directory. open curr dir in read-only mode. if fails it returns a negative number*/ 
    int fd = system_call(SYS_OPEN, ".", O_RDONLY, 0);
    if (fd < 0) { 
        char *error_str = "Error: Exiting with 0x66.\n";
        system_call(SYS_WRITE, STDOUT, error_str, strlen(error_str));
        return 0x66;
    }

    char buf[BUFSIZE]; /* (≤ 10,000 bytes) */
    int nread; /*number of bytes read by GETDENTS SYSCALL*/ 

    /* Read directory entries in a loop (reads a list of dir rntries (file names) into buffer)*/
    while ((nread = system_call(SYS_GETDENTS, fd, buf, BUFSIZE)) > 0) { /* >0 : num of bytes read(num of dir entries)*/
        int pos = 0; /* curr pose in buf*/
        while (pos < nread) { /* each dir entry one by one*/
            struct linux_dirent* dir = (struct linux_dirent*) (buf + pos); /* interpret curr pos in buf as dir entry*/

            /*Directly write the filename to the screen and second line: new line for new file name*/
            system_call(SYS_WRITE, STDOUT, dir->d_name, strlen(dir->d_name));
            system_call(SYS_WRITE, STDOUT, "\n", 1);
            
            pos += dir->d_reclen;
        }
    }

    /* <0 means error occured */
    if (nread < 0) {
        system_call(SYS_CLOSE, fd);
        char *error_str = "Error: Exiting with 0x66.\n";
        system_call(SYS_WRITE, STDOUT, error_str, strlen(error_str));
        return 0x66;
    }
    /* Cleanup */
    system_call(SYS_CLOSE, fd);
    return 0;
}
