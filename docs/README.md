# openfx-text

Alternative internationalized OpenFX text generator for DaVinci Resolve and Fusion Studio.

## Windows

Binaries for Windows 7/8/10 are available on ... (TBD)

## Linux

Requirements:

 * fontconfig development files and libraries
 * pango(cairo) development files and libraries

Build and install:
```
git clone https://github.com/fxarena/openfx-text
cd openfx-text
git submodule update --init
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/OFX/Plugins ..
make && sudo make install
```
