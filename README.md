# OpenixCard

Open Source Version of Allwinner PhoenixCard

[![CMake](https://github.com/YuzukiTsuru/OpenixCard/actions/workflows/cmake.yml/badge.svg)](https://github.com/YuzukiTsuru/OpenixCard/actions/workflows/cmake.yml)

# Usage

```
OpenixCard v0.0.1 alpha 
YuzukiTsuru <GloomyGhost@GloomyGhost.com>

usage:
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
sudo apt install build-essential autoconf libconfuse-dev

# Make build directory
mkdir build
cd build

# Make
cmake .. && make -j
```
