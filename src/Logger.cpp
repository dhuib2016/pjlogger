// 
// Copyright (c) 2025， DongHui
// All rights reserved.
// version： 1.1
// date： 2025-01-04
// 


#include "Logger.h"
#include <log4cplus/initializer.h>
#include <log4cplus/loggingmacros.h>
//#include <log4cplus/helpers/exception.h>
#include <log4cplus/tstring.h>
#include <cstdio>
#include <cstdarg>
#include <memory>
#include <sstream>
#include <iostream>

namespace pj {

static std::atomic<bool> g_rootConfigured{false};

// 线程局部变量（关键优化）
thread_local std::ostringstream Logger::t_buffer;
thread_local int Logger::t_level = 0;

Logger::Logger(const std::string& category)
    : _logger(log4cplus::Logger::getInstance(log4cplus::tstring(category.begin(),category.end()))),
      _buffer(nullptr),
      _priority(0) {
    std::cout << "category:" << category << std::endl;
}

Logger::~Logger() {
    if (_buffer) {
        delete _buffer;
    }
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
    logInternal(5,_format, args);
    va_end(args);
}

void Logger::alert(const std::string& message) throw() {
    logInternal(5,message);
}


void Logger::logInternal(int level, const std::string& msg) {
    //std::cout << "here 2 logInternal:" << message << ",_priority:" << _priority << std::endl;
    switch(level) {
        case 0: 
            std::cout << "message debug:" << msg << std::endl;
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
        default: 
            LOG4CPLUS_INFO(_logger, msg); 
            break;
    }
}

void Logger::logInternal(int level, const char* fmt, va_list args) {
    char buf[4096]; // 扩大buffer
    vsnprintf(buf, sizeof(buf), fmt, args);
    logInternal(level, std::string(buf));
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
    } catch (const log4cplus::helpers::Exception& ex) {
        // Handle exception (e.g., log the error)
        /*
        log4cpp::BasicConfigurator::configure();
        log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
        layout->setConversionPattern("%d %c %p| %m%n");
        log4cpp::Category::getRoot().getAppender()->setLayout(layout);
        log4cpp::Category::getRoot().setPriority(log4cpp::Priority::DEBUG);
        Logger log("Logger");
        log.debug() << "config: " << f.what() << std::endl;
        */
    }
}

int Logger::doConfigure(const std::string& logName,
                        const std::string& configFile,
                        const std::string& logDir,
                        ConfigType type)
{
    try {
        // 1?? 检查配置文件
        if (!std::filesystem::exists(configFile)) {
            return LOG_CONFIG_UNEXIST;
        }

        // 2?? 创建日志目录
        if (!std::filesystem::exists(logDir)) {
            std::filesystem::create_directories(logDir);
        }

        // 3?? 设置环境变量
#ifdef _WIN32
        _putenv_s("LOG_NAME", logName.c_str());
        _putenv_s("LOG_DIR", logDir.c_str());
#else
        setenv("LOG_NAME", logName.c_str(), 1);
        setenv("LOG_DIR", logDir.c_str(), 1);
#endif

        // ==========================
        // 根据 type 做不同处理
        // ==========================
        switch (type)
        {
        case ConfigType::ROOT:
        {
            bool expected = false;
            if (!g_rootConfigured.compare_exchange_strong(expected, true)) {
                return LOG_ALREADY_CONFIGURED;
            }

            log4cplus::PropertyConfigurator::doConfigure(configFile);
            break;
        }

        case ConfigType::THREAD:
        {
            // 每线程独立 logger（建议只设置变量）
            std::string threadName =
                std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));

#ifdef _WIN32
            _putenv_s("THREAD_ID", threadName.c_str());
#else
            setenv("THREAD_ID", threadName.c_str(), 1);
#endif
            break;
        }

        case ConfigType::BRANCH:
        {
            // 分支日志（模块/子系统）
#ifdef _WIN32
            _putenv_s("BRANCH_NAME", logName.c_str());
#else
            setenv("BRANCH_NAME", logName.c_str(), 1);
#endif
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

void Logger::shutdown() {
    log4cplus::Logger::shutdown();
}

}  // namespace pj
