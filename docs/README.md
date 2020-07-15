# openfx-text

Advanced internationalized [OpenFX](http://openeffects.org/) text generator for [Natron](https://natrongithub.github.io/), [The Foundry](https://www.foundry.com/) [Nuke](https://www.foundry.com/products/nuke), [Blackmagic Design](https://blackmagicdesign.com) [DaVinci Resolve](https://www.blackmagicdesign.com/products/davinciresolve) / [Fusion Studio](https://www.blackmagicdesign.com/no/products/fusion).

This project is a continuation of the Text node in Natron (from [openfx-arena](https://github.com/NatronGitHub/openfx-arena)), but written from scratch with focus on cross-application support and more experimental features.

Currently under development, not end-user ready yet.

## Windows

TBD

## macOS

TBD

## Linux

### Requirements:

 * cmake and pkg-config
 * OpenGL development files and libraries
 * fontconfig development files and libraries
 * pangocairo development files and libraries

On Ubuntu install the following packages:

```
sudo apt install build-essential cmake pkg-config libfontconfig1-dev libcairo2-dev libpango1.0-dev libgl-dev
```

### Build and install:

```
git clone https://github.com/fxarena/openfx-text
cd openfx-text
git submodule update -i
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/OFX/Plugins ..
make && sudo make install
```
