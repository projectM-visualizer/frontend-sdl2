# ProjectM SDL2 Frontend

This is a reference implementation of an applicatiaon that makes use of the projectM music visualization library.

It will listen to audio input and produce mesmerizing visuals. Some commands are supported.

## Building from source

### libprojectM

First, [build](https://github.com/projectM-visualizer/projectm/wiki/Building-libprojectM) and `sudo make install` [libprojectM](https://github.com/projectM-visualizer/projectm)

### Dependencies

(Assumes you have dependencies installed for libprojectM)

```shell
apt install libsdl2-dev  # debian/ubuntu
brew install sdl2
```

### Build

```shell
mkdir build
cd build
cmake ..
make
```

If all runs successfully, you should have an executable.
