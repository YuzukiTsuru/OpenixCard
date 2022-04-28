# OpenixCard

Open Source Version of Allwinner PhoenixCard

# Usage

```
OpenixCard v0.0.1 alpha 
YuzukiTsuru <GloomyGhost@GloomyGhost.com>

usage:
    OpenixCard                                               - TUI Interface
    OpenixCard -w -i <img> -o <target> -b <bs size>          - Write Allwinner image to target
    OpenixCard -d -i <img> -o <target>                       - Convert Allwinner image to regular image
```

# Build from source
```
# Download the source code
git clone --recursive https://github.com/YuzukiTsuru/OpenixCard

# Download the depends
sudo apt install build-essential autoconf 

# Make build directory
mkdir build
cd build

# Make
cmake .. && make -j
```
