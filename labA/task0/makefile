# Link combines between the files and the libiries and teturn us an exe file. 

# compile translate to machine 'code'. in C we will do the compile to each file seperately.

# flags explanation for C :
# Key gcc Parameters. gcc is the compile of C ! (gcc is the compiler for c, while nasm is the assembler for assembly code.
# -g: Adds debug information for debugging tools.
# -m32: compile for 32-bit architecture
# -Wall: Enables all warnings.
# -o fileName: Specifies the output file name. will give us a new name to the compilation file
# -c: Compile only; do not link. stom in the compilation do not do also link. because you have more than one file that you want to execute toghether

# "o" file is the object file. its the 'mid - stage' you are not yet able to run this file but its the file after the comilation and its usung you for the link action.

#flage expalantion for assembly:
# nasm is the compiler of assembly !
# Key NASM Parameters:
# -f format: Specifies the output format (e.g., elf).
# -g: Adds debug information.
# -w+all: Enables all warnings. (equivalent to -Wall in gcc)
# -o fileName: Specifies the output file name. will give us a new name to the compilation file

all: clean link 

main.o: main.c
	gcc -g -m32 -Wall -c -o main.o main.c

numbers.o: numbers.c
	gcc -g -m32 -Wall -c -o numbers.o numbers.c

add.o: add.s
	nasm -f elf -g -w+all -o add.o add.s

link: main.o numbers.o add.o
	gcc -g -m32 -Wall -o task0 main.o numbers.o add.o

clean:
	rm -f *.o task0
 