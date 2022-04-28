//
// Created by gloom on 2022/4/27.
//

#include "Genimage.h"

#include <utility>
#include <filesystem>

[[maybe_unused]] Genimage::Genimage(std::string name, std::string config_path, std::string image_path, std::string output_path)
        : name(std::move(name)), config_path(std::move(config_path)),
          image_path(std::move(image_path)), output_path(std::move(output_path)) {
    this->genimage_bin = std::filesystem::current_path() / "genimage";
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
    std::cout << "name: " << this->name << std::endl;
    std::cout << "config_path: " << this->config_path << std::endl;
    std::cout << "image_path: " << this->image_path << std::endl;
    std::cout << "output_path: " << this->output_path << std::endl;
}