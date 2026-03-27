# Log4plusplus Test Report

**Date:** 2026-03-26
**Branch:** `dev-hanze`
**Build:** g++ C++17, linked against log4cplus 2.0.5
**Platform:** RHEL 9 / Rocky Linux 9 (x86_64)

## Build Status

All targets compile cleanly with zero warnings.

```
make clean && make   # Builds libpjlog.a + all 5 test binaries
```

## Test Summary

| Test | Purpose | Result |
|------|---------|--------|
| `rootconfig` | ROOT configuration path | PASS |
| `branchconfig` | BRANCH configuration path | PASS |
| `threadconfig` | Cross-thread configuration | PASS |
| `threadtest` | Multi-threaded logging stress | PASS |
| `benchtest` | Performance benchmark | PASS (9,684 TPS) |

All 5 tests exit with code 0.

---

## Test Details

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
TPS: 9,684
```

**Verification:**
- Exit code 0
- 400,000 log lines written to `test/logfile.log`
- No thread safety violations or crashes

---

## Configuration Files

| File | Purpose |
|------|---------|
| `test/log4cplus.properties` | RollingFileAppender to `logfile.log` (10MB, 3 backups). Pattern: `%D %-5p %c{2} - %m%n` |
| `test/log4cplus_1.properties` | RollingFileAppender using `${LOG_DIR}/${LOG_NAME}.log` (env-variable paths). Pattern includes thread ID and NDC: `%D [%t] [%x] %-5p %c - %m%n` |
| `test/log4cplus_2.properties` | RollingFileAppender using `${LOG_DIR}/%c.log` (category-based file naming). Same pattern as `_1`. |

## API Coverage

The tests exercise the following API surface:

- **Configuration:** `Logger::configure()`, `Logger::doConfigure()` with ROOT, BRANCH, and THREAD config types
- **Logger creation:** Named logger instances via `Logger("name")`
- **Stream-style logging:** `log.info() << ... << std::endl`
- **Printf-style logging:** `log.debug("format %d", value)`
- **Thread safety:** Concurrent logging from multiple threads, cross-thread configuration
- **Performance:** Sustained throughput under multi-threaded load

### Not yet covered by tests

The following APIs were implemented but do not have dedicated test programs:

- Level control: `setLogLevel()`, `getLogLevel()`, `isEnabledFor()`, `isDebugEnabled()` etc.
- Programmatic appenders: `addConsoleAppender()`, `addFileAppender()`, `addRollingFileAppender()`, `addSyslogAppender()`, `addSocketAppender()`, `addAsyncConsoleAppender()`
- Appender management: `removeAppender()`, `removeAllAppenders()`, `getAppenderCount()`
- Logger hierarchy: `getLogger()`, `getRootLogger()`, `getName()`
- NDC (Nested Diagnostic Context): `ndcPush()`, `ndcPop()`, `ndcGet()`, `ndcClear()`, `ndcRemove()`, `NDCContext` RAII guard
- Level utilities: `logLevelToString()`, `logLevelFromString()`
- Log methods: `log()` (generic level), `notice()`, `alert()`

## How to Run

```bash
# Build everything
make

# Run individual tests
cd test
./rootconfig
./branchconfig
./threadconfig
./threadtest
./benchtest

# Clean
make clean
```

**Prerequisites:** `log4cplus-devel` package (`sudo dnf install -y log4cplus-devel` on RHEL/Rocky 9)
