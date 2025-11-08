section .data
    x_struct:  dw  5
    x_num:     db 0xaa, 1,2,0x44,0x4f
    y_struct:  dw  6
    y_num:     db 0xaa, 1,2,3,0x44,0x4f
    isOdd: db 0
    isLeadZero: db 1
    structP: dd 0
    STATE: dw 0xACE1
    MASK: dw 0x002d

    format: db "%02hhx", 0
    newline: db 10, 0
section .bss
    buffer resb 600

section .text
    extern printf
    extern puts
    extern fgets
    extern strlen
    extern malloc
    extern free
    extern stdin
    global main

main:
    push ebp ; Save ebp(base pointer)
    mov ebp, esp
    mov ecx, [ebp+8] ; Get first arguments argc
    mov edx, [ebp+12] ; Get second arguments argv
    cmp ecx, 1
    jz xyStructs
    mov ebx, [edx+4]
    cmp word [ebx], 0x692d     ; "-i" in little endian
    jz getStructs
    cmp word [ebx], 0x722d     ; "-r" in little endian  
    jz generateStructs
    jmp doneMain
    
xyStructs:
    push dword x_struct
    call print_multi
    add esp, 4
    push dword y_struct
    call print_multi
    add esp, 4
    push dword x_struct
    push dword y_struct
    call add_multi2
    add esp, 8
    push eax
    push eax
    call print_multi
    add esp, 4
    ; free malloc
    pop eax
    push eax
    call free
    add esp, 4
    jmp doneMain
    
getStructs:
    call get_multi
    mov ebx, eax
    call get_multi
    mov ecx, eax
    push ebx
    call print_multi
    add esp, 4
    push ecx
    call print_multi
    add esp, 4
    push ebx
    push ecx
    call add_multi
    add esp, 8
    push eax
    push ebx
    push ecx
    push eax
    call print_multi
    add esp,4
    ; free malloc
    pop ecx
    push ecx
    call free
    add esp, 4
    pop ebx
    push ebx
    call free
    add esp, 4
    pop eax
    push eax
    call free
    add esp, 4
    jmp doneMain
    
generateStructs:
    call PRmulti
    mov ebx, eax
    call PRmulti
    mov ecx, eax
    push ebx
    call print_multi
    add esp, 4
    push ecx
    call print_multi
    add esp, 4
    push ebx
    push ecx
    call add_multi
    add esp, 8
    push eax
    push ebx
    push ecx
    push eax
    call print_multi
    add esp,4
    ; free malloc
    pop ecx
    push ecx
    call free
    add esp, 4
    pop ebx
    push ebx
    call free
    add esp, 4
    pop eax
    push eax
    call free
    add esp, 4
    
doneMain:
    mov esp, ebp
    pop ebp
    ret

print_multi:
    push ebp
    mov ebp, esp
    pushad
    mov byte [isLeadZero], 1
    mov ebx, [ebp+8]     ; pointer to struct multi
    mov eax, 0
    mov ax, word [ebx]   ; size (unsigned short, not byte!)
    add ebx, 2           ; skip size field to get to num array
    
printNext:
    mov ecx, 0
    mov cl, [ebx+eax-1]  ; curr element in array (little endian)
    cmp byte [isLeadZero], 1
    jnz print
    cmp cl, 0
    jz afterPrint
    mov byte [isLeadZero], 0
    
print:
    pushad
    push ecx             ; number to print
    push dword format    ; string to print
    call printf
    add esp, 8
    popad
    
afterPrint:
    dec eax              ; move to previous byte
    cmp eax, 0
    jnz printNext
    cmp byte [isLeadZero], 1 ; number is zero
    jnz donePrint
    mov ecx, 0
    push ecx ; number to print
    push dword format ; string to print
    call printf
    add esp, 8
    
donePrint:
    push dword newline
    call printf
    add esp, 4
    popad
    mov esp, ebp
    pop ebp
    ret

