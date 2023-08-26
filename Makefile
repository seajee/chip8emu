CC=gcc
CFLAGS=-Wall -Wextra -std=c17
LIBS=`pkg-config --libs sdl2`
SRC=src/main.c src/emu.c src/chip.c src/font.c

all:
	$(CC) $(CFLAGS) $(LIBS) $(SRC) -o build/chip8emu

debug:
	$(CC) $(CFLAGS) $(LIBS) $(SRC) -o build/chip8emu -DDEBUG

clean:
	rm build/chip8emu

