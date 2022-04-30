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

Usage: OpenixCard [options] 

Optional arguments:
-h --help       shows help message and exits [default: false]
-v --version    prints version information and exits [default: false]
-u --unpack     Unpack Allwinner Image to folder [default: false]
-d --dump       Convert Allwinner image to regular image [default: false]
-c --cfg        Get Allwinner image partition table cfg file [default: false]
-i --input      Input Allwinner image file [required]
-o --output     Output file path [default: "output"]


eg.

OpenixCard                                - TUI Interface -> NOT AVALIABLE
OpenixCard -u -i <img> -o <target dir>    - Unpack Allwinner image to target
OpenixCard -c -i <img> -o <target dir>    - Generate Allwinner image partition table cfg
OpenixCard -d -i <img> -o <target dir>    - Convert Allwinner image to regular image
```

## Build from source

```
# Download the source code
git clone --recursive https://github.com/YuzukiTsuru/OpenixCard

# Download the depends
sudo apt install cmake build-essential autoconf libconfuse-dev

# Make build directory
mkdir build
cd build

# Make
cmake .. && make -j
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
