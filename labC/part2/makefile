all: clean compile

compile: myshell.o LineParser.o
	gcc -g -m32 -Wall -o myshell myshell.o LineParser.o

myshell.o: myshell.c LineParser.h
	gcc -g -m32 -Wall -c myshell.c

LineParser.o: LineParser.c LineParser.h
	gcc -g -m32 -Wall -c LineParser.c

clean:
	rm -f *.o  myshell