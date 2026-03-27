#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <exception.h>
#include <sstream>
#include <string>
#include <filesystem>
#include <atomic>
#include <thread>


// 0��DEBUG
// 1��INFO
// 2��WARN
// 3��ERROR
// 4��FATAL


namespace pj {

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

/// Public log level enumeration.
enum class LogLevel {
    DEBUG  = 0,
    INFO   = 1,
    WARN   = 2,
    ERROR  = 3,
    FATAL  = 4,
    NOTICE = 5,
    ALERT  = 6
};

class Logger {
public:
    typedef Logger& (*Manipulator)(Logger&);

    // Constructor and Destructor
    explicit Logger(const std::string& category);
    ~Logger();

    // Log level functions
    Logger& debug();     // ��ʽ�ӿ�
    void debug(const char* _format, ...) throw();   // printf���
    void debug(const std::string& message) throw(); // string�ӿ�

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

    // Log level control
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;

    // Level checking — allows callers to skip expensive string construction
    bool isEnabledFor(LogLevel level) const;
    bool isDebugEnabled() const;
    bool isInfoEnabled() const;
    bool isWarnEnabled() const;
    bool isErrorEnabled() const;
    bool isFatalEnabled() const;

    // Manipulator operator
    Logger& operator<<(Manipulator pf);

    template<typename T>
    Logger& operator<<(const T& val) {
        t_buffer << val;
        return *this;
    }

    // ֧�� std::endl
    Logger& operator<<(std::ostream& (*pf)(std::ostream&));

    static int doConfigure(const std::string& logName,
                           const std::string& configFile,
                           const std::string& logDir,
                           ConfigType type = ConfigType::ROOT);
    static void configure(const std::string& configFile);
    static void shutdown();

    /// Add a console appender with the given pattern layout.
    /// @param pattern  PatternLayout conversion pattern.
    /// @param logToStdErr  If true, output to stderr instead of stdout.
    void addConsoleAppender(const std::string& pattern = "%D{%Y-%m-%d %H:%M:%S} %-5p %c - %m%n",
                            bool logToStdErr = false);

    /// Add a file appender.
    /// @param filename     Path to the log file.
    /// @param pattern      PatternLayout conversion pattern.
    /// @param append       If true, append to existing file; if false, truncate.
    /// @param immediateFlush  If true, flush after each log event.
    void addFileAppender(const std::string& filename,
                         const std::string& pattern = "%D{%Y-%m-%d %H:%M:%S} %-5p %c - %m%n",
                         bool append = true,
                         bool immediateFlush = true);

    /// Add a rolling file appender.
    /// @param filename       Path to the log file.
    /// @param maxFileSize    Maximum file size in bytes before rollover (default 10MB).
    /// @param maxBackupIndex Maximum number of backup files (default 5).
    /// @param pattern        PatternLayout conversion pattern.
    /// @param immediateFlush If true, flush after each log event.
    void addRollingFileAppender(const std::string& filename,
                                long maxFileSize = 10 * 1024 * 1024,
                                int maxBackupIndex = 5,
                                const std::string& pattern = "%D{%Y-%m-%d %H:%M:%S} %-5p %c - %m%n",
                                bool immediateFlush = true);

    /// Remove a named appender from this logger.
    void removeAppender(const std::string& name);

    /// Get the number of appenders attached to this logger.
    size_t getAppenderCount();

    /// Remove all appenders from this logger.
    void removeAllAppenders();

    /// Add a syslog appender.
    /// @param ident   Syslog identity string.
    /// @param facility  Syslog facility string (e.g. "user", "local0").
    /// @param pattern PatternLayout pattern.
    void addSyslogAppender(const std::string& ident,
                           const std::string& facility = "",
                           const std::string& pattern = "%D{%Y-%m-%d %H:%M:%S} %-5p %c - %m%n");

    /// Add a socket appender for remote logging.
    /// @param host Remote host name or IP.
    /// @param port Remote port number.
    void addSocketAppender(const std::string& host, unsigned short port);

    /// Add an async console appender for non-blocking logging.
    /// @param logToStdErr Output to stderr.
    /// @param pattern Conversion pattern.
    void addAsyncConsoleAppender(bool logToStdErr = false,
                                 const std::string& pattern = "%D{%Y-%m-%d %H:%M:%S} %-5p %c - %m%n");

    /// Returns the category name of this logger.
    std::string getName() const;

    /// Factory: returns a Logger for the given category name.
    static Logger getLogger(const std::string& name);

    /// Factory: returns a Logger wrapping the root logger.
    static Logger getRootLogger();

private:
    Logger();
    explicit Logger(log4cplus::Logger underlying);
    void logInternal(int level, const std::string& msg);
    void logInternal(int level, const char* fmt, va_list args);

    // �̸߳���
    static thread_local std::ostringstream t_buffer;
    static thread_local int t_level;

    log4cplus::Logger _logger;
};

/// Stream manipulator that flushes the logger buffer.
Logger& endl(Logger& log);

/// Push a context string onto the NDC stack for the current thread.
void ndcPush(const std::string& context);

/// Pop the top context from the NDC stack. Returns the popped string.
std::string ndcPop();

/// Get the current full NDC string for the current thread.
std::string ndcGet();

/// Clear the NDC stack for the current thread.
void ndcClear();

/// Remove NDC for current thread (call before thread exit).
void ndcRemove();

/// RAII guard: pushes on construction, pops on destruction.
class NDCContext {
public:
    explicit NDCContext(const std::string& context);
    ~NDCContext();
    NDCContext(const NDCContext&) = delete;
    NDCContext& operator=(const NDCContext&) = delete;
};

/// Convert LogLevel to human-readable string.
const char* logLevelToString(LogLevel level);

/// Convert string to LogLevel. Returns LogLevel::DEBUG if unrecognized.
LogLevel logLevelFromString(const std::string& str);

}  // namespace pj
