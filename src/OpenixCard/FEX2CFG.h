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
#include "payloads/chip.h"

class FEX2CFG {
public:
    explicit FEX2CFG(const std::string &dump_path);

    // save the configuration to the dump file path
    std::string save_file(const std::string &file_path);

    //save the configuration to local path
    [[maybe_unused]] void save_file();

    // load the configuration from the dump file
    [[maybe_unused]] std::string get_cfg();

    // get image name
    [[maybe_unused]] [[nodiscard]] std::string get_image_name() const;

    // get image real size
    [[maybe_unused]] uint get_image_real_size(bool print);

    // Print out the partition table
    [[maybe_unused]] void print_partition_table();

    // regenerate cfg file
    void regenerate_cfg_file(partition_table_type _type);

private:
    AW_IMG_PARA awImgPara;
    inicpp::config fex_classed;
    std::vector <u_int> partition_size_list;
    std::string awImgFex = {};
    std::string awImgCfg = {};
    std::string awImgFexClassed = {};
    partition_table_type type = partition_table_type::gpt;

    void open_file(const std::string &file_path);

    void classify_fex();

    void parse_fex();

    void gen_cfg();

    void get_partition_real_size();
};


#endif //OPENIXCARD_FEX2CFG_H
