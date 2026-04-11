/// Test: NDC (Nested Diagnostic Context) functions and RAII guard.
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

void test_ndc_basic() {
    Logger log("test_basic.ndc");
    log.removeAllAppenders();
    log.setLogLevel(LogLevel::DEBUG);
    log.addFileAppender("test_basic.ndc", "%D{%Y-%m-%d %H:%M:%S} [%x] %-5p %c - %m%n", false, true);

    std::cout << "[NDC basic push/pop/get]" << std::endl;

    ndcClear();
    log.info("NDC empty initially");
    check("NDC empty initially", ndcGet().empty());

    ndcPush("ctx1");
    log.info("after push(ctx1), get() == \"ctx1\"");
    check("after push(ctx1), get() == \"ctx1\"", ndcGet() == "ctx1");

    ndcPush("ctx2");
    log.info("after push(ctx2), get() contains ctx2");
    check("after push(ctx2), get() contains ctx2",
          ndcGet().find("ctx2") != std::string::npos);

    std::string popped = ndcPop();
    log.info("pop() returns \"ctx2\",popped=%s", popped.c_str());
    check("pop() returns \"ctx2\"", popped == "ctx2");
    check("after pop, get() == \"ctx1\"", ndcGet() == "ctx1");

    ndcPop();
    log.info("after second pop, NDC is empty");
    check("after second pop, NDC is empty", ndcGet().empty());

    ndcRemove();
    log.info("ndcRemove() called"); 
}

void test_ndc_clear() {
    Logger log("test_clear.ndc");
    log.removeAllAppenders();
    log.setLogLevel(LogLevel::DEBUG);
    log.addFileAppender("test_clear.ndc", "%D{%Y-%m-%d %H:%M:%S} [%x] %-5p %c - %m%n", false, true);

    std::cout << "\n[NDC clear]" << std::endl;

    ndcPush("a");
    ndcPush("b");
    ndcPush("c");
    log.info("3 items pushed, get() is non-empty");
    check("3 items pushed, get() is non-empty", !ndcGet().empty());

    ndcClear();
    log.info("after clear(), get() is empty");
    check("after clear(), get() is empty", ndcGet().empty());

    ndcRemove();
    log.info("ndcRemove() called");
}

void test_ndc_raii() {
    std::cout << "\n[NDCContext RAII guard]" << std::endl;

    Logger log("test_raii.ndc");
    log.removeAllAppenders();
    log.setLogLevel(LogLevel::DEBUG);
    log.addFileAppender("test_raii.ndc", "%D{%Y-%m-%d %H:%M:%S} [%x] %-5p %c - %m%n", false, true);
    log.info("[NDCContext RAII guard]");
    ndcClear();

    {
        NDCContext guard1("outer");
        log.info("inside guard1, get() == \"outer\"");
        check("inside guard1, get() == \"outer\"", ndcGet() == "outer");

        {
            NDCContext guard2("inner");
            log.info("inside guard2, get() contains \"inner\"");
            check("inside guard2, get() contains \"inner\"",
                  ndcGet().find("inner") != std::string::npos);
        }

        log.info("after guard2 destroyed, \"inner\" removed");
        check("after guard2 destroyed, \"inner\" removed",
              ndcGet().find("inner") == std::string::npos);
        log.info("after guard2 destroyed, \"inner\" removed");
        check("outer still present", ndcGet() == "outer");
    }

    log.info("after guard1 destroyed, NDC is empty");
    check("after guard1 destroyed, NDC is empty", ndcGet().empty());

    ndcRemove();
    log.info("ndcRemove() called");
}

void test_ndc_in_log_output() {
    std::cout << "\n[NDC in log output]" << std::endl;

    const std::string logFile = "test_ndc_output.log";
    std::remove(logFile.c_str());

    Logger log(logFile);
    log.removeAllAppenders();
    log.setLogLevel(LogLevel::DEBUG);
    log.addFileAppender("test_ndc_output.log", "%D{%Y-%m-%d %H:%M:%S} [%x] %-5p - %m%n", false, true);
    log.info("init ndc output");
    //log.debug()<< "[NDC in log output]" << std::endl;

    ndcClear();

    log.debug("msg_no_ndc");

    ndcPush("request-42");
    log.debug("msg_with_ndc");
    ndcPop();

    {
        NDCContext guard("session-99");
        log.info("msg_raii_ndc");
    }

    log.info("msg_after_raii");

    std::string content = readFile(logFile);

    check("msg without NDC has empty context []",
          content.find("[] msg_no_ndc") != std::string::npos);
    check("msg with NDC has [request-42]",
          content.find("[request-42] msg_with_ndc") != std::string::npos);
    check("RAII guard produces [session-99]",
          content.find("[session-99] msg_raii_ndc") != std::string::npos);
    check("after RAII scope, context is empty again",
          content.find("[] msg_after_raii") != std::string::npos);

    log.removeAllAppenders();
    ndcRemove();
    //std::remove(logFile.c_str());
}

int main() {
    test_ndc_basic();
    test_ndc_clear();
    test_ndc_raii();
    test_ndc_in_log_output();

    std::cout << "\n=== NDC Tests: " << g_passed << " passed, "
              << g_failed << " failed ===" << std::endl;
    return g_failed > 0 ? 1 : 0;
}
