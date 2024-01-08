
CC=gcc
CFLAGS=-Wall -Wextra -std=c17
LIBS=`pkg-config --libs sdl2`
SRC=src/main.c src/emu.c src/chip.c src/font.c src/audio.c

CFLAGS_WINDOWS=-Wall -Wextra -std=c17 -static
LIBS_WINDOWN=`pkg-config --libs --cflags --static sdl2`

all:
	$(CC) $(CFLAGS) $(LIBS) $(SRC) -o build/chip8emu

debug:
	$(CC) $(CFLAGS) $(LIBS) $(SRC) -o build/chip8emu -DDEBUG

windows:
	$(CC) $(CFLAGS_WINDOWS) $(LIBS_WINDOWN) $(SRC) -o build/chip8emu $(LIBS_WINDOWN)

clean:
	rm build/chip8emu

