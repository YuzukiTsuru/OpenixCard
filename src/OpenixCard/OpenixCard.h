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

class OpenixCard {
public:
    OpenixCard(int argc, char **argv);

private:
    std::string input_file;
    std::string temp_file_path;
    std::string output_file_path;

    enum OpenixCardOperator {
        PACK,
        UNPACK,
        DUMP,
        SIZE
    };

    enum OpenixCardOperatorExt {
        NONE,
        CFG,
    };

    OpenixCardOperator mode;
    OpenixCardOperatorExt mode_ext;
    bool is_absolute = false;

private:
    static void show_logo();

    static void check_file(const std::string& file_path);
private:
    void parse_args(int argc, char **argv);

    void pack();

    void unpack_target_image();

    void dump_and_clean();

    void save_cfg_file();
};


#endif //OPENIXCARD_OPENIXCARD_H
