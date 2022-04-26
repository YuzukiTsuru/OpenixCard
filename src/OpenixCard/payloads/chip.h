//
// Created by gloom on 2022/4/26.
//

#ifndef OPENIXCARD_CHIP_H
#define OPENIXCARD_CHIP_H

#include <iostream>
#include <sstream>
#include <inicpp/inicpp.h>

typedef struct partition_table_struct {
    std::string name;
    uint64_t size;
    std::string downloadfile;
    uint64_t user_type;
} partition_table_struct;

template<typename T>
std::string tostring(const T &t) {
    std::ostringstream ss;
    ss << t;
    return ss.str();
}

std::string gen_linux_cfg_from_fex_map(const inicpp::config& fex);

#endif //OPENIXCARD_CHIP_H
