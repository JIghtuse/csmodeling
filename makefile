.PHONY: all clean

CC=gcc
CFLAGS=-Wall -pedantic -Werror -ansi -lm -g
APP=run

all: $(APP)

clean:
	rm *.o *.d $(APP)

$(APP): $(patsubst %.c, %.o, $(wildcard *.c))
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -MD $<

include $(wildcard *.d)

test:
	(./$(APP))
