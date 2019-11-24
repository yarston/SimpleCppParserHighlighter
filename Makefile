#all:
#	gcc -lm -lX11 -g -fsanitize=address list.c render.c parser.c map.c main.c -o parser
#clean:
	
	
CC=gcc
CFLAGS=-std=c11
CFLAGS+=-g -fsanitize=address
#CFLAGS+=-g -O2
INCLUDE_DIRS=-I. # -I./src -I./platform -I/usr/include -I/usr/local/include/ -I/usr/include/gsl
LIBS=-lc -lm -lpthread -lX11 # -lgsl -lgslcblas
SOURCE_DIRS := ./ ./src
SEARCH_WILDCARDS := $(addsuffix /*.c,$(SOURCE_DIRS)) 
VPATH := $(SOURCE_DIRS)

parser: $(notdir $(patsubst %.c,%.o,$(wildcard $(SEARCH_WILDCARDS))))
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(LIBS) $^ -o $@
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(LIBS) -c -MD $(addprefix -I,$(SOURCE_DIRS)) $<
clean:
	rm -rf *.o *.d parser
