section .data           ;global var
    new_line: db 10 
    char_buff: dd 0
    infile: dd 0
    outfile: dd 1
    err_msg: db "Error: Input file does not exist.", 10, 0 

section .text

global _start
global main
extern strlen

_start: ; gets argc and argv , push them , and call main func
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc
    call    main        ; call main function (call command push the return address)
    mov     ebx,eax     ;exit
    mov     eax,1
    int     0x80

; Agrements: 1. Push all the args from right-to-left into the stack 2. push curr ebp and save the base(the lowest adress of ebp) adress
; ebp = lower bound of the activation frame 
; esp = upper bound of the AF
main:
    push    ebp             ; Save caller state ;anchor
    mov     ebp, esp        ; set up new stack frame
    sub     esp, 4          ; Leave space for local var on stack. move esp "up" ("increase the activation frame space in memory)
    pushad                  ; Save registers. push to the stack all the registers one by one
    mov     ecx, [ebp+8]    ; ecx = argc

argv_loop:
    push ecx
    mov eax, [ebp+12]   ; eax = argv
    mov ebx, [ebp+8]    ; ebx = argc
    sub ebx, ecx        ; ebx = i       ;first i will be 0
    shl ebx, 2          ; ebx = i * 4
    add eax, ebx        ; eax = eax + i * 4
    mov ecx, [eax]      ; ecx = *(argv + i * 4)
    push ecx
    call check_for_files
    call strlen          ;getting ecx as input ; strlen(const char *str)
    pop ecx
    mov edx, eax        ;eax is returned value edx = strlen(ecx)
    mov eax, 4          
    mov ebx, 1          
    int 0x80
    mov eax, 4              ; add new line
    mov ebx, 1
    mov ecx, new_line
    mov edx, 1              
    int 0x80  
    pop ecx
    dec ecx
    jnz argv_loop
    
encoder_loop: 
    mov eax, 3           ; read char from stdin 
    mov ebx, [infile]      
    mov ecx, char_buff  
    mov edx, 1            
    int 0x80 
    cmp eax, 0               ; check if eax > 0 (returned value from read)
    jle end_encoder_loop      ;jump if eax is  0 or less
    push dword [char_buff]
    call encode               ;encode function
    mov  dword [char_buff], eax
    add esp, 4              ;clean stack
    mov ecx, char_buff          ; write char to stdout
    mov eax, 4            
    mov ebx, [outfile]         
    mov edx, 1
    int 0x80
    jmp encoder_loop

end_encoder_loop:
    cmp dword [infile], 0    ; Close input/output files
    jne close_out_file    ;jumps if(infile!= stdin) 
    mov eax, 6           ; system call for close()
    mov ebx, [infile]      
    int 0x80

close_out_file:
    cmp dword [outfile], 1
    jne main_end            ; Jump if outfile is not equal to stdout
    mov eax, 6          
    mov ebx, [outfile]     
    int 0x80

main_end:
    popad                 ; Restore registers
    mov esp, ebp          ; clean stack
    pop ebp             
    ret                     

encode:
    mov eax , [esp + 4]       ; eax = char
    cmp eax, 'A'               ; check if char is between 'A' and 'Z'
    jb skip
    cmp eax, 'Z'
    ja skip
    sub     eax, 1             ; encode char
skip:
    ret
    
check_for_files:
    push    ebp                     ; Save caller state
    mov     ebp, esp
    pushad                          ; Save some more caller state
    mov ebx, [ebp+8]                ;argv[i]
    cmp byte [ebx], '-'             ; Check if the argument starts with '-' 
    jne check_end
    cmp byte [ebx+1], 'o'           ; Check for output file
    jne check_input_file
    add ebx, 2
    mov eax, 5          ; system_call for open()
    mov ecx, 0x241      ; O_WRONLY == 1 | O_CREAT==64 | O_TRUNC ==512
    mov edx, 0777o      ;file permissions
    int 0x80
    sub ebx, 2
    cmp eax, 0
    jl check_input_file
    mov [outfile], eax

check_input_file:
    cmp byte [ebx+1], 'i'    ; Check for input file
    jne check_end
    add ebx, 2
    mov eax, 5          ; open syscall
    mov ecx, 0          ; O_RDONLY
    int 0x80
    cmp eax, 0
    jl show_error   
    mov [infile], eax
    jmp check_end

show_error:              ; error handling for missing file
    mov eax, 4          
    mov ebx, 2          ; stderr
    mov ecx, err_msg    
    mov edx, 32         
    int 0x80
    mov eax, 4          
    mov ebx, 2          
    mov ecx, new_line    
    mov edx, 1         
    int 0x80
    mov eax, 1         
    mov ebx, 0      
    int 0x80

check_end:
    popad                   ; Restore registers
    pop     ebp             
    ret                     