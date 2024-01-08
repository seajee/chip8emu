# CHIP-8 Emulator

Yet another CHIP-8 emulator.

## Controls
```
Emulator Keybinds
-----------------------
QUIT           | Escape
PAUSE / RESUME | Space
Reset          | Return
```

```
CHIP-8 Machine Keybinds
-------------------------------
CHIP-8 Keypad | QWERTY Keyboard
123C          | 1234
456D          | QWER
789E          | ASDF
A0BF          | ZXCV
```

## Build

To build the project you will need SDL2 installed as a dependency for this project.

```bash
mkdir build
make
```

### Building on Windows

To build the profect on Windows you need MSYS2, MinGW and SDL2 installed as a package to build this project.
 Once inside the correct environment use the same Makefile as following:

```bash
mkdir build
make windows
```

