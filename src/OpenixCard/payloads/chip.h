/*
 * chip.h
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#ifndef OPENIXCARD_CHIP_H
#define OPENIXCARD_CHIP_H

#include <iostream>
#include <sstream>
#include <inicpp/inicpp.h>

typedef struct partition_table_struct {
    std::string name;
    uint64_t size;
    std::string downloadfile;
    uint64_t user_type;
} partition_table_struct;

typedef struct linux_compensate {
    uint64_t gpt_location = 0x100000;
    uint64_t boot0_offset = 0x2000;
    uint64_t boot_packages_offset = 0x1004000;
} linux_compensate;

enum partition_table_type {
    hybrid,
    gpt,
    mbr
};

std::string gen_linux_cfg_from_fex_map(const inicpp::config &fex, partition_table_type type);

[[maybe_unused]] uint linux_common_fex_compensate();

#endif //OPENIXCARD_CHIP_H
