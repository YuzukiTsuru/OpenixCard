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

#include "OpenixIMG.h"
#include "FEX2CFG.h"

int main(int argc, char *argv[]) {
    FEX2CFG fex2Cfg("../test/tina_d1-lichee_rv_uart0.img.dump");
    fex2Cfg.save_file("my.cfg");
    return 0;
}