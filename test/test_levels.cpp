/// Test: Level control, level checking, and log filtering.
#include "Logger.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <filesystem>

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

/// Read entire file into a string.
static std::string readFile(const std::string& path) {
    std::ifstream ifs(path);
    return std::string((std::istreambuf_iterator<char>(ifs)),
                        std::istreambuf_iterator<char>());
}

int main() {
    const std::string logFile = "test_levels.log";
    std::remove(logFile.c_str());

    // --- logLevelToString / logLevelFromString ---
    std::cout << "[logLevelToString / logLevelFromString]" << std::endl;

    check("DEBUG -> \"DEBUG\"",
          std::string(logLevelToString(LogLevel::DEBUG)) == "DEBUG");
    check("INFO -> \"INFO\"",
          std::string(logLevelToString(LogLevel::INFO)) == "INFO");
    check("WARN -> \"WARN\"",
          std::string(logLevelToString(LogLevel::WARN)) == "WARN");
    check("ERROR -> \"ERROR\"",
          std::string(logLevelToString(LogLevel::ERROR)) == "ERROR");
    check("FATAL -> \"FATAL\"",
          std::string(logLevelToString(LogLevel::FATAL)) == "FATAL");
    check("NOTICE -> \"NOTICE\"",
          std::string(logLevelToString(LogLevel::NOTICE)) == "NOTICE");
    check("ALERT -> \"ALERT\"",
          std::string(logLevelToString(LogLevel::ALERT)) == "ALERT");

    check("\"debug\" -> DEBUG (case-insensitive)",
          logLevelFromString("debug") == LogLevel::DEBUG);
    check("\"Info\" -> INFO (mixed case)",
          logLevelFromString("Info") == LogLevel::INFO);
    check("\"WARN\" -> WARN",
          logLevelFromString("WARN") == LogLevel::WARN);
    check("\"unknown\" -> DEBUG (default)",
          logLevelFromString("unknown") == LogLevel::DEBUG);
    check("empty string -> DEBUG (default)",
          logLevelFromString("") == LogLevel::DEBUG);

    // --- setLogLevel / getLogLevel ---
    std::cout << "\n[setLogLevel / getLogLevel]" << std::endl;

    Logger log("test.levels");
    log.addFileAppender(logFile, "%m%n", false, true);

    log.setLogLevel(LogLevel::DEBUG);
    check("setLogLevel(DEBUG), getLogLevel() == DEBUG",
          log.getLogLevel() == LogLevel::DEBUG);

    log.setLogLevel(LogLevel::WARN);
    check("setLogLevel(WARN), getLogLevel() == WARN",
          log.getLogLevel() == LogLevel::WARN);

    log.setLogLevel(LogLevel::FATAL);
    check("setLogLevel(FATAL), getLogLevel() == FATAL",
          log.getLogLevel() == LogLevel::FATAL);

    // --- isEnabledFor / is*Enabled ---
    std::cout << "\n[isEnabledFor / is*Enabled]" << std::endl;

    log.setLogLevel(LogLevel::WARN);
    check("at WARN: isDebugEnabled() == false", !log.isDebugEnabled());
    check("at WARN: isInfoEnabled() == false",  !log.isInfoEnabled());
    check("at WARN: isWarnEnabled() == true",    log.isWarnEnabled());
    check("at WARN: isErrorEnabled() == true",   log.isErrorEnabled());
    check("at WARN: isFatalEnabled() == true",   log.isFatalEnabled());
    check("at WARN: isEnabledFor(DEBUG) == false",
          !log.isEnabledFor(LogLevel::DEBUG));
    check("at WARN: isEnabledFor(WARN) == true",
          log.isEnabledFor(LogLevel::WARN));

    log.setLogLevel(LogLevel::DEBUG);
    check("at DEBUG: all levels enabled",
          log.isDebugEnabled() && log.isInfoEnabled() &&
          log.isWarnEnabled() && log.isErrorEnabled() && log.isFatalEnabled());

    // --- Log filtering: messages below threshold are suppressed ---
    std::cout << "\n[Log filtering]" << std::endl;

    log.removeAllAppenders();
    std::remove(logFile.c_str());
    log.addFileAppender(logFile, "%m%n", false, true);
    log.setLogLevel(LogLevel::ERROR);

    log.debug("should_not_appear_debug");
    log.info("should_not_appear_info");
    log.warn("should_not_appear_warn");
    log.error("should_appear_error");
    log.fatal("should_appear_fatal");

    std::string content = readFile(logFile);
    check("debug message filtered out",
          content.find("should_not_appear_debug") == std::string::npos);
    check("info message filtered out",
          content.find("should_not_appear_info") == std::string::npos);
    check("warn message filtered out",
          content.find("should_not_appear_warn") == std::string::npos);
    check("error message present",
          content.find("should_appear_error") != std::string::npos);
    check("fatal message present",
          content.find("should_appear_fatal") != std::string::npos);

    // --- notice() and alert() regression ---
    std::cout << "\n[notice / alert regression]" << std::endl;

    log.removeAllAppenders();
    std::remove(logFile.c_str());
    log.addFileAppender(logFile, "%m%n", false, true);
    log.setLogLevel(LogLevel::DEBUG);

    log.notice("notice_marker_printf");
    log.notice() << "notice_marker_stream" << std::endl;
    log.alert("alert_marker_printf");
    log.alert() << "alert_marker_stream" << std::endl;

    content = readFile(logFile);
    check("notice printf logged", content.find("notice_marker_printf") != std::string::npos);
    check("notice stream logged", content.find("notice_marker_stream") != std::string::npos);
    check("alert printf logged",  content.find("alert_marker_printf") != std::string::npos);
    check("alert stream logged",  content.find("alert_marker_stream") != std::string::npos);

    // --- log() uses current stream level ---
    std::cout << "\n[log() with current level]" << std::endl;

    log.removeAllAppenders();
    std::remove(logFile.c_str());
    log.addFileAppender(logFile, "%m%n", false, true);
    log.setLogLevel(LogLevel::WARN);

    log.info();  // set t_level to INFO
    log.log("log_at_info_should_not_appear");
    log.error(); // set t_level to ERROR
    log.log("log_at_error_should_appear");

    content = readFile(logFile);
    check("log() at INFO filtered when level=WARN",
          content.find("log_at_info_should_not_appear") == std::string::npos);
    check("log() at ERROR present when level=WARN",
          content.find("log_at_error_should_appear") != std::string::npos);

    // Cleanup
    std::remove(logFile.c_str());

    std::cout << "\n=== Level Tests: " << g_passed << " passed, "
              << g_failed << " failed ===" << std::endl;
    return g_failed > 0 ? 1 : 0;
}
