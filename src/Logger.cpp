// 
// Copyright (c) 2025�� DongHui
// All rights reserved.
// version�� 1.1
// date�� 2025-01-04
// 


#include "Logger.h"
#include <log4cplus/initializer.h>
#include <log4cplus/loggingmacros.h>
//#include <log4cplus/helpers/exception.h>
#include <log4cplus/tstring.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/ndc.h>
#include <log4cplus/syslogappender.h>
#include <log4cplus/socketappender.h>
#include <log4cplus/asyncappender.h>
#include <cstdio>
#include <cstdarg>
#include <memory>
#include <algorithm>
#include <mutex>
#include <sstream>
#include <iostream>

namespace pj {

static log4cplus::LogLevel toLog4cplusLevel(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:  return log4cplus::DEBUG_LOG_LEVEL;
        case LogLevel::INFO:   return log4cplus::INFO_LOG_LEVEL;
        case LogLevel::WARN:   return log4cplus::WARN_LOG_LEVEL;
        case LogLevel::ERROR:  return log4cplus::ERROR_LOG_LEVEL;
        case LogLevel::FATAL:  return log4cplus::FATAL_LOG_LEVEL;
        case LogLevel::NOTICE: return log4cplus::INFO_LOG_LEVEL;
        case LogLevel::ALERT:  return log4cplus::ERROR_LOG_LEVEL;
        default:               return log4cplus::DEBUG_LOG_LEVEL;
    }
}

static LogLevel fromLog4cplusLevel(log4cplus::LogLevel ll) {
    if (ll <= log4cplus::DEBUG_LOG_LEVEL) return LogLevel::DEBUG;
    if (ll <= log4cplus::INFO_LOG_LEVEL)  return LogLevel::INFO;
    if (ll <= log4cplus::WARN_LOG_LEVEL)  return LogLevel::WARN;
    if (ll <= log4cplus::ERROR_LOG_LEVEL) return LogLevel::ERROR;
    return LogLevel::FATAL;
}

static std::atomic<bool> g_rootConfigured{false};
static std::mutex s_configureMutex;

// �ֲ߳̾��������ؼ��Ż���
thread_local std::ostringstream Logger::t_buffer;
thread_local int Logger::t_level = 0;

Logger::Logger(const std::string& category)
    : _logger(log4cplus::Logger::getInstance(log4cplus::tstring(category.begin(),category.end()))) {
}

// Private constructor from log4cplus::Logger
Logger::Logger(log4cplus::Logger underlying)
    : _logger(std::move(underlying)) {}

Logger::~Logger() = default;

std::string Logger::getName() const {
    const auto& name = _logger.getName();
    return std::string(name.begin(), name.end());
}

Logger Logger::getLogger(const std::string& name) {
    return Logger(log4cplus::Logger::getInstance(
        log4cplus::tstring(name.begin(), name.end())));
}

Logger Logger::getRootLogger() {
    return Logger(log4cplus::Logger::getRoot());
}

Logger& Logger::debug() { t_level = 0; return *this; }
Logger& Logger::info()  { t_level = 1; return *this; }
Logger& Logger::warn()  { t_level = 2; return *this; }
Logger& Logger::error() { t_level = 3; return *this; }
Logger& Logger::fatal() { t_level = 4; return *this; }
Logger& Logger::notice(){ t_level = 5; return *this; }
Logger& Logger::alert() { t_level = 6; return *this; }



void Logger::debug(const char* _format, ...) throw() {
    //std::cout << "here 0 debug:" <<  std::endl;
    //_priority = 0;  // Set priority for debug level
    va_list args;
    va_start(args, _format);
    logInternal(0,_format, args);
    va_end(args);
}

void Logger::debug(const std::string& message) throw() {
    //std::cout << "here 1 debug:" << message << std::endl;
    logInternal(0,message);
}


void Logger::info(const char* _format, ...) throw() {
    //_priority = 1;  // Set priority for info level
    va_list args;
    va_start(args, _format);
    logInternal(1,_format, args);
    va_end(args);
}

void Logger::info(const std::string& message) throw() {
    logInternal(1,message);
}


void Logger::warn(const char* _format, ...) throw() {
    //_priority = 2;  // Set priority for info level
    va_list args;
    va_start(args, _format);
    logInternal(2,_format, args);
    va_end(args);
}

