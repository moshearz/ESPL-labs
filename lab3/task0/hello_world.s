WRITE EQU 4
STDOUT EQU 1
EXIT EQU 1

global _start

section .data
    str: db 'hello world!', 10  ; 10 is the newline character ('\n')

section .text

_start:
    mov eax, WRITE          ; syscall number for sys_write
    mov ebx, STDOUT         ; file descriptor 1 (stdout)
    mov ecx, str            ; pointer to the string to print
    mov edx, 13         ; length of the string (including newline)
    int 0x80                ; linux system call
    mov eax, EXIT           ; Exit the program  
    xor ebx, ebx            ; exit code 0 (clean exit)
    int 0x80                ; linux system call


