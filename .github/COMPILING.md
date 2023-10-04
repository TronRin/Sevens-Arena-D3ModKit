## Compiling

The build system is based on CMake: http://cmake.org/

Required libraries are not part of the tree. These are:

- zlib
- OpenAL (OpenAL Soft required, Creative's and Apple's versions are made of fail)
- SDL v1.2 or 2.0 (2.0 recommended)
- libcurl (optional, required for server downloads)
- Optionally, on non-Windows: libbacktrace (usually linked statically)
  - sometimes (e.g. on debian-based distros like Ubuntu) it's part of libgcc (=> always available),
    sometimes (e.g. Arch Linux, openSUSE) it's in a separate package
  - If this is available, dhewm3 prints more useful backtraces if it crashes

For **UNIX-like systems**, these libraries need to be installed (including the
developer files). It is recommended to use the software management tools of
your OS (apt, dnf, portage, BSD ports, [Homebrew for macOS](http://brew.sh), ...).

For **Windows**, there are three options:

- Use the provided binaries (recommended, see below)
- Compile these libraries yourself
- Use [vcpkg](https://vcpkg.io/) to install the dependencies
    - Remember to set `CMAKE_TOOLCHAIN_FILE` as described in their [Getting Started Guide](https://vcpkg.io/en/getting-started.html)

Create a distinct build folder outside of this source repository and issue
the cmake command there, pointing it at the neo/ folder from this repository:

`cmake /path/to/repository/neo`

**macOS** users need to point CMake at OpenAL Soft (better solutions welcome):

`cmake -DOPENAL_LIBRARY=/usr/local/opt/openal-soft/lib/libopenal.dylib -DOPENAL_INCLUDE_DIR=/usr/local/opt/openal-soft/include /path/to/repository/neo`

Newer versions of Homebrew install openal-soft to another directory, so use this instead:

`cmake -DOPENAL_LIBRARY="/opt/homebrew/opt/openal-soft/lib/libopenal.dylib" -DOPENAL_INCLUDE_DIR="/opt/homebrew/opt/openal-soft/include" /path/to/repo/neo`

### Compiling example using Ubuntu

Should be the same for Debian and other Debian-derivatives, but apart from the first step (installing
build dependecies) it should be the same on other Linux distros and even other UNIX-likes in general.

Open a terminal and follow these steps:

* Install build dependencies:  
  `sudo apt install git cmake build-essential libsdl2-dev libopenal-dev zlib1g-dev libcurl4-openssl-dev`  
    - The build-essential package on Debian/Ubuntu/... installs some basics for compiling code
      like GCC (incl. g++), GNU Make and the glibc development package
    - Instead of libcurl4-openssl-dev, other libcurl*-dev packages should also work - or none at all, curl is optional.
    - Not strictly necessary, but I recommend making libbacktrace available. On distributions not
      based on Debian, you may have to manually install some kind of libbacktrace development package.
* Use git to get the code from Github (alternatively you can also download the code as an archive and extract that):  
  `git clone https://github.com/dhewm/dhewm3.git`
* Change into the dhewm3 directory, create a directory to build in and change into the build directory:  
  `cd dhewm3` then `mkdir build` then `cd build`
* Create a Makefile with CMake: `cmake ../neo/`
    - You can set different options for CMake with arguments like `-DDEDICATED=ON` (to enable the dedicated server).
      You can show a list of supported options by running `cmake -LH ../neo/`. You can run CMake again
      with another `-DFOO=BAR` option to change that option (previously set options are remembered,
      unless you remove all contents of the build/ dir).
    - You could also install the **cmake-qt-gui** package and run `cmake-gui ../neo/`, which will let
      you configure the build in a GUI instead of using `-D` commandline-arguments.
* Compile dhewm3: `make -j8`
    - `-j8` specifies the number of compiler processes to run in parallel (8 in this example),
      it makes sense to use the number of CPU threads (or cores) in your system.

When all steps are done and no errors occurred, you should be able to run dhewm3 right there, like:  
`./dhewm3 +set fs_basepath /path/to/your/doom3/`  
*Replace `/path/to/your/doom3/` with the path to your Doom3 installation (that contains `base/` with
`pak000.pk4` to `pak008.pk4`)*

### Using the provided Windows binaries

Get a clone of the latest binaries here: https://github.com/dhewm/dhewm3-libs

There are two subfolders:

- 32-bit binaries are located in `i686-w64-mingw32`
- 64-bit binaries are located in `x86_64-w64-mingw32`

Issue the appropriate command from the build folder, for example (for VS2019 and 32bit):

`cmake -G "Visual Studio 16 2019" -A Win32 -DDHEWM3LIBS=/path/to/dhewm3-libs/i686-w64-mingw32 /path/to/repository/neo`

For 64bit dhewm3 binaries, use `-A x64` and `/path/to/dhewm3-libs/x86_64-w64-mingw32` instead (note that the official dhewm3 binaries for Windows are 32bit).  
For Visual Studio 2022 it's `"Visual Studio 17 2022"`.

For 32bit MinGW use:
`cmake -G "MinGW Makefiles" -DDHEWM3LIBS=/path/to/dhewm3-libs/i686-w64-mingw32 /path/to/repository/neo`

The binaries are compatible with MinGW-w64 and all MSVC versions.

### Cross-compiling

For cross-compiling, a CMake Toolchain file is required.

For the MinGW-w64 toolchain `i686-w64-mingw32` on Ubuntu 12.04, it looks like:

```
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)

set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

Then point CMake at your toolchain file:
`cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/Toolchain.cmake -DDHEWM3LIBS=/path/to/dhewm3-libs/i686-w64-mingw32 /path/to/repository/neo`

If you want to build for x86_64 aka AMD64 aka x64, replace all instances of `i686`
in the toolchain file with `x86_64`.