all: clean compile_bubblesort compile_file2

compile_bubblesort: lab3_bubblesort.c
	gcc -g -m32 -Wall -o lab3_bubblesort lab3_bubblesort.c

compile_file2: file2.c
	gcc -g -m32 -Wall -o file2 file2.c

clean: 
	rm -f lab3_bubblesort file2