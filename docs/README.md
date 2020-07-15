# openfx-text

Alternative internationalized OpenFX text generator for DaVinci Resolve and Fusion Studio.

## Windows

Binaries for Windows 7/8/10 are available on ... (TBD)

## Linux

### Requirements:

 * cmake and pkg-config
 * OpenGL development files and libraries
 * fontconfig development files and libraries
 * pango(cairo) development files and libraries

On Ubuntu install the following packages:

```
sudo apt install build-essential cmake pkg-config libfontconfig1-dev libcairo2-dev libpango1.0-dev libgl-dev
```

### Build and install:

```
git clone https://github.com/fxarena/openfx-text
cd openfx-text
git submodule update --init
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/OFX/Plugins ..
make && sudo make install
```
