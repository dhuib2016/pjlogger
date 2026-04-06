/// Test: Logger identity, factory methods, and stream/printf API variants.
#include "Logger.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

using namespace pj;

static int g_passed = 0;
static int g_failed = 0;

static void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS: " << name << std::endl;
        ++g_passed;
    } else {
        std::cout << "  FAIL: " << name << std::endl;
        ++g_failed;
    }
}

static std::string readFile(const std::string& path) {
    std::ifstream ifs(path);
    return std::string((std::istreambuf_iterator<char>(ifs)),
                        std::istreambuf_iterator<char>());
}

void test_getName() {
    std::cout << "[getName]" << std::endl;

    Logger log1("myapp.module1");
    check("getName() returns category", log1.getName() == "myapp.module1");

    Logger log2("x");
    check("single-char category", log2.getName() == "x");
}

void test_getLogger() {
    std::cout << "\n[getLogger factory]" << std::endl;

    Logger log = Logger::getLogger("factory.test");
    check("getLogger returns correct name", log.getName() == "factory.test");

    // Same name should reference the same underlying logger
    Logger log2 = Logger::getLogger("factory.test");
    check("same name gives same logger name", log.getName() == log2.getName());
}

void test_getRootLogger() {
    std::cout << "\n[getRootLogger]" << std::endl;

    Logger root = Logger::getRootLogger();
    check("root logger name is \"root\"", root.getName() == "root");
}

void test_all_log_methods() {
    std::cout << "\n[All log methods: stream, printf, string]" << std::endl;

    const std::string logFile = "test_all_methods.log";
    std::remove(logFile.c_str());

    Logger log("test.methods");
    log.removeAllAppenders();
    log.setLogLevel(LogLevel::DEBUG);
    log.addFileAppender(logFile, "%-5p %m%n", false, true);

    // Stream style
    log.debug() << "stream_debug" << std::endl;
    log.info()  << "stream_info"  << std::endl;
    log.warn()  << "stream_warn"  << std::endl;
    log.error() << "stream_error" << std::endl;
    log.fatal() << "stream_fatal" << std::endl;

    // Printf style
    log.debug("printf_debug_%d", 1);
    log.info("printf_info_%d", 2);
    log.warn("printf_warn_%d", 3);
    log.error("printf_error_%d", 4);
    log.fatal("printf_fatal_%d", 5);

    // String style
    log.debug(std::string("string_debug"));
    log.info(std::string("string_info"));
    log.warn(std::string("string_warn"));
    log.error(std::string("string_error"));
    log.fatal(std::string("string_fatal"));

    std::string content = readFile(logFile);

    // Stream
    check("stream debug", content.find("stream_debug") != std::string::npos);
    check("stream info",  content.find("stream_info")  != std::string::npos);
    check("stream warn",  content.find("stream_warn")  != std::string::npos);
    check("stream error", content.find("stream_error") != std::string::npos);
    check("stream fatal", content.find("stream_fatal") != std::string::npos);

    // Printf
    check("printf debug", content.find("printf_debug_1") != std::string::npos);
    check("printf info",  content.find("printf_info_2")  != std::string::npos);
    check("printf warn",  content.find("printf_warn_3")  != std::string::npos);
    check("printf error", content.find("printf_error_4") != std::string::npos);
    check("printf fatal", content.find("printf_fatal_5") != std::string::npos);

    // String
    check("string debug", content.find("string_debug") != std::string::npos);
    check("string info",  content.find("string_info")  != std::string::npos);
    check("string warn",  content.find("string_warn")  != std::string::npos);
    check("string error", content.find("string_error") != std::string::npos);
    check("string fatal", content.find("string_fatal") != std::string::npos);

    // Verify log level prefixes in output
    check("DEBUG prefix present", content.find("DEBUG stream_debug") != std::string::npos);
    check("INFO  prefix present", content.find("INFO  stream_info")  != std::string::npos);
    check("WARN  prefix present", content.find("WARN  stream_warn")  != std::string::npos);
    check("ERROR prefix present", content.find("ERROR stream_error") != std::string::npos);
    check("FATAL prefix present", content.find("FATAL stream_fatal") != std::string::npos);

    log.removeAllAppenders();
    std::remove(logFile.c_str());
}

void test_large_printf_message() {
    std::cout << "\n[Large printf message (heap fallback)]" << std::endl;

    const std::string logFile = "test_large_msg.log";
    std::remove(logFile.c_str());

    Logger log("test.large");
    log.removeAllAppenders();
    log.setLogLevel(LogLevel::DEBUG);
    log.addFileAppender(logFile, "%m%n", false, true);

    // Build a string > 4096 bytes to trigger the two-pass vsnprintf heap path
    std::string bigValue(5000, 'X');
    log.info("BIG[%s]END", bigValue.c_str());

    std::string content = readFile(logFile);
    check("large message starts with BIG[",
          content.find("BIG[") != std::string::npos);
    check("large message ends with ]END",
          content.find("]END") != std::string::npos);
    check("large message contains full payload",
          content.find(bigValue) != std::string::npos);

    log.removeAllAppenders();
    std::remove(logFile.c_str());
}

int main() {
    test_getName();
    test_getLogger();
    test_getRootLogger();
    test_all_log_methods();
    test_large_printf_message();

    std::cout << "\n=== Logger API Tests: " << g_passed << " passed, "
              << g_failed << " failed ===" << std::endl;
    return g_failed > 0 ? 1 : 0;
}
