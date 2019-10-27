all:
	gcc -lm -g -fsanitize=address list.c parser.c hashmap.c main.c -o parser
clean:
