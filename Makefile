default:
	gcc main.c -o main.o

strict:
	gcc -std=c99 -pedantic -Wall -Wextra main.c -o main.o

debug:
	gcc -std=c99 -pedantic -Wall -Wextra main.c -g -o0 -o main.o

run:
	./main.o

gdb:
	gdb ./main.o

valgrind:
	valgrind -s --leak-check=yes --track-origins=yes ./main.o

clean:
	rm *.o