get_multi:
    push ebp
    mov ebp, esp
    pushad
    mov byte [isOdd], 0
    push dword [stdin]
    push dword 600
    push dword buffer
    call fgets
    add esp, 12
    push buffer
    call strlen
    add esp, 4
    dec eax                ; remove newline
    mov ebx, eax           ; ebx = string length
    and ebx, 1
    cmp ebx, 1
    jnz initialStruct
    ; odd length - pad with leading zero
    mov byte [isOdd], 1
    inc eax                ; make it even
    
initialStruct:
    shr eax, 1             ; eax = size in bytes
    mov ebx, eax           ; ebx = size in bytes
    add eax, 2             ; add 2 for unsigned short size field
    push ebx
    push eax
    call malloc
    add esp, 4
    pop ebx
    mov [structP], eax
    mov word [eax], bx     ; store size as unsigned short (word)
    
    mov esi, 0             ; input string index
    mov edi, ebx           ; output array index (start from end)
    dec edi                ; point to last byte
    
    cmp byte [isOdd], 1
    jnz loopStruct
    ; handle single leading digit
    mov ecx, 0
    mov cl, byte [buffer+esi]
    inc esi
    push ecx
    call digitToDec
    add esp, 4
    mov ecx, eax
    mov eax, [structP]
    add eax, 2             ; point to data array
    mov byte [eax+edi], cl ; store in little endian
    dec edi
    
loopStruct:
    cmp edi, -1            ; check if done
    jz doneLoopStruct
    
    ; get high nibble
    mov edx, 0
    mov dl, byte [buffer+esi]
    inc esi
    push edx 
    call digitToDec
    add esp, 4
    mov edx, eax
    
    ; get low nibble  
    mov ecx, 0
    mov cl, byte [buffer+esi]
    inc esi
    push ecx 
    call digitToDec
    add esp, 4
    mov ecx, eax
    
    ; combine nibbles
    shl edx, 4
    add ecx, edx
    
    ; store byte
    mov eax, [structP]
    add eax, 2             ; point to data array
    mov byte [eax+edi], cl
    dec edi
    jmp loopStruct
    
doneLoopStruct:
    popad
    mov eax, [structP]
    mov esp, ebp
    pop ebp
    ret

digitToDec:
    push ebp
    mov ebp, esp
    push ecx
    mov cl, [ebp+8] ; digit
    sub cl, '0'
    cmp cl, 9
    jle doneDigitToDec
    add cl, '0'
    sub cl, 'a'
    add cl, 10
    
doneDigitToDec:
    mov eax, 0
    mov al, cl
    pop ecx
    mov esp, ebp
    pop ebp
    ret

getMaxMin:
    push ecx
    push edx
    mov ecx, 0
    mov cx, word [eax]   ; read size as word
    mov edx, 0
    mov dx, word [ebx]   ; read size as word
    cmp dx, cx
    jle doneGetMaxMin
    mov ecx, eax
    mov eax, ebx
    mov ebx, ecx
    
doneGetMaxMin:
    pop edx
    pop ecx
    ret

add_multi:
    push ebp
    mov ebp, esp
    pushad
    mov eax, [ebp+8] ; first pointer 
    mov ebx, [ebp+12] ; second pointer
    call getMaxMin
    
    ; Get sizes (stored as words, not bytes)
    mov ecx, 0
    mov cx, word [eax]   ; size of larger array
    mov edx, 0  
    mov dx, word [ebx]   ; size of smaller array
    
    ; Save original max size
    push ecx             ; save original max size
    
    ; Allocate result struct: size field (2 bytes) + max_size + 1 for carry
    push edx
    push ecx
    mov eax, ecx
    inc eax              ; add 1 for potential carry
    add eax, 2           ; add 2 for size field
    push eax
    call malloc
    add esp, 4
    mov [structP], eax
    pop ecx
    pop edx
    pop edi              ; edi = original max size
    
    ; Store result size (max_size + 1)
    mov eax, [structP]
    mov ebx, ecx
    inc ebx              ; max_size + 1 for carry
    mov word [eax], bx   ; store as word
    
    ; Setup for addition
    mov esi, 0           ; array index (0-based)
    clc                  ; clear carry flag
    
