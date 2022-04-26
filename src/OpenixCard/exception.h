//
// Created by gloom on 2022/4/26.
//

#ifndef OPENIXCARD_EXCEPTION_H
#define OPENIXCARD_EXCEPTION_H

#include <stdexcept>
#include <string>

class file_open_error : public std::runtime_error {
public:
    explicit file_open_error(const std::string &what) : std::runtime_error("Fail to open file: " + what + ".") {};
};

#endif //OPENIXCARD_EXCEPTION_H
