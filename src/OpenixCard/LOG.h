/*
 * LOG.h
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#ifndef OPENIXCARD_LOG_H
#define OPENIXCARD_LOG_H

#include <iostream>

class LOG {
public:
    [[maybe_unused]] static void DATA(const std::string &msg);

    [[maybe_unused]] static void INFO(const std::string &msg);

    [[maybe_unused]] static void DEBUG(const std::string &msg);

    [[maybe_unused]] static void WARNING(const std::string &msg);

    [[maybe_unused]] static void ERROR(const std::string &msg);
};


#endif //OPENIXCARD_LOG_H
