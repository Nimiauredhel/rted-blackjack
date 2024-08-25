default:
	gcc main.c -o main.o; ./main.o

strict:
	gcc -std=c99 -pedantic -Wall -Wextra main.c -o main.o; ./main.o
