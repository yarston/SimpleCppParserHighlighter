all:
	gcc -lm -g -fsanitize=address list.c parser.c map.c main.c -o parser
clean:
