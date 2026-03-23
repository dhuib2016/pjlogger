#include "Logger.h"
#include <chrono>
#include <thread>
#include <vector>
#include <iostream>


using namespace pj;

void bench(int n) {
    Logger log("bench");
    for (int i = 0; i < n; ++i) {
        log.info() << "log " << i << std::endl;
    }
}

int main() {
    Logger::configure("log4cplus.properties");

    int threads = 4;
    int per = 100000;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> v;
    for (int i = 0; i < threads; ++i)
        v.emplace_back(bench, per);

    for (auto& t : v) t.join();

    auto end = std::chrono::high_resolution_clock::now();

    double sec = std::chrono::duration<double>(end - start).count();

    std::cout << "TPS: " << (threads * per / sec) << std::endl;
}
