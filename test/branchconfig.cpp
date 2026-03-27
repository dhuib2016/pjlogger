#include "Logger.h"
#include <iostream>

using namespace pj;

int main() {
    // 1. ROOT config
    int ret = Logger::doConfigure(
        "mainlog",
        "log4cplus_1.properties",
        "./logs",
        ConfigType::ROOT
    );

    if (ret != LOG_SUCCESS) {
        std::cout << "ROOT doConfigure failed: " << ret << std::endl;
        return -1;
    }

    Logger rootLog("main");
    rootLog.info() << "ROOT logger initialized" << std::endl;

    // 2. BRANCH config
    ret = Logger::doConfigure(
        "branch",
        "log4cplus_2.properties",
        "./logs",
        ConfigType::BRANCH
    );

    if (ret != LOG_SUCCESS) {
        std::cout << "BRANCH doConfigure failed: " << ret << std::endl;
        return -1;
    }

    Logger branchLog("branch");
    branchLog.info() << "BRANCH logger initialized" << std::endl;
    branchLog.debug("branch debug value=%d", 200);

    // 3. Log from root again to verify it still works
    rootLog.info() << "ROOT logger still working after BRANCH config" << std::endl;

    return 0;
}
