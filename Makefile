default:
	gcc *.c -o prog

strict:
	gcc  *.c -std=c99 -Wall -pedantic -Wextra -o prog
                            
debug:                      
	gcc  *.c -std=c99 -Wall -pedantic -Wextra -g -o0 -o prog

run:
	./prog

gdb:
	gdb ./prog

valgrind:
	valgrind -s --leak-check=yes --track-origins=yes ./prog

clean:
	rm prog

