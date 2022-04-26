//
// Created by gloom on 2022/3/16.
//

#ifndef OPENIXCARD_AW_IMG_PARA_H
#define OPENIXCARD_AW_IMG_PARA_H

#include <vector>
#include <iostream>

class AW_IMG_PARA {
public:
    std::string image_name = {};
    std::string partition_table_fex = "sys_partition.fex";
    std::string partition_table_fex_path = "sys_partition.fex";
    std::string partition_table_cfg = "sys_partition.cfg";
};

#endif //OPENIXCARD_AW_IMG_PARA_H
