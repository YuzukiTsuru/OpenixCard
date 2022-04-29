/*
 * main.cpp
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#include <iostream>
#include <ColorCout.hpp>
#include <argparse/argparse.hpp>
#include <filesystem>

#include "exception.h"
#include "config.h"

#include "FEX2CFG.h"
#include "Genimage.h"

extern "C" {
#include "OpenixIMG.h"
}

void LOG(const std::string &msg) {
    std::cout << cc::cyan << "[OpenixCard] " << msg << cc::reset << std::endl;
}

void ERR(const std::string &msg) {
    std::cout << cc::red << "[OpenixCard ERROR] " << msg << cc::reset << std::endl;
}

void show_logo() {
    std::cout << cc::blue <<
              " _____             _     _____           _ \n"
              "|     |___ ___ ___|_|_ _|     |___ ___ _| |\n"
              "|  |  | . | -_|   | |_'_|   --| .'|  _| . |\n"
              "|_____|  _|___|_|_|_|_,_|_____|__,|_| |___|\n"
              "      |_| Version: " << PROJECT_GIT_HASH
              << cc::yellow <<
              "\nCopyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>\n"
              << cc::reset << std::endl;
}

int main(int argc, char *argv[]) {

    argparse::ArgumentParser parser("OpenixCard", []() {
        if (std::string(PROJECT_GIT_HASH).empty())
            return PROJECT_VER;
        else
            return PROJECT_GIT_HASH;
    }());

    show_logo();

    parser.add_argument("-u", "--unpack")
            .help("Unpack Allwinner Image to folder")
            .default_value(false)
            .implicit_value(true);
    parser.add_argument("-d", "--dump")
            .help("Convert Allwinner image to regular image")
            .default_value(false)
            .implicit_value(true);
    parser.add_argument("-c", "--cfg")
            .help("Get Allwinner image partition table cfg file")
            .default_value(false)
            .implicit_value(true);
    parser.add_argument("-i", "--input")
            .help("Input Allwinner image file")
            .required();
    parser.add_argument("-o", "--output")
            .help("Output file path")
            .default_value("output");

    try {
        // parser args
        parser.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err) {
        std::cout << cc::red << "INPUT ERROR: " << err.what() << cc::reset << std::endl; // args invalid
        std::cout << parser; // show help
        std::exit(1);
    }

    auto input_file = parser.get<std::string>("input");

    // if input file path is absolute path, convert to relative path, #1
    std::filesystem::path input_path(input_file);
    if (input_path.is_absolute()) {
        input_file = input_path.relative_path();
    }
    auto temp_file_path = input_file + ".dump";
    auto output_file_path = parser.get<std::string>("output");
    auto is_unpack = parser.get<bool>("unpack");
    auto is_dump = parser.get<bool>("dump");
    auto is_cfg = parser.get<bool>("cfg");

    if (!is_unpack && !is_dump && !is_cfg) {
        ERR("No action selected, please select one of the following actions: -u, -d, -c");
        std::cout << parser; // show help
        std::exit(1);
    }

    LOG("Input file: " + input_file + " Now converting...");
    // dump the packed image
    crypto_init();
    unpack_image(input_file.c_str(), temp_file_path.c_str());

    // parse the dump file 'sys_partition.fex' to get the partition config
    LOG("Convert Done! Prasing the partition tables...");

    if (is_dump) {
        FEX2CFG fex2Cfg(temp_file_path);
        auto target_cfg_path = fex2Cfg.save_file(temp_file_path);
        auto image_name = fex2Cfg.get_image_name();
        // generate the image
        LOG("Prase Done! Generating target image...");

        Genimage genimage(target_cfg_path, temp_file_path, parser.get<std::string>("output"));

        // check genimage result
        if (genimage.get_status() != 0) {
            ERR("Generate image failed!");
            std::exit(1);
        }

        LOG("Generate Done! Your image file at " + parser.get<std::string>("output") + " Cleaning up...");
        std::filesystem::remove_all(temp_file_path);
    } else if (is_unpack) {
        LOG("Unpack Done! Your image file at " + temp_file_path);
    } else {
        FEX2CFG fex2Cfg(temp_file_path);
        auto target_cfg_path = fex2Cfg.save_file(parser.get<std::string>("output"));
        std::filesystem::remove_all(temp_file_path);
        LOG("Prase Done! Your cfg file at " + target_cfg_path);
    }
    return 0;
}