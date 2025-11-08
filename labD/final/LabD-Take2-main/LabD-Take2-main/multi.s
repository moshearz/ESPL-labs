section .data
    ; data for tests
    x_struct:  dw  5
    x_num:     db 0xaa, 1,2,0x44,0x4f
    y_struct:  dw  6    
    y_num:     db 0xaa, 1,2,3,0x44,0x4f

    number_format db "%d", 10, 0
    hex_format db "%02hhx", 0        ; format for printing hex bytes
    hex_word_format db "%04x", 10, 0 ; format for printing 16-bit hex
    buffer_size equ 600              ; maximum input size
    
    ; variables for the pseudo-random number generator
    STATE dw 0xACE1                  ; initial state
    MASK dw 0xB400                   ; Fibonacci LFSR mask for 16 bits    
    
    ; command line arguments
    flag_i db "-i", 0
    flag_r db "-r", 0

section .bss
    input_buffer resb 600            ; buffer for readimg input

section .text
    global main
    extern printf
    extern puts
    extern putchar
    extern fgets
    extern stdin
    extern malloc
    extern strlen
    extern strcmp

main:
    ; setup stack frame
    push ebp
    mov ebp, esp
    
    ; save the registers we use
    push ebx
    push esi
    push edi

    ; get argc and argv from stack
    mov eax, [ebp+8]    ; argc
    mov ebx, [ebp+12]   ; argv
    
    ; check number of arguments
    cmp eax, 1
    je default_mode     ; no arguments = default mode
    
    ; check the first argument
    mov esi, [ebx + 4]  ; argv[1]
    
    ; compare with "-i" argument
    push flag_i
    push esi
    call strcmp
    add esp, 8
    test eax, eax
    jz input_mode
    
    ; compare with "-r" argument
    push flag_r
    push esi
    call strcmp
    add esp, 8
    test eax, eax
    jz random_mode
    

default_mode:
    ; use predefined x_struct and y_struct
    push x_struct
    call print_multi
    add esp, 4
    
    push y_struct
    call print_multi
    add esp, 4
    
    ; add them together
    push y_struct
    push x_struct
    call add_multi
    add esp, 8
    
    ; print tge result
    push eax
    call print_multi
    add esp, 4
    
    jmp program_end

input_mode:
    ; read 2 numbers from stdin
    call get_multi      ; read first number
    push eax            ; save first number
    
    call get_multi      ; read second number
    push eax            ; save second number
    
    ; reminder for Amit: stack = [first_struct, second_struct] (second on top)
    
    ; print first number
    push dword [esp + 4]  ; push first number (below second on stack)
    call print_multi
    add esp, 4
    
    ; print second number  
    push dword [esp]      ; push second number (on top of stack)
    call print_multi
    add esp, 4
    
    ; stack is still: [first_struct, second_struct]
    ; add_multi expects: add_multi(first, second)
    
    ; get the struct pointers before pushing them
    mov eax, [esp + 4]    ; get first struct pointer
    mov ebx, [esp]        ; get second struct pointer
    push ebx              ; push second as second parameter
    push eax              ; push first as first parameter
    call add_multi
    add esp, 8
    
    ; print the result
    push eax
    call print_multi
    add esp, 4
    
    ; clean up saved numbers
    add esp, 8
    
    jmp program_end

random_mode:
    ; generate two random numbers
    call PRmulti        ; generate first random number
    push eax            ; save first number
    
    call PRmulti        ; generate second random number
    push eax            ; save second number
    
    ; print first number
    push dword [esp + 4]  ; get first number from stack
    call print_multi
    add esp, 4
    
    ; print second number
    push dword [esp]      ; get second number from stack
    call print_multi
    add esp, 4
    
    ; add them together
    mov eax, [esp + 4]    ; get first struct pointer
    mov ebx, [esp]        ; get second struct pointer
    push ebx              ; push second as second parameter
    push eax              ; push first as first parameter
    call add_multi
    add esp, 8
    
    ; print the result
    push eax
    call print_multi
    add esp, 4
    
    ; clean up saved numbers
    add esp, 8

program_end:
    ; restore registers
    pop edi
    pop esi
    pop ebx

    ; clean up and return
    mov esp, ebp
    pop ebp
    mov eax, 0
    ret


;task 1:
print_multi:
    ; set up stack frame
    push ebp
    mov ebp, esp
    
    ; save registers we'll use
    push ebx
    push esi
    push edi

    ; get struct pointer from argument
    mov ebx, [ebp+8]    ; ebx = pointer to struct multi
    
    ; get size from struct (first 2 bytes)
    movzx edi, word [ebx]    ; edi = size
    
    ; point to the number array (after the size field)
    add ebx, 2          ; ebx now points to start of num array
    
    ; start from the highest byte and work backwards
    mov esi, edi        ; esi = current position
    dec esi             ; start from last valid index

