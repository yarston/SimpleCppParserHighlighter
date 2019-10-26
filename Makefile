all:
	gcc -g -fsanitize=address list.c parser.c hashmap.c main.c -o parser
clean:
