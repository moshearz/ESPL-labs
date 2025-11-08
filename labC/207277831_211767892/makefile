all: clean myshell mypipeline looper

myshell: myshell.o LineParser.o
	gcc -g -m32 -Wall -o myshell myshell.o LineParser.o

myshell.o: myshell.c LineParser.h
	gcc -g -m32 -Wall -c myshell.c

mypipeline: mypipeline.o
	gcc -g -m32 -Wall -o mypipeline mypipeline.o

mypipeline.o: mypipeline.c
	gcc -g -m32 -Wall -c mypipeline.c

LineParser.o: LineParser.c LineParser.h
	gcc -g -m32 -Wall -c LineParser.c

looper: looper.o
	gcc -g -m32 -Wall -o looper looper.o

looper.o: looper.c
	gcc -g -m32 -Wall -c looper.c

clean:
	rm -f *.o  myshell mypipeline looper
