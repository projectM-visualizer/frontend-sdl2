# ProjectM SDL2 Frontend

This is a reference implementation of an applicatiaon that makes use of the projectM music visualization library.

It will listen to audio input and produce mesmerizing visuals. Some commands are supported.

This project is in a bit of a transition state and is in the process of being modernized. There are many rough edges at
present.

## Building from source

### Build and install libprojectM

First, [build](https://github.com/projectM-visualizer/projectm/wiki/Building-libprojectM)
and `sudo make install` [libprojectM](https://github.com/projectM-visualizer/projectm)

### Dependencies

This project requires two third-party libraries in addition to libprojectM's core library dependencies:

- SDL2 (version 2.0.16 or higher)
- POCO (version 1.12 or higher)

Depending on your needs, you can either build them yourself or install them using your favorite package manager. Here
are some examples for the three major desktop platforms:

```shell
sudo apt install libsdl2-dev libpoco-dev cmake  # Debian/Ubuntu Linux
brew install sdl2  # macOS
vcpkg install sdl2 poco # Windows
```

### Configure and build projectMSDL

If all dependencies are in the CMake and/or the system search directories, you can configure and build the application
with these commands, executed from the source dir:

```shell
mkdir cmake-build
cmake -S . -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build --config Release
```

You can optionally add the `--parallel` argument to the build command to speed up the build.

If your dependencies are in different locations than the default search paths, or you're cross-compiling, you'll need to
add more options. Covering all CMake options is out of the scope of this document. Please read
the [Mastering CMake guide](https://cmake.org/cmake/help/book/mastering-cmake/index.html) and
the [CMake documentation](https://cmake.org/cmake/help/latest/) for more information.

This will use CMake's default build file generator for your current platform and build the project in Release (
optimized) configuration. If the build was successful, you should have an executable in the build directory.

### Install projectMSDL

While you can run projectMSDL directly from the build directory, it's recommended to install the project. This will copy
all required files, including a default configuration file, into the installation dir.

You can set the installation target path in the first CMake command which configures the build. The `install` target
will then copy everything under this directory:

```shell
mkdir cmake-build
cmake -S . -B cmake-build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/path/to/install/dir
cmake --build cmake-build --config Release --target install
```

### Run directly from the build dir

You should have a directory of visual presets you wish to use. You can fetch a giant trove of curated
presets [here](https://github.com/projectM-visualizer/presets-cream-of-the-crop). You will also need textures (images)
used in many presets. The projectM team has
assembled [a pack of textures](https://github.com/projectM-visualizer/presets-milkdrop-texture-pack), which covers the
needs of most presets.

If you want to run the executable from the build dir, you'll need to pass any non-default settings via arguments. You
can also create a user configuration file in your user's home directory. Depending on the platform, this will be:

- Windows: `%APPDATA%\projectM\projectMSDL.properties`
- Linux:
  - If `XDG_CONFIG_HOME` env var is non-empty: `$XDG_CONFIG_HOME/projectM/projectMSDL.properties`
  - Otherwise: `~/.config/projectM/projectMSDL.properties`
- macOS: `~/Library/Preferences/projectM/projectMSDL.properties`

You can copy the [config file template](src/resources/projectMSDL.properties.in) there and change anything in `@@`.

Depending on the build system, you'll find the projectM executable in `cmake-build/src/`, or a subdirectory with the
name of your build type (`Release`, `Debug` and so on).

If you're not using a config file, provide the presets and texture paths you wish to use when starting projectMSDL:

```shell
cmake-build/src/projectMSDL --presetPath /path/to/presets-cream-of-the-crop --texturePath /path/to/textures
```

Press ESC to toggle the UI.

## Developing

This project uses CMake, which can generate project files for your favorite IDE or build system

### Windows

To generate a Visual Studio 2022 project for Win64 and build for Release:

```shell
mkdir cmake-build
cmake -G "Visual Studio 17 2022" -A x64 -S . -B cmake-build
cmake --build cmake-build --config Release
```

### Linux

To generate a Makefile project build for Release:

```shell
mkdir cmake-build
cmake -G "Unix Makefile" -S . -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build
```

To generate a Ninja project build for Release:

```shell
mkdir cmake-build
cmake -G "Ninja" -S . -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build
```

### macOS

To generate an Xcode project and build for Release:

```shell
mkdir cmake-build
cmake -G Xcode -S . -B cmake-build
cmake --build cmake-build --config Release
```
