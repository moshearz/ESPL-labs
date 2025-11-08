all: clean task1 task4

task1: task1.c	
	gcc -g -m32 -Wall -o task1 task1.c

task4: task4.c
	gcc -g -m32 -Wall -fno-pie -fno-stack-protector -o task4 task4.c

clean: 
	rm -f *.o task1 task4