void Logger::warn(const std::string& message) throw() {
    logInternal(2,message);
}


void Logger::error(const char* _format, ...) throw() {
    //_priority = 3;  // Set priority for info level
    va_list args;
    va_start(args, _format);
    logInternal(3,_format, args);
    va_end(args);
}

void Logger::error(const std::string& message) throw() {
    logInternal(3,message);
}

void Logger::fatal(const char* _format, ...) throw() {
    //_priority = 4;  // Set priority for info level
    va_list args;
    va_start(args, _format);
    logInternal(4,_format, args);
    va_end(args);
}

void Logger::fatal(const std::string& message) throw() {
    logInternal(4,message);
}

void Logger::notice(const char* _format, ...) throw() {
    //_priority = 7;  // Set priority for info level
    va_list args;
    va_start(args, _format);
    logInternal(5,_format, args);
    va_end(args);
}

void Logger::notice(const std::string& message) throw() {
    logInternal(5,message);
}

void Logger::alert(const char* _format, ...) throw() {
    //_priority = 8;  // Set priority for info level
    va_list args;
    va_start(args, _format);
    logInternal(6,_format, args);
    va_end(args);
}

void Logger::alert(const std::string& message) throw() {
    logInternal(6,message);
}

void Logger::log(const char* _format, ...) throw() {
    va_list args;
    va_start(args, _format);
    logInternal(t_level, _format, args);
    va_end(args);
}

void Logger::log(const std::string& message) throw() {
    logInternal(t_level, message);
}

void Logger::setLogLevel(LogLevel level) {
    _logger.setLogLevel(toLog4cplusLevel(level));
}

LogLevel Logger::getLogLevel() const {
    return fromLog4cplusLevel(_logger.getLogLevel());
}

bool Logger::isEnabledFor(LogLevel level) const {
    return _logger.isEnabledFor(toLog4cplusLevel(level));
}

bool Logger::isDebugEnabled() const { return isEnabledFor(LogLevel::DEBUG); }
bool Logger::isInfoEnabled()  const { return isEnabledFor(LogLevel::INFO); }
bool Logger::isWarnEnabled()  const { return isEnabledFor(LogLevel::WARN); }
bool Logger::isErrorEnabled() const { return isEnabledFor(LogLevel::ERROR); }
bool Logger::isFatalEnabled() const { return isEnabledFor(LogLevel::FATAL); }

void Logger::logInternal(int level, const std::string& msg) {
    //std::cout << "here 2 logInternal:" << message << ",_priority:" << _priority << std::endl;
    switch(level) {
        case 0:
            LOG4CPLUS_DEBUG(_logger, msg);
            break;
        case 1: 
            LOG4CPLUS_INFO(_logger, msg); 
            break;
        case 2: 
            LOG4CPLUS_WARN(_logger, msg); 
            break;
        case 3: 
            LOG4CPLUS_ERROR(_logger, msg); 
            break;
        case 4:
            LOG4CPLUS_FATAL(_logger, msg);
            break;
        case 5:
            LOG4CPLUS_INFO(_logger, msg);
            break;
        case 6:
            LOG4CPLUS_ERROR(_logger, msg);
            break;
        default: 
            LOG4CPLUS_INFO(_logger, msg); 
            break;
    }
}

void Logger::logInternal(int level, const char* fmt, va_list args) {
    // First pass: try stack buffer
    char stackBuf[4096];
    va_list argsCopy;
    va_copy(argsCopy, args);
    int needed = vsnprintf(stackBuf, sizeof(stackBuf), fmt, args);

    if (needed < 0) {
        // Encoding error
        va_end(argsCopy);
        logInternal(level, std::string("[log format error]"));
        return;
    }

    if (static_cast<size_t>(needed) < sizeof(stackBuf)) {
        // Fits in stack buffer — common fast path
        va_end(argsCopy);
        logInternal(level, std::string(stackBuf));
        return;
    }

    // Second pass: heap allocate for large messages
    std::string heapBuf(static_cast<size_t>(needed) + 1, '\0');
    vsnprintf(heapBuf.data(), heapBuf.size(), fmt, argsCopy);
    va_end(argsCopy);
    heapBuf.resize(static_cast<size_t>(needed));
    logInternal(level, heapBuf);
}

