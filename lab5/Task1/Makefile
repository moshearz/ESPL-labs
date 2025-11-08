loader: loader.c
	gcc -m32 -o loader loader.c

test.c:
	echo 'int main() { return 0; }' > test.c

test: loader test.c
	gcc -m32 -o test test.c
	./loader test

clean:
	rm -f loader test test.c *.o