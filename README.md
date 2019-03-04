# Chip-8 Emulator

This is just a simple chip-8 emulator, probably not very different from most of the others on github. But I'm trying to use this repo as a sandbox for playing around with some ideas for my next emulators, such as improving performance on instruction fetching (perhaps use computed goto?), using very few and small third party libs for sound and video, how to organize the code, etc.


## To Do:
Those are the missing features that I'm most interested in.

* ~~Add a simple sound library~~: [TinySoundFont](https://github.com/schellingb/TinySoundFont)
* Add controller support (should be easy with [Dear ImGui](https://github.com/ocornut/imgui)
* Show current instruction and registers, with a step-by-step
* Add a memory viewer
* Perhaps add breakpoints?


## References

This is quite a simple emulator to write, so I've managed to use only the following links:

<http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/>

<https://en.wikipedia.org/wiki/CHIP-8>

<http://devernay.free.fr/hacks/chip8/C8TECH10.HTM>


## Dependencies

For Linux (Ubuntu):

```sh
apt-get install libglfw3 libglfw3-dev
```

On Mac:

```sh
brew install glfw3
```


## Build

```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
make
```

The emulator will be on chip8/bin folder.
