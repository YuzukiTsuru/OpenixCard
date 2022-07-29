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


#include "OpenixCard.h"
#include "LOG.h"

int main(int argc, char *argv[]) {
    try {
        OpenixCard openixCard(argc, argv);
    } catch (const std::runtime_error& error) {
        LOG::ERROR(error.what());
        return -1;
    }
    return 0;
}