print_bytes:
    ; check if we finished all bytes
    cmp esi, 0
    jl print_finished

    ; get the byte at current position
    movzx eax, byte [ebx + esi]
    
    ; print this byte in hex
    push eax
    push hex_format
    call printf
    add esp, 8          ; clean up stack

    ; move to next byte
    dec esi
    jmp print_bytes

print_finished:
    ; add newline at the end
    push 10
    call putchar
    add esp, 4

    ; restore registers
    pop edi
    pop esi
    pop ebx

    ; clean up and return
    mov esp, ebp
    pop ebp
    ret


; task 1.b:
get_multi:
    ; set up stack frame
    push ebp
    mov ebp, esp
    
    ; save registers we'll use
    push ebx
    push esi
    push edi

    ; read input line using fgets
    push dword [stdin]
    push buffer_size
    push input_buffer
    call fgets
    add esp, 12

    ; check if read was successful
    test eax, eax
    jz read_failed

    ; get length of input string
    push input_buffer
    call strlen
    add esp, 4
    mov edi, eax        ; edi = string length

    ; remove newline if present
    cmp byte [input_buffer + edi - 1], 10
    jne no_newline
    dec edi
    mov byte [input_buffer + edi], 0

no_newline:
    ; make sure we have even number of hex digits
    ; if odd, add a leading zero
    test edi, 1         ; check if length is odd
    jz even_length
    
    ; shift string right by 1 and add leading zero
    mov esi, edi        ; start from end
shift_loop:
    cmp esi, 0
    jl shift_done
    mov al, byte [input_buffer + esi]
    mov byte [input_buffer + esi + 1], al
    dec esi
    jmp shift_loop

shift_done:
    mov byte [input_buffer], '0'
    inc edi             ; update length

even_length:
    ; calculate number of bytes needed (length / 2)
    mov eax, edi
    shr eax, 1          ; divide by 2
    mov ebx, eax        ; ebx = number of bytes needed

    ; allocate memory for the struct (2 bytes for size + data bytes)
    add eax, 2
    push eax
    call malloc
    add esp, 4
    mov esi, eax        ; esi = pointer to allocated struct

    ; set the size field
    mov word [esi], bx

    ; convert hex string to bytes (little endian storage)
    mov ecx, 0          ; current byte index
    mov edx, 0          ; current position in string

convert_loop:
    cmp ecx, ebx        ; check if we've converted all bytes
    jge convert_done

    ; get two hex characters
    movzx eax, byte [input_buffer + edx]
    call hex_char_to_num
    shl eax, 4          ; shift to upper nibble
    mov edi, eax        ; save upper nibble

    movzx eax, byte [input_buffer + edx + 1]
    call hex_char_to_num
    or eax, edi         ; combine nibbles

    ; store byte in little endian order (reverse from string order)
    mov edi, ebx        ; total bytes
    sub edi, ecx        ; calculate position from end
    dec edi             ; adjust for 0-based indexing
    mov byte [esi + 2 + edi], al

    ; move to next byte
    inc ecx
    add edx, 2
    jmp convert_loop

convert_done:
    ; return pointer to struct
    mov eax, esi

read_failed:
    ; restore registers
    pop edi
    pop esi
    pop ebx

    ; clean up and return
    mov esp, ebp
    pop ebp
    ret

; helper function to convert hex character to number
hex_char_to_num:
    ; input: eax = hex character
    ; output: eax = numeric value
    cmp al, '9'
    jle is_digit
    
    ; it's a letter (a-f or A-F)
    cmp al, 'F'
    jle is_upper
    
    ; lowercase a-f
    sub al, 'a'
    add al, 10
    jmp hex_done

is_upper:
    ; uppercase A-F
    sub al, 'A'
    add al, 10
    jmp hex_done

is_digit:
    ; digit 0-9
    sub al, '0'

hex_done:
    ret


; task 2.a:
get_max_min:
    ; input: eax = pointer to first struct, ebx = pointer to second struct
    ; output: eax = pointer to struct with larger length, ebx = pointer to other struct
    
    ; save original pointers
    push ecx
    push edx
    mov ecx, eax        ; save first pointer
    mov edx, ebx        ; save second pointer
    
    ; get lengths from both structs
    movzx eax, word [ecx]    ; length of first struct
    movzx ebx, word [edx]    ; length of second struct
    
    ; compare lengths
    cmp eax, ebx
    jge first_is_larger
    
    ; second struct is larger
    mov eax, edx        ; return second struct in eax
    mov ebx, ecx        ; return first struct in ebx
    jmp max_min_done
    
first_is_larger:
    ; first struct is larger (or equal)
    mov eax, ecx        ; return first struct in eax
    mov ebx, edx        ; return second struct in ebx
    