Logger& Logger::flush() {
    if (!t_buffer.str().empty()) {
        logInternal(t_level, t_buffer.str());
        t_buffer.str("");
        t_buffer.clear();
    }
    return *this;
}

Logger& Logger::operator<<(Manipulator pf) {
    //std::cout << "here 3 Manipulator" << std::endl;
    return (*pf)(*this);
}

Logger& Logger::operator<<(std::ostream& (*pf)(std::ostream&)) {
    if (pf == static_cast<std::ostream& (*)(std::ostream&)>(std::endl)) {
        flush();
    }
    return *this;
}

Logger& endl(Logger& log) {
    return log.flush();
}


void Logger::configure(const std::string& configFile) {
    try {
        log4cplus::PropertyConfigurator::doConfigure(configFile);
    } catch (const std::exception& ex) {
        std::cerr << "[pj::Logger] configure failed: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "[pj::Logger] configure failed with unknown error" << std::endl;
    }
}

int Logger::doConfigure(const std::string& logName,
                        const std::string& configFile,
                        const std::string& logDir,
                        ConfigType type)
{
    try {
        // 1?? ��������ļ�
        if (!std::filesystem::exists(configFile)) {
            return LOG_CONFIG_UNEXIST;
        }

        // 2?? ������־Ŀ¼
        if (!std::filesystem::exists(logDir)) {
            std::filesystem::create_directories(logDir);
        }

        // 3?? ���û�������
#ifdef _WIN32
        _putenv_s("LOG_NAME", logName.c_str());
        _putenv_s("LOG_DIR", logDir.c_str());
#else
        setenv("LOG_NAME", logName.c_str(), 1);
        setenv("LOG_DIR", logDir.c_str(), 1);
#endif

        // ==========================
        // ���� type ����ͬ����
        // ==========================
        switch (type)
        {
        case ConfigType::ROOT:
        {
            std::lock_guard<std::mutex> lock(s_configureMutex);
            bool expected = false;
            if (!g_rootConfigured.compare_exchange_strong(expected, true)) {
                return LOG_ALREADY_CONFIGURED;
            }

            log4cplus::PropertyConfigurator::doConfigure(configFile);
            break;
        }

        case ConfigType::THREAD:
        {
            std::lock_guard<std::mutex> lock(s_configureMutex);
            std::string threadName =
                std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));

#ifdef _WIN32
            _putenv_s("THREAD_ID", threadName.c_str());
#else
            setenv("THREAD_ID", threadName.c_str(), 1);
#endif
            if (!configFile.empty()) {
                log4cplus::PropertyConfigurator::doConfigure(configFile);
            }
            break;
        }

        case ConfigType::BRANCH:
        {
            std::lock_guard<std::mutex> lock(s_configureMutex);
#ifdef _WIN32
            _putenv_s("BRANCH_NAME", logName.c_str());
#else
            setenv("BRANCH_NAME", logName.c_str(), 1);
#endif
            if (!configFile.empty()) {
                log4cplus::PropertyConfigurator::doConfigure(configFile);
            }
            break;
        }

        default:
            return LOG_OTHER_ERROR;
        }

        return LOG_SUCCESS;
    }
    catch (...) {
        return LOG_OTHER_ERROR;
    }
}

void Logger::addConsoleAppender(const std::string& pattern, bool logToStdErr) {
    log4cplus::SharedAppenderPtr appender(new log4cplus::ConsoleAppender(logToStdErr));
    appender->setLayout(std::unique_ptr<log4cplus::Layout>(
        new log4cplus::PatternLayout(log4cplus::tstring(pattern.begin(), pattern.end()))));
    _logger.addAppender(appender);
}

void Logger::addFileAppender(const std::string& filename, const std::string& pattern,
                              bool append, bool immediateFlush) {
    std::ios_base::openmode mode = append ? std::ios_base::app : std::ios_base::trunc;
    log4cplus::SharedAppenderPtr appender(new log4cplus::FileAppender(
        log4cplus::tstring(filename.begin(), filename.end()), mode, immediateFlush, true));
    appender->setLayout(std::unique_ptr<log4cplus::Layout>(
        new log4cplus::PatternLayout(log4cplus::tstring(pattern.begin(), pattern.end()))));
    _logger.addAppender(appender);
}