add2Loop: ; both arrays have bytes
    cmp esi, edx         ; check if smaller array exhausted
    jae add1Loop
    cmp esi, edi         ; check against original max size
    jae finishAddLoop
    
    ; Add bytes from both arrays
    mov eax, [ebp+8]     ; reload first pointer
    mov ebx, [ebp+12]    ; reload second pointer
    call getMaxMin       ; ensure eax=larger, ebx=smaller
    
    push ecx
    push edx
    push edi
    mov ecx, 0
    mov edx, 0
    mov cl, byte [ebx+esi+2]  ; +2 to skip size field
    mov dl, byte [eax+esi+2]  ; +2 to skip size field
    adc cl, dl
    mov eax, [structP]
    mov byte [eax+esi+2], cl  ; +2 to skip size field
    pop edi
    pop edx
    pop ecx
    
    inc esi
    jmp add2Loop
    
add1Loop: ; only larger array has bytes
    cmp esi, edi         ; check against original max size
    jae finishAddLoop
    
    mov eax, [ebp+8]     ; reload first pointer  
    mov ebx, [ebp+12]    ; reload second pointer
    call getMaxMin       ; ensure eax=larger
    
    push ecx
    push edi
    mov ecx, 0
    mov cl, byte [eax+esi+2]  ; +2 to skip size field
    adc cl, 0                 ; add carry only
    mov eax, [structP]
    mov byte [eax+esi+2], cl  ; +2 to skip size field
    pop edi
    pop ecx
    
    inc esi
    jmp add1Loop
    
finishAddLoop:
    ; Handle final carry
    mov ecx, 0
    adc cl, 0            ; get final carry
    mov eax, [structP]
    mov byte [eax+esi+2], cl  ; store final carry
    
    popad
    mov eax, [structP]
    mov esp, ebp
    pop ebp
    ret

rand_num:
    push ebx
    push ecx
    push edx
    mov ax, [MASK]       ; 0x002d
    mov bx, [STATE]
    mov cx, bx
    and cx, ax           ; mask the relevant bits
    
    ; compute parity of masked bits
    mov dx, 0            ; parity accumulator
parity_loop:
    test cx, cx
    jz parity_done
    test cx, 1
    jz skip_bit
    inc dx               ; count set bits
skip_bit:
    shr cx, 1
    jmp parity_loop
parity_done:
    and dx, 1            ; dx = parity (0 or 1)
    
    ; shift state right
    shr bx, 1
    
    ; set MSB based on parity
    test dx, dx
    jz no_msb
    or bx, 0x8000        ; set MSB
no_msb:
    mov word [STATE], bx
    mov eax, edx         ; return parity bit
    pop edx
    pop ecx
    pop ebx
    ret

PRmulti:
    pushad
    
generateN:
    call generate8bits
    cmp eax, 0
    jz generateN
    ; eax contains the size in bytes
    mov ebx, eax         ; ebx = size in bytes
    add eax, 2           ; add 2 for word-sized size field
    push ebx
    push eax
    call malloc
    add esp, 4
    pop ebx
    mov [structP], eax
    mov word [eax], bx   ; store size as word
    
    mov ecx, 0           ; byte counter
loopGenerateNum:
    cmp ecx, ebx
    jz doneGenerateStruct
    call generate8bits
    mov edx, [structP]
    mov byte [edx+ecx+2], al ; +2 to skip size field
    inc ecx
    jmp loopGenerateNum
    
doneGenerateStruct:
    popad
    mov eax, [structP]
    ret

generate8bits:
    push edx
    push ebx
    mov edx, 8           ; counter
    mov ebx, 0           ; accumulator
    
loopGenerate8:
    cmp edx, 0
    jz doneGenerate8
    shl ebx, 1
    call rand_num
    ; eax is the new bit
    or ebx, eax
    dec edx
    jmp loopGenerate8
    
doneGenerate8:
    mov eax, ebx
    pop ebx
    pop edx
    ret

