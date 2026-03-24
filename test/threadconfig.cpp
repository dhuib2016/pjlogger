#include "Logger.h"
#include <thread>

using namespace pj;

void netThread() {
    Logger::doConfigure("net", "", "", ConfigType::BRANCH);
    Logger log("net");

    log.info() << "net thread running" << std::endl;
}

int main() {
    // 1. 主配置
    Logger::doConfigure("app", "log4cplus_1.properties", "./logs", ConfigType::ROOT);

    // 2. 主日志
    Logger mainlog("main");
    mainlog.info() << "system start" << std::endl;

    // 3. 子线程
    std::thread t(netThread);
    t.join();

    return 0;
}
