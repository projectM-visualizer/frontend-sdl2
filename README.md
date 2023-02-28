# ProjectM SDL2 Frontend

This is a reference implementation of an applicatiaon that makes use of the projectM music visualization library.

It will listen to audio input and produce mesmerizing visuals. Some commands are supported.

This project is in a bit of a transition state and is in the process of being modernized. There are many rough edges at present.

## Building from source

### Build and install libprojectM

First, [build](https://github.com/projectM-visualizer/projectm/wiki/Building-libprojectM) and `sudo make install` [libprojectM](https://github.com/projectM-visualizer/projectm)

### Dependencies

(Assumes you have dependencies installed for libprojectM)

```shell
apt install libsdl2-dev libpoco-dev cmake  # debian/ubuntu
brew install sdl2  # macOS
```

### Build

```shell
mkdir build
cd build
cmake ..
make
```

If all runs successfully, you should have an executable.

#### Linux
[Note: 'make install' is unimplemented at the moment. Just copy the binary 'projectMSDL' to your choice of run-path. E.g.]
```shell
cp src/projectMSDL ~/bin
```
Create a configuration file or projectMSDL will complain, a lot.
```shell
mkdir ~/.config/projectM
cp src/projectMSDL.properties ~/.config/projectM
```
The default audio device (-1) may or may not be your actual default audio output. `projectMSDL -l` will list audio devices; hopefully, one of them looks familiar - like "Monitor of ... digital stereo" or "Monitor of USB Audio Device ..." (if you have one of those).


### Run

You should have a directory of visual presets you wish to use. You can fetch a giant trove of curated presets [here](https://github.com/projectM-visualizer/presets-cream-of-the-crop).

Provide the presets path you wish to use when starting projectMSDL:

```shell
src/projectMSDL --presetPath /path/to/presets-cream-of-the-crop
```

Press F1 for help menu.

## Developing

This project uses cmake, which can generate project files for your favorite IDE.

To generate an Xcode project:

```shell
make clean
cmake -G Xcode -S . -B build
```
