/*
 * main.cpp
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#include <iostream>
#include <argparse/argparse.hpp>

#include "exception.h"
#include "config.h"
#include "FEX2CFG.h"

extern "C" {
#include "OpenixIMG.h"
}

int main(int argc, char *argv[]) {
    argparse::ArgumentParser parser("OpenixCard", PROJECT_VER);
    parser.add_argument("write", "-w", "--write")
            .help("Write Allwinner Image to SD Card");
    parser.add_argument("dump", "-d", "--dump")
            .help("Convert Allwinner image to regular image");

    FEX2CFG fex2Cfg("../test/tina_d1-lichee_rv_uart0.img.dump");
    fex2Cfg.save_file();

    return 0;
}