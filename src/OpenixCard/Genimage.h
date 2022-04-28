//
// Created by gloom on 2022/4/27.
//

#ifndef OPENIXCARD_GENIMAGE_H
#define OPENIXCARD_GENIMAGE_H

#include <iostream>


class Genimage {
public:
    [[maybe_unused]] Genimage(std::string config_path, std::string image_path, std::string output_path);

    void print();

private:
    std::string config_path;
    std::string name;
    std::string image_path;
    std::string output_path;
    std::string genimage_bin = "bin/genimage";

    // subprocess runner for genimage
    void run_genimage();

    void generate_blank_fex();
};


#endif //OPENIXCARD_GENIMAGE_H
