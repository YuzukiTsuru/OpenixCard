/*
 * Genimage.h
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#ifndef OPENIXCARD_GENIMAGE_H
#define OPENIXCARD_GENIMAGE_H

#include <iostream>


class Genimage {
public:
    [[maybe_unused]] Genimage(std::string config_path, std::string image_path, std::string output_path);

    [[maybe_unused]] void print();

    [[maybe_unused]] [[nodiscard]] int get_status() const;

private:
    std::string config_path;
    std::string name;
    std::string image_path;
    std::string output_path;
    std::string genimage_bin = "bin/genimage";

    int status = 0;

    // subprocess runner for genimage
    void run_genimage();

    void generate_blank_fex();
};


#endif //OPENIXCARD_GENIMAGE_H
