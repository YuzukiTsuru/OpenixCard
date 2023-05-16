/*
 * linux.cpp
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#ifndef OPENIXCARD_LINUX_H
#define OPENIXCARD_LINUX_H

#include <chip.h>

std::string gen_linux_cfg_from_fex_map(const inicpp::config &fex, partition_table_type type) {
    partition_table_struct patab;
    linux_compensate compensate;
    std::string cfg_data;

    // check type
    for (auto &sect: fex) {
        patab = {}; // reflash struce
        for (auto &opt: sect) {
            if (opt.get_name() == "name") {
                patab.name = opt.get<inicpp::string_ini_t>();
            } else if (opt.get_name() == "size") {
                patab.size = opt.get<inicpp::unsigned_ini_t>();
            } else if (opt.get_name() == "downloadfile") {
                patab.downloadfile = opt.get<inicpp::string_ini_t>();
            } else if (opt.get_name() == "user_type") {
                patab.user_type = opt.get<inicpp::unsigned_ini_t>();
            } else {
                // Droped.
            }
        }
        if (patab.name == "boot-resource") {
            type = partition_table_type::hybrid;
        } else if (patab.downloadfile == "\"boot-resource.fex\"") {
            type = partition_table_type::hybrid;
        }
    }

    cfg_data += "\thdimage{\n";

    switch(type){
        case partition_table_type::hybrid:
            cfg_data += "\t\tpartition-table-type = \"hybrid\"\n";
            break;
        case partition_table_type::gpt:
            cfg_data += "\t\tpartition-table-type = \"gpt\"\n";
            break;
        case partition_table_type::mbr:
            cfg_data += "\t\tpartition-table-type = \"mbr\"\n";
            break;
        default:
            cfg_data += "\t\tpartition-table-type = \"gpt\"\n";
            break;
    }

    cfg_data += "\t\tgpt-location = " + std::to_string(compensate.gpt_location / 0x100000) + "M\n";
    cfg_data += "\t}\n";

    // add sdcard boot image
    cfg_data += "\tpartition boot0 {\n"
                "\t\tin-partition-table = \"no\"\n"
                "\t\timage = \"boot0_sdcard.fex\"\n"
                "\t\toffset = " + std::to_string(compensate.boot0_offset / 0x400) + "K\n" +
                "\t}\n";

    cfg_data += "\tpartition boot-packages {\n"
                "\t\tin-partition-table = \"no\"\n"
                "\t\timage = \"boot_package.fex\"\n"
                "\t\toffset = " + std::to_string(compensate.boot_packages_offset / 0x400) + "K\n" +
                "\t}\n";

    for (auto &sect: fex) {
        patab = {}; // reflash struce
        for (auto &opt: sect) {
            if (opt.get_name() == "name") {
                patab.name = opt.get<inicpp::string_ini_t>();
            } else if (opt.get_name() == "size") {
                patab.size = opt.get<inicpp::unsigned_ini_t>();
            } else if (opt.get_name() == "downloadfile") {
                patab.downloadfile = opt.get<inicpp::string_ini_t>();
            } else if (opt.get_name() == "user_type") {
                patab.user_type = opt.get<inicpp::unsigned_ini_t>();
            } else {
                // Droped.
            }
        }
        if (patab.name != "UDISK") {
            cfg_data += "\tpartition " + patab.name + " {\n";
            if (patab.name == "boot-resource") {
                cfg_data += "\t\tpartition-type = 0xC\n";
            } else if (patab.downloadfile == "\"boot-resource.fex\"") {
                cfg_data += "\t\tpartition-type = 0xC\n";
            }
            if (patab.downloadfile.empty())
                cfg_data += "\t\timage = \"blank.fex\"\n";
            else
                cfg_data += "\t\timage = " + patab.downloadfile + "\n";
            cfg_data += "\t\tsize = " + std::to_string(patab.size / 2) + "K\n";
            cfg_data += "\t}\n";
        }
    }
    return cfg_data;
}

uint linux_common_fex_compensate() {
    linux_compensate compensate;
    return compensate.gpt_location / 0x400 + compensate.boot0_offset / 0x400 + compensate.boot_packages_offset / 0x400;
}

#endif //OPENIXCARD_LINUX_H
