/*
 * GenIMG.h
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#ifndef OPENIXCARD_GENIMG_H
#define OPENIXCARD_GENIMG_H

#include <iostream>

class GenIMG {
public:
    [[maybe_unused]] GenIMG(std::string config_path, std::string image_path, std::string output_path);

    [[maybe_unused]] void print();

    [[maybe_unused]] [[nodiscard]] int get_status() const;

private:
    std::string config_path;
    std::string name;
    std::string image_path;
    std::string output_path;

    int status = 0;

private:
    // subprocess runner for genimage-src
    void run_genimage();

    void generate_blank_fex();
};


#endif //OPENIXCARD_GENIMG_H
