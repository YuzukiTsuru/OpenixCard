/*
 * OpenixCard.h
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#ifndef OPENIXCARD_OPENIXCARD_H
#define OPENIXCARD_OPENIXCARD_H

#include <iostream>
#include <vector>

class OpenixCard {
public:
    OpenixCard(int argc, char **argv);

private:
    std::vector<std::string> input_file_vector;
    std::string input_file;
    std::string temp_file_path;
    std::string output_file_path;

    enum OpenixCardOperator {
        NONE,
        PACK,
        UNPACK,
        UNPACKCFG,
        DUMP,
        SIZE,
    };

    OpenixCardOperator mode;
    bool is_absolute = false;

private:
    static void show_logo();

    static void check_file(const std::string& file_path);
private:
    void pack();

    void unpack_target_image();

    void dump_and_clean();

    void save_cfg_file();

    void get_real_size();
};


#endif //OPENIXCARD_OPENIXCARD_H
