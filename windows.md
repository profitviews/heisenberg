### Windows

**Some further work required**

With some work a manual build with Msys2 was possible, however there were runtime problems.  It is probably not difficult to complete this work.

1. Msys
   To build on Windows you need [Msys2](https://www.msys2.org/).  Follow the instructions there.

2. Msys will give you `pacman` package manager, `g++` and other essential programs.
   Use `pacman` to get:
   `cmake`
   `mingw-w64-x86_64-boost`
   `mingw-w64-x86_64-poco`
