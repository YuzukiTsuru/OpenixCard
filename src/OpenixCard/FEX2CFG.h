/*
 * FEX2CFG.h
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#ifndef OPENIXCARD_FEX2CFG_H
#define OPENIXCARD_FEX2CFG_H

#include <iostream>

#include <inicpp/inicpp.h>

#include "AW_IMG_PARA.h"

class FEX2CFG {
public:
    explicit FEX2CFG(const std::string &dump_path);

    void save_file(const std::string &file_path);

    [[maybe_unused]] void save_file();

    [[maybe_unused]] std::string get_cfg();

    [[maybe_unused]] std::string get_image_name();

    void print_partition_table();

private:
    AW_IMG_PARA awImgPara;
    inicpp::config fex_classed;
    std::string awImgFex = {};
    std::string awImgCfg = {};
    std::string awImgFexClassed = {};

    void open_file(const std::string &file_path);

    void classify_fex();

    void parse_fex();

    void gen_cfg();
};


#endif //OPENIXCARD_FEX2CFG_H
