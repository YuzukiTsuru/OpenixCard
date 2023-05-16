# OpenixCard

Open Source Version of Allwinner PhoenixCard to Dump, Unpack, Flash Allwinner IMG Files on Linux

[![forthebadge](https://forthebadge.com/images/badges/made-with-c-plus-plus.svg)](https://forthebadge.com)
[![forthebadge](https://forthebadge.com/images/badges/made-with-c.svg)](https://forthebadge.com)
[![forthebadge](https://forthebadge.com/images/badges/powered-by-black-magic.svg)](https://forthebadge.com)
[![forthebadge](https://forthebadge.com/images/badges/uses-git.svg)](https://forthebadge.com)

[![CMake](https://github.com/YuzukiTsuru/OpenixCard/actions/workflows/cmake.yml/badge.svg)](https://github.com/YuzukiTsuru/OpenixCard/actions/workflows/cmake.yml)

## Usage

```
 _____             _     _____           _ 
|     |___ ___ ___|_|_ _|     |___ ___ _| |
|  |  | . | -_|   | |_'_|   --| .'|  _| . |
|_____|  _|___|_|_|_|_,_|_____|__,|_| |___|
      |_| 
Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>

Usage: OpenixCard [options] input 

Positional arguments:
input           Input image file or directory path [required]

Optional arguments:
-h --help       shows help message and exits [default: false]
-v --version    prints version information and exits [default: false]
-u --unpack     Unpack Allwinner Image to folder [default: false]
-d --dump       Convert Allwinner image to regular image [default: false]
-c --cfg        Get Allwinner image partition table cfg file (use together with unpack) [default: false]
-p --pack       pack dumped Allwinner image to regular image from folder (needs cfg file) [default: false]
-s --size       Get the accurate size of Allwinner image [default: false]

eg.:
OpenixCard -u  <img>   - Unpack Allwinner image to target
OpenixCard -uc <img>   - Unpack Allwinner image to target and generate Allwinner image partition table cfg
OpenixCard -d  <img>   - Convert Allwinner image to regular image
OpenixCard -p  <dir>   - pack dumped Allwinner image to regular image from folder
OpenixCard -s  <img>   - Get the accurate size of Allwinner image
```

## Download
### ArchLinux
OpenixCard Now available at [AUR](https://aur.archlinux.org/packages/openixcard) [#3](https://github.com/YuzukiTsuru/OpenixCard/issues/3#issuecomment-1135317155)
```
yay -S openixcard
```

### Other Linux
You can find the new release file:  
https://github.com/YuzukiTsuru/OpenixCard/releases

## Build from source

```
# Download the source code
git clone --recursive --depth 1 https://github.com/YuzukiTsuru/OpenixCard

# Download the depends
sudo apt install cmake build-essential automake autoconf libconfuse-dev pkg-config

# Make build directory
mkdir build
cd build

# Make
cmake .. && make -j
```

> Note: Ubuntu 20.04 compilation will report an error, This is caused by the bug of ar, you can compile and install the new version.

```
sudo apt-get install texinfo

wget https://ftp.gnu.org/gnu/binutils/binutils-2.38.tar.xz && \
  tar xvf binutils-2.38.tar.xz && \
  cd binutils-2.38 && \
  ./configure --prefix=/usr/local && \
  make
  
sudo make install
```

## LICENSE
```
GNU GENERAL PUBLIC LICENSE Version 2, June 1991
                       
Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

See README and LICENSE for more details.
 ```