void Logger::addRollingFileAppender(const std::string& filename, long maxFileSize,
                                     int maxBackupIndex, const std::string& pattern,
                                     bool immediateFlush) {
    log4cplus::SharedAppenderPtr appender(new log4cplus::RollingFileAppender(
        log4cplus::tstring(filename.begin(), filename.end()),
        maxFileSize, maxBackupIndex, immediateFlush, true));
    appender->setLayout(std::unique_ptr<log4cplus::Layout>(
        new log4cplus::PatternLayout(log4cplus::tstring(pattern.begin(), pattern.end()))));
    _logger.addAppender(appender);
}

void Logger::removeAppender(const std::string& name) {
    _logger.removeAppender(log4cplus::tstring(name.begin(), name.end()));
}

size_t Logger::getAppenderCount() {
    return _logger.getAllAppenders().size();
}

void Logger::removeAllAppenders() {
    _logger.removeAllAppenders();
}

void Logger::addSyslogAppender(const std::string& ident,
                                const std::string& facility,
                                const std::string& pattern) {
    log4cplus::SharedAppenderPtr appender(new log4cplus::SysLogAppender(
        log4cplus::tstring(ident.begin(), ident.end()),
        log4cplus::tstring(),  // host (empty = local syslog via remote protocol to localhost)
        514,                   // port
        log4cplus::tstring(facility.begin(), facility.end())));
    appender->setLayout(std::unique_ptr<log4cplus::Layout>(
        new log4cplus::PatternLayout(log4cplus::tstring(pattern.begin(), pattern.end()))));
    _logger.addAppender(appender);
}

void Logger::addSocketAppender(const std::string& host, unsigned short port) {
    log4cplus::SharedAppenderPtr appender(new log4cplus::SocketAppender(
        log4cplus::tstring(host.begin(), host.end()), port));
    _logger.addAppender(appender);
}

void Logger::addAsyncConsoleAppender(bool logToStdErr, const std::string& pattern) {
    log4cplus::SharedAppenderPtr consoleAppender(new log4cplus::ConsoleAppender(logToStdErr));
    consoleAppender->setLayout(std::unique_ptr<log4cplus::Layout>(
        new log4cplus::PatternLayout(log4cplus::tstring(pattern.begin(), pattern.end()))));
    log4cplus::SharedAppenderPtr asyncAppender(
        new log4cplus::AsyncAppender(consoleAppender, 1000));
    _logger.addAppender(asyncAppender);
}

void Logger::shutdown() {
    log4cplus::Logger::shutdown();
}

const char* logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:  return "DEBUG";
        case LogLevel::INFO:   return "INFO";
        case LogLevel::WARN:   return "WARN";
        case LogLevel::ERROR:  return "ERROR";
        case LogLevel::FATAL:  return "FATAL";
        case LogLevel::NOTICE: return "NOTICE";
        case LogLevel::ALERT:  return "ALERT";
        default:               return "UNKNOWN";
    }
}

LogLevel logLevelFromString(const std::string& str) {
    std::string upper = str;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (upper == "DEBUG")  return LogLevel::DEBUG;
    if (upper == "INFO")   return LogLevel::INFO;
    if (upper == "WARN")   return LogLevel::WARN;
    if (upper == "ERROR")  return LogLevel::ERROR;
    if (upper == "FATAL")  return LogLevel::FATAL;
    if (upper == "NOTICE") return LogLevel::NOTICE;
    if (upper == "ALERT")  return LogLevel::ALERT;
    return LogLevel::DEBUG;  // default
}

void ndcPush(const std::string& context) {
    log4cplus::getNDC().push(log4cplus::tstring(context.begin(), context.end()));
}

std::string ndcPop() {
    auto s = log4cplus::getNDC().pop();
    return std::string(s.begin(), s.end());
}

std::string ndcGet() {
    const auto& s = log4cplus::getNDC().get();
    return std::string(s.begin(), s.end());
}

void ndcClear() {
    log4cplus::getNDC().clear();
}

void ndcRemove() {
    log4cplus::getNDC().remove();
}

NDCContext::NDCContext(const std::string& context) {
    ndcPush(context);
}

NDCContext::~NDCContext() {
    ndcPop();
}

}  // namespace pj
