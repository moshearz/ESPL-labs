all: clean compile 

compile: myshell.c LineParser.c
	gcc -g -m32 -Wall -o myshell myshell.c LineParser.c 

clean:
	rm -f myshell
