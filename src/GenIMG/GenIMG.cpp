/*
 * GenIMG.cpp
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>

#include <ColorCout.hpp>
#include <subprocess.hpp>

#include "GenIMG.h"
#include "exception.h"

extern "C" {
#include "GenimageWrapper.h"
}

[[maybe_unused]] GenIMG::GenIMG(std::string config_path, std::string image_path, std::string output_path)
    : config_path(std::move(config_path))
    , image_path(std::move(image_path))
    , output_path(std::move(output_path))
{
    // generate blank.fex file for commented partition
    generate_blank_fex();

    generate_tmp_dir();
    // call genimage-src
    run_genimage();
}

void GenIMG::generate_tmp_dir()
{
    for (int i = 0; i < 2; ++i)
        temp_dir.emplace_back([]() -> std::string {
            auto dir_name_str = std::string(subprocess::check_output({ "mktemp", "-d" }).buf.data());
            size_t start_pos = 0;
            while ((start_pos = dir_name_str.find('\n', start_pos)) != std::string::npos) {
                dir_name_str.replace(start_pos, 1, "");
                start_pos += 1;
            }
            return dir_name_str;
        }());
}

void GenIMG::run_genimage()
{
    char arg0[] = "OpenixCard";
    char arg1[] = "--config";
    char arg2[] = "--rootpath";
    char arg3[] = "--tmppath";
    char arg4[] = "--inputpath";
    char arg5[] = "--outputpath";
    char* argv[] = {
        &arg0[0],
        &arg1[0], const_cast<char*>(this->config_path.c_str()),
        &arg2[0], const_cast<char*>(temp_dir[0].c_str()),
        &arg3[0], const_cast<char*>(temp_dir[1].c_str()),
        &arg4[0], const_cast<char*>(this->image_path.c_str()),
        &arg5[0], const_cast<char*>(this->output_path.c_str()),
        nullptr
    };

    int argc = static_cast<int>((sizeof(argv) / sizeof(argv[0]))) - 1;

    std::cout << cc::cyan;
    status = GenimageWrapper(argc, argv);
    status != 0 ? std::cout << cc::red : std::cout << cc::reset;
    std::cout << cc::reset;
}

[[maybe_unused]] void GenIMG::print()
{
    std::cout << "\tconfig_path: " << this->config_path << std::endl;
    std::cout << "\timage_path: " << this->image_path << std::endl;
    std::cout << "\toutput_path: " << this->output_path << std::endl;
    std::cout << "\ttemp_dir[0]: " << this->temp_dir[0] << std::endl;
    std::cout << "\ttemp_dir[1]: " << this->temp_dir[0] << std::endl;
}

void GenIMG::generate_blank_fex()
{
    std::ofstream out(this->image_path + "/blank.fex");
    // File not open, throw error.
    if (!out.is_open()) {
        throw file_open_error(this->image_path + "/blank.fex");
    }
    out << "blank.fex";
    out.close();
}

[[maybe_unused]] int GenIMG::get_status() const
{
    return this->status;
}

void GenIMG::re_run_genimage(std::string _config_path, std::string _image_path, std::string _output_path)
{
    config_path = std::move(_config_path);
    image_path = std::move(_image_path);
    output_path = std::move(_output_path);
    run_genimage();
}
