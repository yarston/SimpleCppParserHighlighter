all:
	gcc -g -fsanitize=address list.c main.c -o parser
clean:
