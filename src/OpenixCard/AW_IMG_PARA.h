/*
 * AW_IMG_PARA.h
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#ifndef OPENIXCARD_AW_IMG_PARA_H
#define OPENIXCARD_AW_IMG_PARA_H

#include <vector>
#include <iostream>

class AW_IMG_PARA {
public:
    std::string image_name = {};
    std::string partition_table_fex = "sys_partition.fex";
    std::string partition_table_fex_path = "sys_partition.fex";
    std::string partition_table_cfg = "sys_partition.cfg";
};

#endif //OPENIXCARD_AW_IMG_PARA_H
