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


std::string gen_linux_cfg_from_fex_map(const inicpp::config &fex) {
    partition_table_struct patab;
    std::string cfg_data;
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
            }
            if (patab.downloadfile.empty())
                cfg_data += "\t\timage = \"blank.fex\"\n";
            else
                cfg_data += "\t\timage = " + patab.downloadfile + "\n";
            cfg_data += "\t\tsize = " + tostring(patab.size / 2) + "K\n";
            cfg_data += "\t}\n";
        }
    }
    return cfg_data;
}

#endif //OPENIXCARD_LINUX_H
