# Chip-8 Emulator

This is just a simple chip-8 emulator, probably not very different from most of the others on github. But I'm trying to use this repo as a sandbox for playing around with some ideas for my next emulators, such as improving performance on instruction fetching (perhaps use computed goto?), using very few and small third party libs for sound and video, how to organize the code, etc.

## Build

```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
make
```

The emulator will be on chip8/bin folder.
