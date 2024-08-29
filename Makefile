default:
	gcc main.c -o main.o

strict:
	gcc -std=c99 -pedantic -Wall -Wextra main.c -o main.o

debug:
	gcc -std=c99 -pedantic -Wall -Wextra main.c -g -o main.o

run:
	./main.o

clean:
	rm *.o

