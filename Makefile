CC=gcc
CFLAGS=-std=c11
CFLAGS+=-Os #-g -fsanitize=address
INCLUDE_DIRS=-I.
LIBS=-lc -lm -lpthread -lX11
SOURCE_DIRS := src
SEARCH_WILDCARDS := $(addsuffix /*.c,$(SOURCE_DIRS)) 
VPATH := $(SOURCE_DIRS)

parser: $(patsubst %.c,build/%.o,$(wildcard $(SEARCH_WILDCARDS)))
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(LIBS) $^ -o $@
	strip $@
	
build/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(LIBS) -c -MD $(addprefix -I,$(SOURCE_DIRS)) $< -o $(patsubst %.c,%.o,build/$<)
	
clean:
	find build/ -type f -name "*.o" -delete
	find build/ -type f -name "*.d" -delete
	rm -f parser

-include $(patsubst %.c,build/%.d,$(wildcard $(SEARCH_WILDCARDS)))