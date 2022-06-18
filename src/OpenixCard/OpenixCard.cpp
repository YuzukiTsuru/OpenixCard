/*
 * OpenixCard.cpp
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#include <ColorCout.hpp>
#include <argparse/argparse.hpp>
#include <filesystem>

#include "LOG.h"
#include "exception.h"
#include "config.h"
#include "FEX2CFG.h"
#include "GenIMG.h"

extern "C" {
#include "OpenixIMG.h"
}

#include "OpenixCard.h"

OpenixCard::OpenixCard(int argc, char **argv) {
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
    parser.add_argument("-p", "--pack")
            .help("Pack dumped Allwinner image to regular image from folder")
            .default_value(false)
            .implicit_value(true);
    parser.add_argument("-s", "--size")
            .help("Dump the real size of Allwinner image")
            .default_value(false)
            .implicit_value(true);
    parser.add_argument("-i", "--input")
            .help("Input Allwinner image file or dumped image directory")
            .required();

    try {
        // parser args
        parser.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err) {
        std::cout << cc::red << "INPUT ERROR: " << err.what() << cc::reset << std::endl; // args invalid
        std::cout << parser; // show help
        std::exit(1);
    }

    input_file = parser.get<std::string>("input");

    // if input file path is absolute path, convert to relative path, #1
    std::filesystem::path input_path(input_file);

    is_absolute = input_path.is_absolute();
    temp_file_path = input_file + ".dump";
    output_file_path = temp_file_path + ".out";

    // Basic Operator
    mode = [&]() {
        if (parser.get<bool>("pack")) {
            return OpenixCardOperator::PACK;
        } else if (parser.get<bool>("unpack")) {
            return OpenixCardOperator::UNPACK;
        } else if (parser.get<bool>("dump")) {
            return OpenixCardOperator::DUMP;
        } else if (parser.get<bool>("size")) {
            return OpenixCardOperator::SIZE;
        } else {
            return OpenixCardOperator::NONE;
        }
    }();

    if (mode == OpenixCardOperator::NONE) {
        std::cout << parser;
        throw operator_missing_error();
    }

    // Addition Operator
    mode_ext = [&]() {
        if (parser.get<bool>("cfg")) {
            return OpenixCardOperator::UNPACKCFG;
        } else {
            return OpenixCardOperator::NONE;
        }
    }();

    if (mode == OpenixCardOperator::DUMP) {
        LOG::INFO("Input file: " + input_file + " Now converting...");
        check_file(input_file);
        unpack_target_image();
        LOG::INFO("Convert Done! Prasing the partition tables...");
        dump_and_clean();
    } else if (mode == OpenixCardOperator::UNPACK) {
        LOG::INFO("Input file: " + input_file + " Now converting...");
        check_file(input_file);
        unpack_target_image();
        if (mode_ext == OpenixCardOperator::UNPACKCFG) {
            LOG::INFO("Unpack Done! Your image file and cfg file at " + temp_file_path);
            save_cfg_file();
        } else {
            LOG::INFO("Unpack Done! Your image file at " + temp_file_path);
        }
    } else if (mode == OpenixCardOperator::PACK) {
        pack();
    }
}

void OpenixCard::show_logo() {
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

void OpenixCard::pack() {
    LOG::INFO("Generating target image...");

    std::string target_cfg_path = {};

    auto a = std::filesystem::directory_iterator(input_file);

    for (const auto &entry: std::filesystem::directory_iterator(input_file)) {
        if (entry.path().extension() == ".cfg") {
            if (entry.path().filename() != "image.cfg") {
                target_cfg_path = entry.path().string();
            }
        }
    }

    if (target_cfg_path.empty()) {
        LOG::ERROR("Can't find target image partition table cfg file in target folder");
        return;
    }

    GenIMG gen_img(target_cfg_path, input_file, input_file);

    // check gen_img-src result
    if (gen_img.get_status() != 0) {
        LOG::ERROR("Generate image failed!");
        std::exit(1);
    }

    LOG::INFO("Generate Done! Your image file at " + input_file + " Cleaning up...");
}

void OpenixCard::unpack_target_image() {
    // dump the packed image
    std::filesystem::create_directories(temp_file_path);
    crypto_init();
    unpack_image(input_file.c_str(), temp_file_path.c_str(), is_absolute);
}

void OpenixCard::dump_and_clean() {
    FEX2CFG fex2Cfg(temp_file_path);
    auto target_cfg_path = fex2Cfg.save_file(temp_file_path);
    auto image_name = fex2Cfg.get_image_name();
    // generate the image
    LOG::INFO("Prase Done! Generating target image...");

    GenIMG genimage(target_cfg_path, temp_file_path, output_file_path);

    // check genimage-src result
    if (genimage.get_status() != 0) {
        LOG::ERROR("Generate image failed!");
        std::exit(1);
    }

    LOG::INFO("Generate Done! Your image file at " + output_file_path + " Cleaning up...");
    std::filesystem::remove_all(temp_file_path);
}

void OpenixCard::save_cfg_file() {
    FEX2CFG fex2Cfg(temp_file_path);
    auto target_cfg_path = fex2Cfg.save_file(temp_file_path);
    LOG::INFO("Prase Done! Your cfg file at " + target_cfg_path);
}

void OpenixCard::check_file(const std::string &file_path) {
    if (!std::filesystem::exists(file_path)) {
        throw file_open_error(file_path);
    }
}

