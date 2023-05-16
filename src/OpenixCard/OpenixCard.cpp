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
            .help("Get Allwinner image partition table cfg file (use together with unpack)")
            .default_value(false)
            .implicit_value(true);
    parser.add_argument("-p", "--pack")
            .help("pack dumped Allwinner image to regular image from folder (needs cfg file)")
            .default_value(false)
            .implicit_value(true);
    parser.add_argument("-s", "--size")
            .help("Get the accurate size of Allwinner image")
            .default_value(false)
            .implicit_value(true);
    parser.add_argument("input")
            .help("Input image file or directory path")
            .required()
            .remaining();
    parser.add_epilog(
            "\r\neg.:\r\nOpenixCard -u  <img>   - Unpack Allwinner image to target"
            "\r\nOpenixCard -uc <img>   - Unpack Allwinner image to target and generate Allwinner image partition table cfg"
            "\r\nOpenixCard -d  <img>   - Convert Allwinner image to regular image"
            "\r\nOpenixCard -p  <dir>   - pack dumped Allwinner image to regular image from folder"
            "\r\nOpenixCard -s  <img>   - Get the accurate size of Allwinner image)"
            "\r\n");

    if (argc < 2) {
        std::cout << parser; // show help
        return;
    }

    try {
        // parser args
        parser.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err) {
        std::cout << parser; // show help
        throw operator_error(err.what());
    }

    try {
        input_file_vector = parser.get<std::vector<std::string>>("input");
    } catch (const std::logic_error &err) {
        std::cout << parser; // show help
        throw no_file_provide_error();
    }

    input_file = input_file_vector[0];

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
            if (parser.get<bool>("cfg")) {
                return OpenixCardOperator::UNPACKCFG;
            }
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
        // Break here.
        throw operator_missing_error();
    }


    if (mode == OpenixCardOperator::DUMP) {
        unpack_target_image();
        LOG::INFO("Convert Done! Parsing the partition tables...");
        dump_and_clean();
    } else if (mode == OpenixCardOperator::UNPACK || mode == OpenixCardOperator::UNPACKCFG) {
        unpack_target_image();
        LOG::INFO("Unpack Done! Your image file is at " + temp_file_path);
        if (mode == OpenixCardOperator::UNPACKCFG) {
            save_cfg_file();
        }
    } else if (mode == OpenixCardOperator::PACK) {
        pack();
    }

    if (parser.get<bool>("size")) {
        get_real_size();
    }
}

void OpenixCard::show_logo() {
    std::cout << cc::green <<
              " _____             _     _____           _ \n"
              "|     |___ ___ ___|_|_ _|     |___ ___ _| |\n"
              "|  |  | . | -_|   | |_'_|   --| .'|  _| . |\n"
              "|_____|  _|___|_|_|_|_,_|_____|__,|_| |___|\n"
              "      |_| Version: " << PROJECT_GIT_HASH << " Commit: " << PROJECT_VER
              << cc::magenta <<
              "\nCopyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>\n"
              << cc::reset << std::endl;
}

void OpenixCard::pack() {
    LOG::INFO("Generating target image...");

    std::string target_cfg_path = {};

    auto a = std::filesystem::directory_iterator(input_file);
    temp_file_path = input_file; // for potential size query

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
    if (gen_img.get_status() == -EINVAL) {
        LOG::ERROR("Generate image failed! Check your cfg file in: " + target_cfg_path);
        std::exit(1);
    } else if (gen_img.get_status() != 0) {
        LOG::ERROR("Generate image failed!");
        std::exit(1);
    }

    LOG::INFO("Generate Done! Your image file is at " + input_file + " Cleaning up...");
}

void OpenixCard::unpack_target_image() {
    // dump the packed image
    LOG::INFO("Converting input file: " + input_file);
    check_file(input_file);
    std::filesystem::create_directories(temp_file_path);
    crypto_init();
    std::cout << cc::cyan;
    auto unpack_img_ret = unpack_image(input_file.c_str(), temp_file_path.c_str(), is_absolute);
    std::cout << cc::reset;

    switch (unpack_img_ret) {
        case 2:
            throw file_open_error(input_file);
        case 3:
            throw file_size_error(input_file);
        case 4:
            throw std::runtime_error("Unable to allocate memory for image: " + input_file);
        case 5:
            throw file_format_error(input_file);
        default:
            break;
    }
}

void OpenixCard::dump_and_clean() {
    FEX2CFG fex2Cfg(temp_file_path);
    auto target_cfg_path = fex2Cfg.save_file(temp_file_path);
    auto image_name = fex2Cfg.get_image_name();
    // generate the image
    LOG::INFO("Parse Done! Generating target image...");

    GenIMG genimage(target_cfg_path, temp_file_path, output_file_path);

    // check genimage-src result
    if (genimage.get_status() != 0) {
        LOG::ERROR("Generate image failed!");
        std::exit(1);
    }

    LOG::INFO("Generate Done! Your image file is at " + output_file_path);
    LOG::INFO("Cleaning up...");
}

void OpenixCard::save_cfg_file() {
    FEX2CFG fex2Cfg(temp_file_path);
    auto target_cfg_path = fex2Cfg.save_file(temp_file_path);
    LOG::INFO("Parse Done! Your cfg file is at " + target_cfg_path);
}

void OpenixCard::check_file(const std::string &file_path) {
    if (!std::filesystem::exists(file_path)) {
        throw file_open_error(file_path);
    }
}

void OpenixCard::get_real_size() {
    LOG::INFO("Getting accurate size of Allwinner img...");
    FEX2CFG fex2Cfg(temp_file_path);
    auto real_size = fex2Cfg.get_image_real_size(true);
    LOG::DATA("The accurate size of image: " + std::to_string(real_size / 1024) + "MB, " + std::to_string(real_size) + "KB");
    std::filesystem::remove_all(temp_file_path);
}

