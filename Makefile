all:
	gcc -lm -lxcb -lxcb-image -lX11 -g -fsanitize=address list.c parser.c map.c main.c -o parser
clean:
