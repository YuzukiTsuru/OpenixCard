/*
 * exception.h
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#ifndef OPENIXCARD_EXCEPTION_H
#define OPENIXCARD_EXCEPTION_H

#include <stdexcept>
#include <string>

class file_open_error : public std::runtime_error {
public:
    explicit file_open_error(const std::string &what) : std::runtime_error("Fail to open file: " + what + ".") {};
};

class operator_error : public std::runtime_error {
public:
    explicit operator_error(const std::string &what) : std::runtime_error("Operate ERROR: " + what + ".") {};
};

class operator_missing_error : public std::runtime_error {
public:
    explicit operator_missing_error() : std::runtime_error("Operate ERROR, You must specify a Operator.") {};
};

#endif //OPENIXCARD_EXCEPTION_H
