# Chip-8 Emulator

This is just a simple chip-8 emulator, probably not very different from most of the others on github. But I'm trying to use this repo as a sandbox for playing around with some ideas for my next emulators, such as improving performance on instruction fetching (perhaps use computed goto?), using very few and small third party libs for sound and video, how to organize the code, etc.

## References

This is quite a simple emulator to write, so I've managed to use only the following links:

<http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/>

<https://en.wikipedia.org/wiki/CHIP-8>

<http://devernay.free.fr/hacks/chip8/C8TECH10.HTM>

## Build

```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
make
```

The emulator will be on chip8/bin folder.
