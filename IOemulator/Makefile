WARNING = -Wall -Wshadow -Wvla -pedantic
GCC = gcc $(WARNING)
SRCS = main.c bluart.c bluetooth.c ttl.c io.c
OBJS = $(SRCS:%.c=%.o)

SRCD = src
OBJD = obj


all: $(OBJS:%=obj/%)
	$(GCC) $^ -o emulator

obj/%.o: src/%.c
	$(GCC) -g -c $^ -o $@

debug: $(OBJS:%=obj/%)
	$(GCC) -g $^ -o emulator

clean:
	rm obj/*.o
	rm emulator