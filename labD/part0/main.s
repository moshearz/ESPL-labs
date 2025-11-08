

section .data ; this section contains initialized data
    format db "%d" , 10, 0 ; format string for printf

section .text ; this section contains code
    global main ; this is the entry point of the program
    extern printf ; this is used to print formatted output (import from C library)
    extern puts ; this is used to print a string (import from C library)

main:
    ; [esp] - return address , [esp + 4] - argc (num of parameters), [esp + 8] - argv(pointer to array of strings
    push ebp ; 
    mov ebp, esp ; set base pointer to current stack pointer

    mov eax, [ebp + 8] ; get argc (number of arguments)
    push eax ; push argc onto the stack for printf
    push format ; push the format string onto the stack
    call printf ; call printf to print the number of arguments
    add esp, 8 ; clean up the stack (2 arguments were pushed)

    mov ecx, [ebp + 8] ; get argc again
    mov ebx, [ebp + 12] ; get argv (pointer to array of strings)
    xor eax, eax ; clear eax to use it as a counter (eax = 0)

.loop:
    cmp eax, ecx ; compare counter with argc
    je .done ; if eax >= ecx, jump to done
    mov edx, [ebx + eax * 4] ; get the address of the current argument (argv[eax])
    push ecx ; push argc onto the stack for puts
    push eax ; push the current index onto the stack
    push edx ; push the current argument onto the stack
    call puts ; call puts to print the current argument
    add esp, 4 ; clean up the stack (3 arguments were pushed)
    pop eax ; increment the counter (eax = eax + 1)
    pop ecx ; restore argc
    add eax, 1 ; increment the counter
    jmp .loop ; repeat the loop

.done:
    mov eax, 0 ; set return value to 0
    mov esp, ebp ; restore stack pointer
    pop ebp ; restore base pointer
    mov eax, 1 ; return 0
    int 0x80 ; exit the program (Linux syscall)
    nop ; no operation (just a placeholder, can be removed)
