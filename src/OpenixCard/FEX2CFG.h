//
// Created by gloom on 2022/3/16.
//

#ifndef OPENIXCARD_FEX2CFG_H
#define OPENIXCARD_FEX2CFG_H

#include <iostream>

#include <inicpp/inicpp.h>

#include "AW_IMG_PARA.h"

class FEX2CFG {
public:
    explicit FEX2CFG(const std::string &dump_path);

    void save_file(const std::string &file_path);

    std::string get_cfg();

    void print_partition_table();

private:
    AW_IMG_PARA awImgPara;
    inicpp::config fex_classed;
    std::string awImgFex = {};
    std::string awImgCfg = {};
    std::string awImgFexClassed = {};

    void open_file(const std::string &file_path);

    void classify_fex();

    void parse_fex();

    void gen_cfg();
};


#endif //OPENIXCARD_FEX2CFG_H
