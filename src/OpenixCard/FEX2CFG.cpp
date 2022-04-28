/*
 * FEX2CFG.cpp
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#include <string>
#include <sstream>

#include "FEX2CFG.h"
#include "exception.h"

#include <payloads/chip.h>

FEX2CFG::FEX2CFG(const std::string &dump_path) {
    // parse basic files
    awImgPara.partition_table_fex_path = dump_path + '/' + awImgPara.partition_table_fex;
    awImgPara.image_name = dump_path.substr(dump_path.find_last_of('/') + 1, dump_path.length() - dump_path.find_last_of('/') + 1);
    awImgPara.image_name = awImgPara.image_name.substr(0, awImgPara.image_name.find('.'));
    awImgPara.partition_table_fex = awImgPara.image_name.substr(0, awImgPara.image_name.rfind('.')) + ".cfg";
    awImgPara.partition_table_cfg = awImgPara.image_name.substr(0, awImgPara.image_name.rfind('.')) + ".fex";

    // Parse File
    open_file(awImgPara.partition_table_fex_path);
    classify_fex();
    parse_fex();
    gen_cfg();
}

void FEX2CFG::save_file(const std::string &file_path) {
    std::ofstream out(file_path);
    // File not open, throw error.
    if (!out.is_open()) {
        throw file_open_error(file_path);
    }
    out << awImgCfg;
    out.close();
}

[[maybe_unused]] void FEX2CFG::save_file() {
    std::ofstream out(awImgPara.image_name + ".cfg");
    // File not open, throw error.
    if (!out.is_open()) {
        throw file_open_error(awImgPara.image_name + ".cfg");
    }
    out << awImgCfg;
    out.close();
}

void FEX2CFG::open_file(const std::string &file_path) {
    std::ifstream in;
    in.open(file_path, std::ios::in | std::ios::out | std::ios::binary);
    // File not open, throw error.
    if (!in.is_open()) {
        throw file_open_error(file_path);
    }
    awImgFex = std::string((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
    in.close();
}

/*
 * The partition table of Allwinner's IMAGEWTY is very special.
 * It is parsed based on INI but its Section is repeated, which will cause the parsing to fail。
 * so use this function to organize
 */

void FEX2CFG::classify_fex() {
    int occ = 0;
    std::string::size_type pos = 0;
    std::string _temp = awImgFex;

    _temp = _temp.substr(_temp.find("[partition_start]"));

    while ((pos = _temp.find("[partition]", pos)) != std::string::npos) {
        ++occ;
        pos += std::string("[partition]").length();
    }

    std::string _section, _less_out = _temp;
    for (int i = 0; i < occ; ++i) {
        _section = _less_out.substr(_less_out.rfind("[partition]") + std::string("[partition]").length());
        _less_out = _less_out.substr(0, _less_out.rfind("[partition]"));
        awImgFexClassed.insert(0, "[partition" + std::to_string(occ - i) + "]" + _section);
    }
}

void FEX2CFG::parse_fex() {
    fex_classed = inicpp::parser::load(awImgFexClassed);
}

[[maybe_unused]] std::string FEX2CFG::get_image_name() {
    return awImgPara.image_name;
}

[[maybe_unused]] std::string FEX2CFG::get_cfg() {
    return awImgCfg;
}

void FEX2CFG::gen_cfg() {
    awImgCfg += "image ";
    awImgCfg += awImgPara.image_name;
    awImgCfg += ".img {\n";
    awImgCfg += "\thdimage{\n";
    awImgCfg += "\t\tpartition-table-type = \"hybrid\"\n";
    awImgCfg += "\t\tgpt-location = 1M\n";
    awImgCfg += "\t}\n";

    // add sdcard boot image
    awImgCfg += "\tpartition boot0 {\n"
                "\t\tin-partition-table = \"no\"\n"
                "\t\timage = \"boot0_sdcard.fex\"\n"
                "\t\toffset = 8K\n"
                "\t}\n";

    awImgCfg += "\tpartition boot-packages {\n"
                "\t\tin-partition-table = \"no\"\n"
                "\t\timage = \"boot_package.fex\"\n"
                "\t\toffset = 16400K\n"
                "\t}\n";

    // For Debug
    print_partition_table();

    // Generate file from FEX
    awImgCfg += gen_linux_cfg_from_fex_map(fex_classed);

    awImgCfg += "}";
}

void FEX2CFG::print_partition_table() {
    for (auto &sect: fex_classed) {
        std::cout << "  Partition: '" << sect.get_name() << "'" << std::endl;
        // Iterate through options in a section
        for (auto &opt: sect) {
            std::cout << "    Option: '" << opt.get_name() << "' with value(s): ";
            std::cout << "'" << opt.get<inicpp::string_ini_t>() << "'" << std::endl;
        }
    }
}
