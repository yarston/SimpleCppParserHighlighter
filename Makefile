all:
	gcc -g -fsanitize=address list.c parser.c main.c -o parser
clean:
