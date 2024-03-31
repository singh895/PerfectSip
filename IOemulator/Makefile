WARNING = -Wall -Wshadow -Wvla -pedantic
GCC = gcc -O3 $(WARNING)
SRCS = main.c bluart.c bluetooth.c ttl.c io.c
OBJS = $(SRCS:%.c=%.o)

SRCD = src
OBJD = obj


all: $(OBJS:%=obj/%)
	$(GCC) $^ -o emulator

obj/%.o: src/%.c
	$(GCC) -c $^ -o $@