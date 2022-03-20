//
// Created by gloom on 2022/3/16.
//

#ifndef OPENIXCARD_FEX2CFG_H
#define OPENIXCARD_FEX2CFG_H

#include <iostream>

#include "aw_img_para.h"

class FEX2CFG {
public:
    FEX2CFG(std::string dump_path);

    void savefile(std::string file_path);

private:
    AW_IMG_PARA awImgPara;
};


#endif //OPENIXCARD_FEX2CFG_H
