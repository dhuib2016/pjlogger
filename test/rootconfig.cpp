#include "Logger.h"
#include <iostream>

using namespace pj;

int main() {
    int ret = Logger::doConfigure(
        "mainlog",
        "log4cplus_1.properties",
        "./logs",
        ConfigType::ROOT
    );

    if (ret != LOG_SUCCESS) {
        std::cout << "doConfigure failed: " << ret << std::endl;
        return -1;
    }

    Logger log("main");

    log.info() << "ROOT config test" << std::endl;
    log.debug("debug value=%d", 100);

    return 0;
}
