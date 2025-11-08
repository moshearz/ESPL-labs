all: clean compile looper mypipe

compile: myshell.o LineParser.o
	gcc -g -m32 -Wall -o myshell myshell.o LineParser.o

#there is no myshell.h file and in myshell.c we writes all the files which found in #incluse (in myshell.c : #iclude "LineParser")
myshell.o: myshell.c LineParser.h
	gcc -g -m32 -Wall -c myshell.c

LineParser.o: LineParser.c LineParser.h
	gcc -g -m32 -Wall -c LineParser.c

looper: looper.c
	gcc -g -m32 -Wall -o looper looper.c

mypipe: mypipe.c
	gcc -g -m32 -Wall -o mypipe mypipe.c

#clean: remove all object files and the executable
clean:
	rm -f *.o myshell looper my pipe