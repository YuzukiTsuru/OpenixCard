# OpenixCard

Open Source Version of Allwinner PhoenixCard to Dump, Unpack, Flash Allwinner IMG Files

[![CMake](https://github.com/YuzukiTsuru/OpenixCard/actions/workflows/cmake.yml/badge.svg)](https://github.com/YuzukiTsuru/OpenixCard/actions/workflows/cmake.yml)

# Usage

```
OpenixCard                                - TUI Interface -> NOT AVALIABLE
OpenixCard -u -i <img> -o <target dir>    - Unpack Allwinner image to target
OpenixCard -c -i <img> -o <target dir>    - Generate Allwinner image partition table cfg
OpenixCard -d -i <img> -o <target dir>    - Convert Allwinner image to regular image
```

# Build from source
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

# LICENSE
```
GNU GENERAL PUBLIC LICENSE Version 2, June 1991
                       
Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

See README and LICENSE for more details.
 ```