max_min_done:
    pop edx
    pop ecx
    ret


;task 2.b:
add_multi:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    
    ; get arguments
    mov esi, [ebp+8]     ; first strucr
    mov edi, [ebp+12]    ; second struct
    
    ; get max/min
    mov eax, esi
    mov ebx, edi
    call get_max_min
    ; eax = larger, ebx = smaller
    
    ; get sizes
    movzx ecx, word [eax]    ; max size
    movzx edx, word [ebx]    ; min size
    
    ; allocate result with max_size + 1 for potential carry
    inc ecx              ; temp result size = max + 1
    push eax             ; save larger
    push ebx             ; save smaller
    push ecx             ; save temp result size
    push edx             ; save min size
    
    lea eax, [ecx + 2]   ; total allocation
    push eax
    call malloc
    add esp, 4
    
    pop edx              ; restore min size
    pop ecx              ; restore temp result size
    
    ; temporarily set size to max_len + 1
    mov word [eax], cx   ; set temp size
    
    ; start the addition process
    mov esi, 0           ; index
    mov edi, 0           ; carry
    
add_loop:
    dec ecx              ; convert to 0-based
    cmp esi, ecx
    jg add_done
    inc ecx              ; restore
    
    ; get byte from larger number
    mov ebx, [esp + 4]   ; larger struct
    movzx ebx, byte [ebx + 2 + esi]
    add ebx, edi         ; add carry
    mov edi, 0           ; clear carry
    
    ; add from smaller number if available
    cmp esi, edx
    jge skip_smaller
    mov ecx, [esp]       ; smaller struct
    movzx ecx, byte [ecx + 2 + esi]
    add ebx, ecx
    
skip_smaller:
    ; handle carry
    cmp ebx, 255
    jle store_it
    mov edi, 1
    sub ebx, 256
    
store_it:
    mov byte [eax + 2 + esi], bl
    inc esi
    movzx ecx, word [eax]    ; reload temp result size
    jmp add_loop
    
add_done:
    ; check if we actually have a final carry
    cmp edi, 0
    je no_final_carry
    
    ; store final carry and keep size as is
    mov byte [eax + 2 + esi], 1
    jmp cleanup
    
no_final_carry:
    ; no final carry, reduce the size by 1
    movzx ecx, word [eax]    ; get current size
    dec ecx                  ; reduce by 1
    mov word [eax], cx       ; update size
    
cleanup:
    ; clean up stack
    add esp, 8           ; remove saved pointers
    
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret


;task 3:
rand_num:
    ; linear feedback shift register implementation
    push ebx
    push ecx
    
    ; get current state
    mov ax, [STATE]
    
    ; apply mask to get relevant bits
    mov bx, [MASK]
    and bx, ax           ; bx = STATE & MASK
    
    ; compute parity of masked bits
    mov cx, bx
    xor bx, bx           ; clear bx to use as parity counter
    
parity_loop:
    test cx, cx
    jz parity_done
    
    ; check if LSB is set
    test cx, 1
    jz skip_bit
    inc bx               ; increment parity counter
    
skip_bit:
    shr cx, 1            ; shift right to check next bit
    jmp parity_loop
    
parity_done:
    ; get parity bit (bx & 1)
    and bx, 1            ; bx = parity bit (0 or 1)
    
    ; shift STATE right by 1
    mov ax, [STATE]
    shr ax, 1
    
    ; set MSB based on parity
    test bx, bx
    jz no_parity_bit
    or ax, 0x8000        ; set MSB if parity is 1
    
no_parity_bit:
    ; store new state
    mov [STATE], ax
    
    pop ecx
    pop ebx
    ret


PRmulti:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    
generate_length:
    ; generate ramdom length (1-255 bytes)
    call rand_num
    mov al, byte [STATE]     ; get lower 8 bits
    test al, al
    jz generate_length       ; if zero, generate again
    
    movzx ebx, al            ; ebx = length in bytes
    
    ; allocate memory for struct
    lea eax, [ebx + 2]       ; size + 2 bytes for length field
    push ebx                 ; save length
    push eax
    call malloc
    add esp, 4
    pop ebx                  ; restore length
    
    ; set length field
    mov word [eax], bx
    
    ; generate random bytes for the number
    mov esi, 0               ; current byte index
    mov edi, eax             ; edi = struct pointer
    
generate_bytes:
    cmp esi, ebx
    jge generation_done
    
    ; generate 8 random bits
    call rand_num
    mov cl, byte [STATE]     ; get random byte
    
    ; store in little endian order
    mov byte [edi + 2 + esi], cl
    
    inc esi
    jmp generate_bytes
    
generation_done:
    ; return struct pointer
    mov eax, edi
    
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret