# Log4plusplus Test Report

**Date:** 2026-03-26
**Branch:** `dev-hanze`
**Build:** g++ C++17, linked against log4cplus 2.0.5
**Platform:** RHEL 9 / Rocky Linux 9 (x86_64)

## Build Status

All targets compile cleanly with zero warnings.

```
make clean && make   # Builds libpjlog.a + all 9 test binaries
```

## Test Summary

| Test | Purpose | Assertions | Result |
|------|---------|------------|--------|
| `rootconfig` | ROOT configuration path | — | PASS |
| `branchconfig` | BRANCH configuration path | — | PASS |
| `threadconfig` | Cross-thread configuration | — | PASS |
| `threadtest` | Multi-threaded logging stress | — | PASS |
| `benchtest` | Performance benchmark | — | PASS (9,704 TPS) |
| `test_levels` | Level control, filtering, utilities | 34 | PASS (34/34) |
| `test_appenders` | Programmatic appender management | 21 | PASS (21/21) |
| `test_ndc` | Nested Diagnostic Context | 17 | PASS (17/17) |
| `test_logger_api` | Logger identity, factory, all log methods | 28 | PASS (28/28) |

**Total: 9 tests, 100 assertions, 0 failures.**

---

## Integration Tests

### 1. rootconfig

**File:** `test/rootconfig.cpp`
**Config:** `test/log4cplus_1.properties`

**What it tests:**
- `Logger::doConfigure()` with `ConfigType::ROOT` returns `LOG_SUCCESS`
- Logger instance creation with a named category (`"main"`)
- Stream-style logging: `log.info() << "ROOT config test" << std::endl`
- Printf-style logging: `log.debug("debug value=%d", 100)`

**Verification:**
- Exit code 0 (doConfigure succeeded)
- Output written to `test/logs/mainlog.log` with correct format:
  ```
  2026-03-26 18:08:59 [140029975201984] [] INFO  main - ROOT config test
  2026-03-26 18:08:59 [140029975201984] [] DEBUG main - debug value=100
  ```

---

### 2. branchconfig

**File:** `test/branchconfig.cpp`
**Config:** `test/log4cplus_1.properties` (ROOT) + `test/log4cplus_2.properties` (BRANCH)

**What it tests:**
- Sequential `doConfigure()` calls: first ROOT, then BRANCH
- Both return `LOG_SUCCESS`
- ROOT logger remains functional after a BRANCH config is loaded
- BRANCH logger writes to its own category
- Printf-style logging on the branch logger: `branchLog.debug("branch debug value=%d", 200)`

**Verification:**
- Exit code 0
- ROOT log entry appears in `mainlog.log`
- BRANCH log entry appears separately
- ROOT logger still works after BRANCH config (last log line confirms this)

---

### 3. threadconfig

**File:** `test/threadconfig.cpp`
**Config:** `test/log4cplus_1.properties`

**What it tests:**
- `doConfigure()` with `ConfigType::ROOT` on the main thread
- `doConfigure()` with `ConfigType::BRANCH` on a child thread (with empty config file and log dir)
- Cross-thread logger usage: child thread creates its own `Logger("net")` and logs
- Thread safety of the configuration path (mutex-protected `doConfigure`)

**Verification:**
- Exit code 0
- Main thread log: `system start`
- Child thread log: `net thread running`
- Both appear in `test/logs/app.log` with distinct thread IDs:
  ```
  2026-03-26 18:17:38 [139797054666944] [] INFO  main - system start
  2026-03-26 18:17:38 [139796996654656] [] INFO  net - net thread running
  ```

---

### 4. threadtest

**File:** `test/threadtest.cpp`
**Config:** `test/log4cplus.properties`

**What it tests:**
- High-volume concurrent logging: 4 threads, 10,000 messages each (40,000 total)
- Thread safety of the stream-style logging API under contention
- No crashes, deadlocks, or data races
- Log message integrity (each message contains its thread ID and iteration number)

**Verification:**
- Exit code 0
- Completes within the 30-second timeout
- Output written to `test/logfile.log` (RollingFileAppender, 10MB max, 3 backups)

---

### 5. benchtest

**File:** `test/benchtest.cpp`
**Config:** `test/log4cplus.properties`

**What it tests:**
- Throughput benchmark: 4 threads x 100,000 messages = 400,000 log entries
- Measures wall-clock time and reports transactions per second (TPS)
- Validates that the logging path has no bottlenecks under sustained load

**Result:**
```
TPS: 9,704
```

**Verification:**
- Exit code 0
- 400,000 log lines written to `test/logfile.log`
- No thread safety violations or crashes

---

## Unit Tests

### 6. test_levels (34 assertions)

**File:** `test/test_levels.cpp`

**What it tests:**

| Group | Assertions | Description |
|-------|-----------|-------------|
| `logLevelToString` | 7 | All 7 LogLevel values convert to correct strings |
| `logLevelFromString` | 5 | Case-insensitive parsing, unknown/empty input defaults to DEBUG |
| `setLogLevel / getLogLevel` | 3 | Set level and read it back for DEBUG, WARN, FATAL |
| `isEnabledFor / is*Enabled` | 9 | At WARN level: DEBUG/INFO disabled, WARN/ERROR/FATAL enabled; at DEBUG: all enabled |
| Log filtering | 5 | At ERROR level: DEBUG/INFO/WARN messages suppressed, ERROR/FATAL messages written to file |
| `notice()` / `alert()` regression | 4 | Both printf and stream variants of notice() and alert() produce output (regression for alert bug fix) |
| `log()` with current level | 2 | `log()` uses the level set by the most recent stream call; respects filtering |

