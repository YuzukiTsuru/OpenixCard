/*
 * LOG.cpp
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#include <ColorCout.hpp>

#include "LOG.h"

void LOG::DATA(const std::string &msg) {
    std::cout << cc::green << msg << cc::reset << std::endl;
}

void LOG::INFO(const std::string &msg) {
    std::cout << cc::cyan << "[OpenixCard INFO] " << msg << cc::reset << std::endl;
}

[[maybe_unused]] void LOG::DEBUG(const std::string &msg) {
    std::cout << cc::white << "[OpenixCard DEBUG] " << msg << cc::reset << std::endl;
}

void LOG::WARNING(const std::string &msg) {
    std::cout << cc::yellow << "[OpenixCard WARNING] " << msg << cc::reset << std::endl;
}

void LOG::ERROR(const std::string &msg) {
    std::cout << cc::red << "[OpenixCard ERROR] " << msg << cc::reset << std::endl;
}
