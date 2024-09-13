default:
	gcc main.c card_funcs.c -o main.o

strict:
	gcc -std=c99 -pedantic -Wall -Wextra main.c card_funcs.c -o main.o

debug:
	gcc -std=c99 -pedantic -Wall -Wextra main.c card_funcs.c -g -o0 -o main.o

run:
	./main.o

gdb:
	gdb ./main.o

valgrind:
	valgrind -s --leak-check=yes --track-origins=yes ./main.o

clean:
	rm *.o

