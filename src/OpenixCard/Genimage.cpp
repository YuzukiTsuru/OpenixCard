/*
 * Genimage.cpp
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#include <utility>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <subprocess.hpp>

#include "Genimage.h"
#include "exception.h"


[[maybe_unused]] Genimage::Genimage(std::string config_path, std::string image_path, std::string output_path)
        : config_path(std::move(config_path)), image_path(std::move(image_path)), output_path(std::move(output_path)) {
    this->genimage_bin = std::filesystem::current_path() / "bin/genimage-src";
    // generate blank.fex file for commented partition
    generate_blank_fex();
    // call genimage-src
    run_genimage();
}

void Genimage::run_genimage() {
    auto temp_dir = std::vector<std::string>{};
    for (int i = 0; i < 2; ++i)
        temp_dir.emplace_back([]() -> std::string {
            auto dir_name_str = std::string(subprocess::check_output({"mktemp", "-d"}).buf.data());
            size_t start_pos = 0;
            while ((start_pos = dir_name_str.find('\n', start_pos)) != std::string::npos) {
                dir_name_str.replace(start_pos, 1, "");
                start_pos += 1;
            }
            return dir_name_str;
        }());

    auto process = subprocess::Popen({genimage_bin,
                                      "--config", this->config_path,
                                      "--rootpath", temp_dir[0],
                                      "--tmppath", temp_dir[1],
                                      "--inputpath", this->image_path,
                                      "--outputpath", this->output_path
                                     }, subprocess::output{subprocess::PIPE}, subprocess::error{subprocess::PIPE});
    process.wait();
    auto output = process.communicate().first;
    auto error = process.communicate().second;
    this->status = process.retcode();
}

[[maybe_unused]] void Genimage::print() {
    std::cout << "\tconfig_path: " << this->config_path << std::endl;
    std::cout << "\timage_path: " << this->image_path << std::endl;
    std::cout << "\toutput_path: " << this->output_path << std::endl;
}

void Genimage::generate_blank_fex() {
    std::ofstream out(this->image_path + "/blank.fex");
    // File not open, throw error.
    if (!out.is_open()) {
        throw file_open_error(this->image_path + "/blank.fex");
    }
    out << "blank.fex";
    out.close();
}

[[maybe_unused]] int Genimage::get_status() const {
    return this->status;
}
