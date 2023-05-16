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
#include <string_view>
#include <sstream>
#include <iomanip>

#include <ColorCout.hpp>

#include "FEX2CFG.h"
#include "LOG.h"
#include "exception.h"
#include "payloads/chip.h"

FEX2CFG::FEX2CFG(const std::string &dump_path) {
    // parse basic files
    awImgPara.partition_table_fex_path = dump_path + '/' + awImgPara.partition_table_fex;
    awImgPara.image_name = dump_path.substr(dump_path.find_last_of('/') + 1, dump_path.length() - dump_path.find_last_of('/') + 1);
    awImgPara.image_name = awImgPara.image_name.substr(0, awImgPara.image_name.find('.'));
    awImgPara.partition_table_fex = awImgPara.image_name.substr(0, awImgPara.image_name.rfind('.')) + ".fex";
    awImgPara.partition_table_cfg = awImgPara.image_name.substr(0, awImgPara.image_name.rfind('.')) + ".cfg";

    // Parse File
    open_file(awImgPara.partition_table_fex_path);
    classify_fex();
    
    parse_fex();
    gen_cfg();
}

std::string FEX2CFG::save_file(const std::string &file_path) {
    auto path = file_path + "/" + awImgPara.partition_table_cfg;
    std::ofstream out(path);
    // File not open, throw error.
    if (!out.is_open()) {
        throw file_open_error(path);
    }
    out << awImgCfg;
    out.close();
    return path;
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
    std::istringstream _temp_aw_img_fex(awImgFex);
    std::string _temp = {};
    std::string _temp_str = {};

    // clean the comment message
    while(std::getline(_temp_aw_img_fex, _temp_str)){
        if (_temp_str.substr(0, 1) != ";"){
            _temp += _temp_str + "\n";
        }
    }

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
    // Quick Fix for #26
    try {
        fex_classed = inicpp::parser::load(awImgFexClassed);
    } catch(const inicpp::ambiguity_exception &e) {
        LOG::ERROR(std::string("Partition table error, bad format. ") + std::string(e.what()));
        LOG::ERROR(std::string("Your Partition table: "));
        std::cout << awImgFexClassed << std::endl;
        LOG::ERROR(std::string("Please fix in `sys_partition.fex` and re-pack with Allwinner BSP"));
        std::exit(-1);
    }
}

[[maybe_unused]] std::string FEX2CFG::get_image_name() const {
    return awImgPara.image_name;
}

[[maybe_unused]] std::string FEX2CFG::get_cfg() {
    return awImgCfg;
}

uint FEX2CFG::get_image_real_size(bool print) {
    get_partition_real_size();
    uint total_size = 0;
    for (auto &size: partition_size_list) {
        total_size += size;
    }
    if (print) {
        LOG::DATA("Partition Table: ");
        print_partition_table();
    }
    return total_size + linux_common_fex_compensate();
}

void FEX2CFG::gen_cfg() {
    // Generate Prefix
    awImgCfg += "image ";
    awImgCfg += awImgPara.image_name;
    awImgCfg += ".img {\n";

    // For Debug
    print_partition_table();

    // Generate file from FEX
    awImgCfg += gen_linux_cfg_from_fex_map(fex_classed, type);

    awImgCfg += "}";
}

[[maybe_unused]] void FEX2CFG::print_partition_table() {
    std::cout << cc::green;
    for (auto &sect: fex_classed) {
        std::cout << std::left << std::setw(13) << "  Partition: '";
        // Iterate through options in a section
        for (auto &opt: sect) {
            if (opt.get_name() == "name") {
                auto name = opt.get<inicpp::string_ini_t>();
                std::cout << std::left << std::setw(18) << name + "'";
                if (name == "UDISK") {
                    std::cout << "Remaining space.";
                }
            } else if (opt.get_name() == "size") {
                std::cout << std::left << std::setw(9) << static_cast<double>(opt.get<inicpp::unsigned_ini_t>()) / 2 / 0x300 << "MB - "
                          << std::left << std::setw(7) << opt.get<inicpp::unsigned_ini_t>() / 2 << "KB";
            }
        }
        std::cout << std::endl;
    }
    std::cout << cc::reset;
}

void FEX2CFG::get_partition_real_size() {
    for (auto &sect: fex_classed) {
        for (auto &opt: sect) {
            if (opt.get_name() == "size") {
                partition_size_list.emplace_back(opt.get<inicpp::unsigned_ini_t>() / 2);
            }
        }
    }
}

void FEX2CFG::regenerate_cfg_file(partition_table_type _type) {
    awImgCfg = "";
    this->type = _type;
    gen_cfg();
}

