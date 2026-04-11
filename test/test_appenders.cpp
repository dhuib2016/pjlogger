/// Test: Programmatic appender management (console, file, rolling file, async).
#include "Logger.h"
#include <log4cplus/initializer.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <thread>
#include <chrono>
#include <filesystem>

#ifndef PJLOG_TEST_OUTPUT_DIR
#define PJLOG_TEST_OUTPUT_DIR "."
#endif

/// Log path under the test directory (absolute when built via Makefile).
static std::string testLogPath(const char* basename) {
    namespace fs = std::filesystem;
    return (fs::path(PJLOG_TEST_OUTPUT_DIR) / basename).string();
}

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

void test_file_appender() {
    std::cout << "[FileAppender]" << std::endl;

    const std::string logFile = testLogPath("test_file_appender.log");
    std::remove(logFile.c_str());

    Logger log("test.file");
    log.removeAllAppenders();
    log.setLogLevel(LogLevel::DEBUG);

    check("initial appender count is 0", log.getAppenderCount() == 0);

    log.addFileAppender(logFile, "%m%n", false, true);
    check("appender count after add is 1", log.getAppenderCount() == 1);

    log.info("file_appender_test_message");
    log.debug("file_appender_debug_msg");

    std::string content = readFile(logFile);
    check("info message written to file",
          content.find("file_appender_test_message") != std::string::npos);
    check("debug message written to file",
          content.find("file_appender_debug_msg") != std::string::npos);

    // Must detach the first file appender; otherwise the logger keeps two file appenders and every
    // later log line (e.g. seed_before_truncate) is written to BOTH log files.
    log.removeAllAppenders();

    // log.removeAllAppenders() 只做一件事：从这个 Logger 对象上摘掉所有 Appender（不再往那些目的地继续发新日志）。它不会去删磁盘上的日志文件，也不会清空文件里已经写进去的内容。
    // 已经通过 FileAppender 写到 test_file_appender.log 里的行，是写在文件里的数据；卸掉 appender 只是关闭了「以后还往这个文件打」这条通路，不会把文件截断或删掉。
    // 只有在再次用 addFileAppender(..., append=false)（打开模式是 truncate）打开同一个文件时，才会在打开那一刻把文件清空——那是重新打开文件时的行为，不是 removeAllAppenders() 做的。

    // Truncate test uses a separate file so test_file_appender.log keeps the two lines above for inspection.
    const std::string truncLog = testLogPath("test_file_appender_truncate.log");
    std::remove(truncLog.c_str());

    log.addFileAppender(truncLog, "%m%n", true, true);
    log.info("seed_before_truncate");
    log.removeAllAppenders();
    log.addFileAppender(truncLog, "%m%n", false, true);
    log.info("after_truncate");

    content = readFile(truncLog);
    check("truncate mode clears previous content",
          content.find("seed_before_truncate") == std::string::npos);
    check("new message present after truncate",
          content.find("after_truncate") != std::string::npos);
}

void test_rolling_file_appender() {
    std::cout << "\n[RollingFileAppender]" << std::endl;

    const std::string logFile = testLogPath("test_rolling.log");
    std::remove(logFile.c_str());
    // Clean up any backup files from previous runs
    for (int i = 1; i <= 3; ++i) {
        std::remove((logFile + "." + std::to_string(i)).c_str());
    }

    Logger log("test.rolling");
    log.removeAllAppenders();
    log.setLogLevel(LogLevel::DEBUG);

    // log4cplus enforces min 200KB; use that minimum to trigger rollover
    log.addRollingFileAppender(logFile, 200 * 1024, 2, "%m%n", true);
    check("rolling appender attached", log.getAppenderCount() == 1);

    // Write enough to exceed 200KB and trigger rollover
    // Each line ~80 bytes, need ~2600+ lines to exceed 200KB
    for (int i = 0; i < 4000; ++i) {
        log.info("rolling_test_line_%06d_padding_to_fill_the_log_file_past_limit", i);
    }

    // Check that backup file was created
    std::ifstream backup(logFile + ".1");
    check("backup file .1 created after rollover", backup.good());
    backup.close();

    // Main file should still exist and be writable
    std::string content = readFile(logFile);
    check("main log file still has content", !content.empty());

    log.removeAllAppenders();
    /*std::remove(logFile.c_str());
    for (int i = 1; i <= 3; ++i) {
        std::remove((logFile + "." + std::to_string(i)).c_str());
    }
    */
}

void test_console_appender() {
    std::cout << "\n[ConsoleAppender]" << std::endl;

    Logger log("test.console");
    log.removeAllAppenders();
    log.setLogLevel(LogLevel::DEBUG);

    // We can't easily capture console output, but we verify it doesn't crash
    // and the appender count is correct.
    log.addConsoleAppender("%m%n", false);
    log.info("console appender attached (stdout)");
    check("console appender attached (stdout)", log.getAppenderCount() == 1);

    log.addConsoleAppender("%m%n", true);
    log.debug("second console appender attached (stderr)");
    check("second console appender attached (stderr)", log.getAppenderCount() == 2);
   
    log.debug("logging through console appender did not crash");
    check("logging through console appender did not crash", true);

    log.removeAllAppenders();
    check("appender count after removeAll is 0", log.getAppenderCount() == 0);
}

void test_async_console_appender() {
    std::cout << "\n[AsyncConsoleAppender]" << std::endl;

    Logger log("test.async");
    log.removeAllAppenders();
    log.setLogLevel(LogLevel::DEBUG);

    log.addAsyncConsoleAppender(false, "%m%n");
    check("async console appender attached", log.getAppenderCount() == 1);

    log.info("async_smoke_test");
    // Give the async appender time to flush
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    check("async logging did not crash", true);

    log.removeAllAppenders();
}

void test_appender_management() {
    std::cout << "\n[Appender management]" << std::endl;

    const std::string file1 = testLogPath("test_mgmt_1.log");
    const std::string file2 = testLogPath("test_mgmt_2.log");
    std::remove(file1.c_str());
    std::remove(file2.c_str());

    Logger log("test.mgmt");
    log.removeAllAppenders();
    log.setLogLevel(LogLevel::DEBUG);

    check("start with 0 appenders", log.getAppenderCount() == 0);

    log.addFileAppender(file1, "%m%n");
    log.addFileAppender(file2, "%m%n");
    check("2 appenders after adding two files", log.getAppenderCount() == 2);

    log.info("dual_output_test");

    std::string c1 = readFile(file1);
    std::string c2 = readFile(file2);
    check("message in first file", c1.find("dual_output_test") != std::string::npos);
    check("message in second file", c2.find("dual_output_test") != std::string::npos);

    log.removeAllAppenders();
    check("0 appenders after removeAll", log.getAppenderCount() == 0);

    // Verify logging with no appenders doesn't crash
    log.info("no_appender_should_not_crash");
    check("logging with no appenders did not crash", true);

    //std::remove(file1.c_str());
    //std::remove(file2.c_str());
}

int main() {
    log4cplus::Initializer log4cplusInit;
    //test_file_appender();
    //test_rolling_file_appender();
    //test_console_appender();
    //test_async_console_appender();
    test_appender_management();

    std::cout << "\n=== Appender Tests: " << g_passed << " passed, "
              << g_failed << " failed ===" << std::endl;
    return g_failed > 0 ? 1 : 0;
}
