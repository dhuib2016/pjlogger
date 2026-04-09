#include "Logger.h"
#include <iostream>

using namespace pj;

int main() {
    int ret = Logger::doConfigure( "rootlog", "log4cplus_1.properties", "./logs", ConfigType::ROOT);

    // 1. ROOT config
    ret = Logger::doConfigure(
        "mainlog",
        "log4cplus_3.properties",
        "./logs",
        ConfigType::BRANCH
    );

    if (ret != LOG_SUCCESS) {
        std::cout << "mainlog doConfigure failed: " << ret << std::endl;
        return -1;
    }

    Logger myLog("mainlog");
    myLog.info() << "main logger initialized" << std::endl;

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

    Logger othLog("ooplog");
    othLog.debug() << "other log for test " << std::endl;

    Logger branchLog("branch");
    branchLog.info() << "BRANCH logger initialized" << std::endl;
    branchLog.debug("branch debug value=%d", 200);

    // 3. Log from root again to verify it still works
    myLog.info() << "main logger still working after BRANCH config" << std::endl;

    return 0;
}
