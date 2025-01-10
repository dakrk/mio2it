# Building

## Linux

You will need at least GCC 12 or Clang 14.

### Installing dependencies

#### Arch Linux

```bash
sudo pacman -S base-devel git cmake
```

#### Debian & Ubuntu

You should be using at least Debian 12 or Ubuntu 23 as to get a recent GCC version.

```bash
sudo apt install build-essential git cmake
```

### Compiling

```bash
git clone https://github.com/dakrk/mio2it
cd mio2it
cmake -B build
cmake --build build -j

# And if successful, you should be able to run:
./build/mio2it
```

## Windows (MSYS2 UCRT64)

### Installing dependencies

```bash
pacman -Syu
pacman -S base-devel git mingw-w64-ucrt-x86_64-{toolchain,cmake}
```

### Compiling

```bash
git clone https://github.com/dakrk/mio2it
cd mio2it
cmake -B build
cmake --build build -j

# And if successful, you should be able to run:
./build/mio2it.exe
```

## Windows (Visual Studio)

You will need at least Visual Studio 2022, with C++ and CMake support enabled.

### Compiling

You can then open Visual Studio, and either try the "Clone a repository" option, or try opening a command prompt with Git available and running the following:

```shell
git clone https://github.com/dakrk/mio2it
```

Then in Visual Studio, click the "Open a local folder" option and navigate to the folder you cloned to and select it.

Once the main IDE window opens, CMake should start generating. At the top you should be able to select between a debug and release build. You can then find the "Build" menu item at the top, and click "Build All" which should show after a successful generation.

## MacOS

Not entirely sure, sorry. I would expect it to be the exact same as the Linux instructions, except you use the [Homebrew](https://brew.sh) package manager to get the dependencies.
