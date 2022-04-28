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

#include "Genimage.h"
#include "FEX2CFG.h"

extern "C" {
#include "OpenixIMG.h"
}

void LOG(std::string msg) {
    std::cout << cc::cyan << "[OpenixCard] " << msg << cc::reset << std::endl;
}

void show_logo() {
    std::cout << cc::blue <<
              " _____             _     _____           _ \n"
              "|     |___ ___ ___|_|_ _|     |___ ___ _| |\n"
              "|  |  | . | -_|   | |_'_|   --| .'|  _| . |\n"
              "|_____|  _|___|_|_|_|_,_|_____|__,|_| |___|\n"
              "      |_| Version: "
              << PROJECT_GIT_HASH << cc::reset << std::endl;
}

int main(int argc, char *argv[]) {

    argparse::ArgumentParser parser("OpenixCard", []() {
        if (std::string(PROJECT_GIT_HASH).empty())
            return PROJECT_VER;
        else
            return PROJECT_GIT_HASH;
    }());

    show_logo();

    parser.add_argument("-w", "--write")
            .help("Write Allwinner Image to SD Card")
            .default_value(false)
            .implicit_value(true);
    parser.add_argument("-d", "--dump")
            .help("Convert Allwinner image to regular image")
            .default_value(false)
            .implicit_value(true);
    parser.add_argument("-i", "--input")
            .help("Input Allwinner image file")
            .required();
    parser.add_argument("-o", "--output")
            .help("Output file path")
            .required();
    parser.add_argument("-b", "--blocksize")
            .help("Block size")
            .default_value("512"); // 512, 1024, 2048, 4096

    try {
        // parser args
        parser.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err) {
        std::cout << cc::red << "INPUT ERROR: " << err.what() << cc::reset << std::endl; // args invalid
        std::cout << parser; // show help
        std::exit(1);
    }

    auto is_write = parser.get<bool>("write");
    auto is_dump = parser.get<bool>("dump");

    if (!is_write && !is_dump) {
        std::cout << cc::red << "INPUT ERROR: " << cc::reset << "You must specify --write or --dump\n" << std::endl;
        std::cout << parser; // show help
        std::exit(1);
    }

    if (is_dump) {
        LOG("Input file: " + parser.get("input") + " Now converting...");

        auto input_file = parser.get<std::string>("input");
        auto temp_file_path = input_file + ".dump";
        // dump the packed image
        crypto_init();
        unpack_image(input_file.c_str(), temp_file_path.c_str());

        // parse the dump file 'sys_partition.fex' to get the partition config

        LOG("Convert Done! Prasing the partition tables...");

        FEX2CFG fex2Cfg(temp_file_path);
        auto target_cfg_path = fex2Cfg.save_file(temp_file_path);

        // generate the image

        LOG("Prase Done! Generating target image...");

        Genimage genimage(target_cfg_path, temp_file_path, parser.get<std::string>("output"));

        LOG("Generate Done! Cleaning up...");

        std::filesystem::remove_all(temp_file_path);
    } else {
        std::cout << cc::red << "Sorry, Write function not implemented yet!" << cc::reset << std::endl;
    }
    return 0;
}