add_multi2:
    push ebp
    mov ebp, esp
    pushad
    
    mov eax, [ebp+8]      ; first struct pointer
    mov ebx, [ebp+12]     ; second struct pointer
    
    ; Get sizes
    movzx ecx, word [eax] ; first struct size
    movzx edx, word [ebx] ; second struct size
    
    ; Find max size for result allocation
    mov esi, ecx
    cmp edx, esi
    jle size_ok
    mov esi, edx          ; esi = max(ecx, edx)
    
size_ok:
    ; Allocate result struct: size field (2 bytes) + max_size + 1 for carry
    push eax              ; save first pointer
    push ebx              ; save second pointer
    push ecx              ; save first size
    push edx              ; save second size
    push esi              ; save max size
    
    mov eax, esi
    inc eax               ; +1 for potential carry
    add eax, 2            ; +2 for size field
    push eax
    call malloc
    add esp, 4
    mov [structP], eax
    
    pop esi               ; restore max size
    pop edx               ; restore second size
    pop ecx               ; restore first size
    pop ebx               ; restore second pointer
    pop eax               ; restore first pointer
    
    ; Set result size (max_size + 1 for potential carry)
    push eax
    mov eax, [structP]
    mov edi, esi
    inc edi               ; max_size + 1
    mov word [eax], di
    pop eax
    
    ; Addition loop
    mov edi, 0            ; byte index
    mov byte [isOdd], 0   ; use as carry flag (0 or 1)
    
addition_loop:
    ; Check if we've processed all bytes
    cmp edi, ecx
    jge check_second_array
    cmp edi, edx  
    jge process_first_only
    
    ; Both arrays have bytes at this position
    movzx esi, byte [eax+edi+2]     ; byte from first array
    movzx ecx, byte [ebx+edi+2]     ; byte from second array
    add esi, ecx                    ; add second byte
    movzx ecx, byte [isOdd]         ; get carry
    add esi, ecx                    ; add carry
    mov eax, [ebp+8]                ; restore first pointer
    
    ; Store result byte and update carry
    push eax
    mov eax, [structP]
    mov ecx, esi                    ; move result to ecx for cl access
    mov [eax+edi+2], cl             ; store low 8 bits (esi -> ecx for cl)
    pop eax
    
    mov byte [isOdd], 0             ; clear carry
    cmp esi, 255
    jbe no_carry1
    mov byte [isOdd], 1             ; set carry
    
no_carry1:
    inc edi
    jmp addition_loop
    
process_first_only:
    ; Only first array has bytes left
    cmp edi, ecx
    jge check_final_carry
    
    movzx esi, byte [eax+edi+2]     ; byte from first array
    movzx ecx, byte [isOdd]         ; get carry
    add esi, ecx                    ; add carry only
    
    ; Store result and update carry
    push eax
    mov eax, [structP]
    mov ecx, esi                    ; move result to ecx for cl access
    mov [eax+edi+2], cl
    pop eax
    
    mov byte [isOdd], 0
    cmp esi, 255
    jbe no_carry2
    mov byte [isOdd], 1
    
no_carry2:
    inc edi
    mov ecx, [ebp+8]                ; reload for size check
    movzx ecx, word [ecx]
    jmp addition_loop
    
check_second_array:
    ; Only second array has bytes left
    cmp edi, edx
    jge check_final_carry
    
    movzx esi, byte [ebx+edi+2]     ; byte from second array
    movzx ecx, byte [isOdd]         ; get carry  
    add esi, ecx                    ; add carry only
    
    ; Store result and update carry
    push eax
    mov eax, [structP]
    mov ecx, esi                    ; move result to ecx for cl access
    mov [eax+edi+2], cl
    pop eax
    
    mov byte [isOdd], 0
    cmp esi, 255
    jbe no_carry3
    mov byte [isOdd], 1
    
no_carry3:
    inc edi
    jmp addition_loop
    
check_final_carry:
    ; Handle final carry
    push eax
    mov eax, [structP]
    movzx ecx, byte [isOdd]
    mov [eax+edi+2], cl             ; store final carry
    
    ; If no final carry, reduce the size
    cmp ecx, 0
    jnz keep_size
    dec word [eax]                  ; reduce size by 1
    
keep_size:
    pop eax
    
    popad
    mov eax, [structP]
    mov esp, ebp
    pop ebp
    ret