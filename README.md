# TERMI8

TERMI8 is a fictive video game console for your terminal. It has the limitations and features of a contemporary 1980 console like a limited color palette, resolution and audio.

## Features

### CPU:

TERMI8 emulates a 6502 CPU running at arround 1 MIPS (arround 2-6 Mhz depending on the instruction executet). The 6502 is the most used CPU of all time, that means that there is a lot of documentation for it, a lot of software allready written and a lot of support and help. Plus the instruction set is rather simple yet effective.

### Memory:

TERMI8 has 64 Kilobytes of RAM excluding some bytes for hardware registers (internaly part of the ram). The display takes up arround 12 Kilobytes of memory for a fullscreen buffer, that leaves 52 kilobytes of programm and data space.

### Graphics:

TERMI8 emulates a display driver with a resolution of 160 by 144 pixels with 16 colors out of a palette of 256. The pointer to the screen memory is freely movable allowing for pixel perfect scrolling in the Y direction and 2 pixel exact scrolling in the X direction. There are also 8 freely movable sprites that can use 15 of the 16 color (color 0 is used as transparent). The sprites have a fixed resolution of 16 by 16 pixels.

### Sound:

TERMI8 emulates a sound chip with 4 voices. Each voice has its own ADSR simmilar to the C64 SID chip and 4 waveforms (pulse, triangle, sawtooth and noise). The sound capability of TERMI8 is somewhere between the NES and C64 having more channels then the C64 but lacking the filters and at the same time having more flexibility then the NES but lacking the PCM channel.

### I/O:

TERMI8 has a virtual gamepad onto wich keyboard buttons can be maped (configurable via config). There is a DPAD (Up, Down, Left, Right) and buttons A, B, C and D (who needs start and select anyways).

There is also a feature that allows TERMI8 to acces file of the host PC but only in a specified path. The bootloader can load binary files from this directory and boot them but nothing hinders programs to use this feature as well to split a large program into little rom chunks to cope with the memory restrains. Files can be read from and written to but not created. A possible use next to chainloading programs is to impliment a highscore system.

## Build instructions

There is no release or alpha yet so this section is empty but I try to keep dependencies to a minimum.

## Current development status

TERMI8 is still in very early development and has no stable or even working alpha let alone a release.

Here is a checklist of things that should be in the finished 1.0 release build:

 - [ ] Impliment CPU (WIP)
 - [ ] Impliment Graphics
 - [ ] Impliment Gamepad
 - [ ] Impliment file I/O
 - [ ] Impliment Sound
 - [ ] Impliment Bootloader
	- [ ] Impliment Assembler
	- [ ] Impliment Asset generation tools
 - [ ] Impliemnt External development tools

And here are some things that could be implimented in the future:

- [ ] Networking -> Multiplayer games

As one can see there is still **a lot** to do...

## Credits

TERMI8 uses code from qnaplus to impliment timers used for the vertical synchronisation. The code can be found [here](https://qnaplus.com/implement-periodic-timer-linux/). The author wrote in the comments that its ok to use the code for commercial products so I guess it's ok for non comercial ones as well.

## License

The TERMI8 code is licensed under GPL-3. Do what you want to do with my code but dont blame me if your house burns down while compiling it.
