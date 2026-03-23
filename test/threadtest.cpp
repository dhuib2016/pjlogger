#include "Logger.h"
#include <thread>
#include <vector>

using namespace pj;

void worker(int id) {
    Logger log("thread");

    for (int i = 0; i < 10000; ++i) {
        log.info() << "thread=" << id << " i=" << i << std::endl;
    }
}

int main() {
    Logger::configure("log4cplus.properties");

    std::vector<std::thread> v;
    for (int i = 0; i < 4; ++i)
        v.emplace_back(worker, i);

    for (auto& t : v) t.join();
}
