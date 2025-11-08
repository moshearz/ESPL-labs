#include "util.h"

#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291

extern int system_call();

int main (int argc , char* argv[], char* envp[])
{
  int i; /* In ANSI C (C89/C90), all variables must be declared at the beginning of a block. (-ansi flag is in our Makefile gcc command which tells GCC to use ANSI C) */
  for(i = 0; i < argc; i++){
    system_call(SYS_WRITE, STDOUT, argv[i], strlen(argv[i])); /* Write the argument to the standard output */
    system_call(SYS_WRITE, STDOUT, "\n", 1); /* Write a newline after each argument */
  }
  return 0;
}
