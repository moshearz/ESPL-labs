all: link

link: start.o util.o
	ld -m elf_i386 start.o util.o -o task1

util.o: util.c util.h
	gcc -g -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

start.o: start.s
	nasm -g -f elf32 start.s -o start.o

.PHONY: clean 

clean:
	rm -f *.o task1

