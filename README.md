# WSJTX JT4, jt65, jt9, FT4, FT8, C++ library
The fortran code is owned by Joe Taylor, K1JT.
Copyright (C) 2001 - 2021 by Joe Taylor, K1JT.
Copyright (C) 2023 PA0PHH c++ part

This library is an attempt to package wsjtx fortran code into a c++ callable library
It is intended for implementation in SDR tranceivers running in linux (raspberry pi)
The library can be build, on linux, by downloading and building the cmake file.

## Install and compile with cmake
To use the library dowload git repo, there is an dependency to cmake, gfortran, and fftw3
```
sudo apt install cmake g++
sudo apt install gfortran
sudo apt install libfftw3-3
sudo apt install libfftw3-dev
```

## Compile with cmake (assuming all libraries are available)
```
git clone the repo
cd wsjtx_lib
mkdir build
cd build
cmake ..
make
sudo make install
```
To use the library 
```
#include <wsjtx_lib.h>

add to the linker -l"wsjtx_lib" -l"pthread" -l"fftw3f" -l"fftw3f_threads" -l"gfortran"
```
create wsjtx_lib class and use the methods

ToDo:
- Expose ... FST4, WSPR and other protocols

Done:
- FT8 decoder
- FT4 decoder
- FT8 Encoder
- FT8 Encoder