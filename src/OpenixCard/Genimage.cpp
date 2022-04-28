//
// Created by gloom on 2022/4/27.
//

#include "Genimage.h"

#include "exception.h"

#include <utility>
#include <filesystem>
#include <fstream>

[[maybe_unused]] Genimage::Genimage(std::string config_path, std::string image_path, std::string output_path)
        : config_path(std::move(config_path)), image_path(std::move(image_path)), output_path(std::move(output_path)) {
    this->genimage_bin = std::filesystem::current_path() / "bin/genimage";
    // generate blank.fex file for commented partition
    generate_blank_fex();
    // call genimage
    run_genimage();
}

void Genimage::run_genimage() {
    std::string cmd = genimage_bin
                      + " --config "
                      + this->config_path
                      + " --rootpath `mktemp -d` --tmppath `mktemp -d` --inputpath "
                      + this->image_path
                      + " --outputpath "
                      + this->output_path;
    system(cmd.c_str());
}

void Genimage::print() {
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