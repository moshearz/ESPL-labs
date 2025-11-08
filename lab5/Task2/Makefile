loader: loader.c start.o startup.o
	gcc -m32 -c loader.c -o loader.o
	ld -m elf_i386 -o loader loader.o start.o startup.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

start.o: start.s
	nasm -f elf32 start.s -o start.o

startup.o: startup.s
	nasm -f elf32 startup.s -o startup.o

clean:
	rm -f loader loader.o start.o startup.o

