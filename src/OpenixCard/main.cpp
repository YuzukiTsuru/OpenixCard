//
// Created by gloom on 2022/2/13.
//

#include <iostream>

#include "FEX2CFG.h"

int main(int argc, char *argv[]) {
    FEX2CFG fex2Cfg("../test/tina_d1-lichee_rv_uart0.img.dump");
    fex2Cfg.save_file("my.cfg");
    return 0;
}