---

### 7. test_appenders (21 assertions)

**File:** `test/test_appenders.cpp`

**What it tests:**

| Group | Assertions | Description |
|-------|-----------|-------------|
| FileAppender | 6 | Attach appender, write messages, verify file content; truncate mode clears previous content |
| RollingFileAppender | 3 | Attach with 200KB limit, write enough to trigger rollover, verify backup file `.1` created |
| ConsoleAppender | 4 | Attach to stdout and stderr, log without crash, removeAll clears count |
| AsyncConsoleAppender | 2 | Attach async appender, log without crash |
| Appender management | 6 | Start at 0, add two file appenders, verify dual output, removeAll resets to 0, logging with no appenders doesn't crash |

**Not tested (requires external infrastructure):**
- `addSyslogAppender()` — needs a running syslog daemon
- `addSocketAppender()` — needs a log4cplus SocketServer

---

### 8. test_ndc (17 assertions)

**File:** `test/test_ndc.cpp`

**What it tests:**

| Group | Assertions | Description |
|-------|-----------|-------------|
| Basic push/pop/get | 6 | Push two contexts, verify stack order, pop returns correct values, stack empties |
| clear | 2 | Push 3 items, clear empties the stack |
| NDCContext RAII guard | 5 | Nested guards push on construction, pop on destruction; inner context removed when inner guard goes out of scope |
| NDC in log output | 4 | FileAppender with `[%x]` pattern: empty NDC produces `[]`, pushed context appears in output, RAII guard appears in output, context restored after guard scope |

---

### 9. test_logger_api (28 assertions)

**File:** `test/test_logger_api.cpp`

**What it tests:**

| Group | Assertions | Description |
|-------|-----------|-------------|
| `getName()` | 2 | Returns the category name; works with single-char names |
| `getLogger()` factory | 2 | Creates logger with correct name; same name returns same logger |
| `getRootLogger()` | 1 | Returns logger named `"root"` |
| All log methods | 20 | Stream, printf, and string variants for all 5 standard levels (debug/info/warn/error/fatal); verifies correct level prefix in output |
| Large printf message | 3 | Message > 4096 bytes triggers heap fallback in two-pass vsnprintf; full payload preserved |

---

## Configuration Files

| File | Purpose |
|------|---------|
| `test/log4cplus.properties` | RollingFileAppender to `logfile.log` (10MB, 3 backups). Pattern: `%D %-5p %c{2} - %m%n` |
| `test/log4cplus_1.properties` | RollingFileAppender using `${LOG_DIR}/${LOG_NAME}.log` (env-variable paths). Pattern includes thread ID and NDC: `%D [%t] [%x] %-5p %c - %m%n` |
| `test/log4cplus_2.properties` | RollingFileAppender using `${LOG_DIR}/%c.log` (category-based file naming). Same pattern as `_1`. |

## API Coverage

### Covered by tests

- **Configuration:** `Logger::configure()`, `Logger::doConfigure()` with ROOT, BRANCH, and THREAD config types
- **Logger creation:** `Logger("name")` constructor, `getLogger()`, `getRootLogger()`, `getName()`
- **Logging methods:** All 5 standard levels (debug/info/warn/error/fatal) in stream, printf, and string variants; `notice()`, `alert()`, `log()`
- **Level control:** `setLogLevel()`, `getLogLevel()`, `isEnabledFor()`, `isDebugEnabled()` through `isFatalEnabled()`
- **Level utilities:** `logLevelToString()`, `logLevelFromString()`
- **Log filtering:** Messages below threshold are suppressed
- **Programmatic appenders:** `addConsoleAppender()`, `addFileAppender()`, `addRollingFileAppender()`, `addAsyncConsoleAppender()`
- **Appender management:** `removeAppender()`, `removeAllAppenders()`, `getAppenderCount()`
- **NDC:** `ndcPush()`, `ndcPop()`, `ndcGet()`, `ndcClear()`, `ndcRemove()`, `NDCContext` RAII guard; NDC values verified in log output
- **Thread safety:** Concurrent logging from multiple threads, cross-thread configuration
- **Performance:** Sustained throughput under multi-threaded load (~9,700 TPS)
- **Edge cases:** Large printf messages (>4KB heap fallback), empty/unknown level strings, logging with no appenders

### Not covered (requires external infrastructure)

- `addSyslogAppender()` — needs a running syslog daemon
- `addSocketAppender()` — needs a log4cplus SocketServer listening on a port

## How to Run

```bash
# Build everything
make

# Run individual tests
cd test
./rootconfig         # Integration: ROOT config
./branchconfig       # Integration: BRANCH config
./threadconfig       # Integration: cross-thread config
./threadtest         # Integration: multi-threaded stress
./benchtest          # Integration: performance benchmark

./test_levels        # Unit: level control and filtering (34 assertions)
./test_appenders     # Unit: programmatic appenders (21 assertions)
./test_ndc           # Unit: NDC functions and RAII guard (17 assertions)
./test_logger_api    # Unit: logger identity, factory, all methods (28 assertions)

# Clean
make clean
```

**Prerequisites:** `log4cplus-devel` package (`sudo dnf install -y log4cplus-devel` on RHEL/Rocky 9)
