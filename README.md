# WSJTX JT4, jt65, jt9, FT4, FT8, C++ library
The fortran code is owned by Joe Taylor, K1JT.
Copyright (C) 2001 - 2021 by Joe Taylor, K1JT.

This library is an attempt to package wsjtx fortran code into a c++ callable library
It is intended for implementation in SDR tranceivers running in linux (raspberry pi)
The library can be build, on linux, by downloading and building the cmake file.

to use the library dowload git repo, there is an dependency to cmake, gfortran, and fftw3
sudo apt install cmake g++
sudo apt install gfortran
sudo apt install libfftw3-3
sudo apt install libfftw3-dev

git clone https://github.com/paulh002/wsjtx_lib
cd wsjtx_lib
mkdir build
cd build
cmake ..
make
sudo make install

To use the library 
#include <wsjtx_lib.h>
create wsjtx_lib class and use the methods

ToDo:
- Expose ... FST4, WSPR and other protocols

Done:
- FT8 decoder
- FT4 decoder
- FT8 Encoder
- FT8 Encoder