#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <exception.h>
#include <sstream>
#include <string>
#include <filesystem>
#include <atomic>
#include <thread>


// 0：DEBUG
// 1：INFO
// 2：WARN
// 3：ERROR
// 4：FATAL


namespace pj {

typedef enum {
    ROOT_CONFIG    = 1             //主进程log初始化调用
   ,THREAD_CONFIG  = 2             //多线程log初始化调用
   ,BRANCH_CONFIG  = 3             //分支log初始化调用
}en_config_type;

enum class ConfigType {
    ROOT = 1,
    THREAD = 2,
    BRANCH = 3
};

enum ConfigResult {
    LOG_SUCCESS = 0,
    LOG_CONFIG_UNEXIST = 1,
    LOG_PATH_UNEXIST = 2,
    LOG_ALREADY_CONFIGURED = 3,
    LOG_OTHER_ERROR = 4
};

class Logger {
public:
    typedef Logger& (*Manipulator)(Logger&);

    // Constructor and Destructor
    explicit Logger(const std::string& category);
    ~Logger();

    // Log level functions
    Logger& debug();     // 流式接口
    void debug(const char* _format, ...) throw();   // printf风格
    void debug(const std::string& message) throw(); // string接口

    Logger& info();
    void info(const char* _format, ...) throw();
    void info(const std::string& message) throw();

    Logger& notice();
    void notice(const char* _format, ...) throw();
    void notice(const std::string& message) throw();

    Logger& warn();
    void warn(const char* _format, ...) throw();
    void warn(const std::string& message) throw();

    Logger& error();
    void error(const char* _format, ...) throw();
    void error(const std::string& message) throw();

    Logger& alert();
    void alert(const char* _format, ...) throw();
    void alert(const std::string& message) throw();

    Logger& fatal();
    void fatal(const char* _format, ...) throw();
    void fatal(const std::string& message) throw();

    void log(const char* _format, ...) throw();
    void log(const std::string& message) throw();

    Logger& flush();

    // Manipulator operator
    Logger& operator<<(Manipulator pf);

    template<typename T>
    Logger& operator<<(const T& val) {
        t_buffer << val;
        return *this;
    }

    // 支持 std::endl
    Logger& operator<<(std::ostream& (*pf)(std::ostream&));

    static int doConfigure(const std::string& logName,
                           const std::string& configFile,
                           const std::string& logDir,
                           ConfigType type = ConfigType::ROOT);
    static void configure(const std::string& configFile);
    static void shutdown();

private:
    Logger();
    void logInternal(int level, const std::string& msg);
    void logInternal(int level, const char* fmt, va_list args);

    // 线程隔离
    static thread_local std::ostringstream t_buffer;
    static thread_local int t_level;

    log4cplus::Logger _logger;
    std::ostringstream* _buffer;
    int _priority;
};

}  // namespace pace
