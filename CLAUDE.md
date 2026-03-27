# Log4plusplus — Project Rules for Claude Code

## Project Overview

This is **Log4plusplus**, an incomplete C++ logging library. Many functions are missing. The goal is to systematically implement all missing functionality, write tests, and maintain the library as new features are added or removed.

## Current State

- The codebase has implemented functions declared in the header files, with some comments written in Chinese.
- It should be missing formatting functions, and lacking in support to write into different destinations.
- Before implementing anything, always run reconnaissance to understand what exists and what doesn't.
- Never assume a function works just because it has a body — check for TODO comments, `throw std::runtime_error("not implemented")`, or empty bodies that silently do nothing.

## Build & Test

- Build system: **Makefile**
- Build command: `make`
- Test targets: check the Makefile for test-related targets (e.g., `make test`, `make check`, or direct compilation of test files in `test/`)
- Test files: `test/threadtest.cpp`, `test/benchtest.cpp`, `test/rootconfig.cpp`, `test/threadconfig.cpp`
- Config files: `test/log4cplus.properties`, `test/log4cplus_1.properties`, `test/log4cplus_2.properties`
- Always build after every implementation change. Always run tests after writing new tests.
- If the build or tests fail, fix the issue before moving on.

## Code Conventions

### Style
- Use `snake_case` for functions and variables, `PascalCase` for classes and structs (adjust if the existing code differs — existing code wins).
- Prefer `std::string_view` over `const std::string&` for read-only string parameters where the codebase already does so.
- All public API functions must have doc comments.

### Error Handling
- Match the existing error strategy. If the codebase uses exceptions, use exceptions. If it uses error codes, use error codes. Do not mix.
- Never silently swallow errors in logging internals — a broken logger that hides failures is worse than one that crashes.

### Thread Safety
- Assume logging will be called from multiple threads unless the codebase explicitly documents otherwise.
- Protect shared state with the same synchronization primitives already used in the codebase (e.g., `std::mutex`, `std::lock_guard`).
- If no thread safety patterns exist yet, use `std::mutex` with `std::lock_guard` and document the choice.

### Headers & Includes
- Use `#pragma once` if the existing headers do. Otherwise use include guards matching the existing pattern.
- Keep includes minimal. Prefer forward declarations in headers where possible.

## File Organization

- `include/Logger.h` — Main logger class declaration
- `include/exception.h` — Custom exception types
- `src/Logger.cpp` — Logger implementation
- `test/` — Test programs and `.properties` config files
- `Makefile` — Build configuration
- `README.md` — Project documentation
- Do not create new top-level directories without discussing it first.

## Architecture Notes

This library follows the **log4cplus / log4j** pattern:
- **Properties files** (`test/log4cplus*.properties`) indicate configuration-driven setup with appenders, layouts, and log levels.
- Expect concepts like: Logger hierarchy, Appenders (console, file, rolling file), Layouts (pattern, simple), Filters, and LogLevel/Priority.
- Thread-related test files (`threadtest.cpp`, `threadconfig.cpp`) confirm multi-threaded usage is a design requirement.
- `benchtest.cpp` indicates performance matters — avoid unnecessary allocations in hot paths.

## Implementation Rules

1. **Never modify a function signature** that exists in a public header without flagging it. Other code may depend on it.
2. **One batch at a time.** Implement related functions together (e.g., all formatters, or all sink operations), not scattered functions across unrelated subsystems.
3. **Write tests alongside implementation**, not after. For each function implemented, write at minimum: one happy-path test, one edge-case test, and one error/failure test.
4. **Check for callers.** Before implementing a function, grep for all call sites to understand what inputs it will actually receive and what callers expect.

## Deletion & Removal Rules

When removing a function or feature:
1. Remove the implementation from the source file.
2. Remove or update the declaration in the header.
3. Search for ALL callers across the entire codebase and update them.
4. Remove or update all tests that reference the removed function.
5. Update any documentation, README, or comments that mention it.
6. Build and run full test suite to confirm nothing is broken.

## Subagent Guidelines

### Explore Subagents
- Use Explore subagents for all codebase investigation. Keep exploration out of the main context.
- Good Explore tasks: "find all unimplemented functions", "map the inheritance hierarchy", "identify all log sinks and their status", "find all thread-safety primitives used."

### Plan Subagent
- Always plan before implementing a new batch of functions.
- The plan should include: which functions, in what order (respecting dependencies), what tests are needed, and any architectural concerns.

### Implementer Subagent
- Reads existing patterns before writing anything.
- Builds and runs tests after every change.
- Updates its memory with patterns, conventions, and architectural decisions it discovers.

### Test Writer Subagent
- Follows existing test patterns and framework.
- Tests must compile and pass before the subagent returns.
- Cover: happy path, edge cases (empty input, null, max size), error conditions, and thread safety where applicable.

## Subagent Invocation Rules
 
When delegating to any subagent, always include in the task prompt:
- Specific file paths relevant to the task (e.g., "implement the functions declared in `include/Logger.h` lines 45–82")
- Any decisions or discoveries from this session that the subagent needs (e.g., "we determined that all appenders inherit from `AppenderBase` and must override `doAppend()`")
- Clear success criteria (e.g., "done means: all 4 functions compile, tests pass, no warnings")
- If passing results from one subagent to another, include the relevant findings inline — do not assume the next subagent can see what the previous one returned
 
When the explorer returns its inventory, summarize the key findings and pass them to the implementer and test-writer explicitly. Do not assume they know what was discovered.

## Compaction Rules
 
When compacting, always preserve:
- The full list of files modified in this session and what changed in each
- The current implementation plan: which batch we're on, what's done, what's next
- All build and test commands, including any Makefile targets discovered
- The function inventory: which functions are implemented, stubbed, or missing
- Any architectural decisions made during this session and their rationale
- Active bugs or failing tests and their status
- Thread safety decisions and which shared state is protected by what
 
When compacting, you may discard:
- Full file contents that were read for reference (re-read them if needed later)
- Verbose build output from successful builds
- Exploration details that have already been summarized into a plan
- Superseded plans that were replaced by newer ones

## Dependency Installation
 
This project runs on a Linux VM. Sudo access is available.
 
When the explorer identifies missing libraries, headers, or tools:
1. Report what's missing and why it's needed.
2. Propose the install commands (e.g., `sudo apt-get install -y libboost-dev`).
3. Wait for developer approval before running `sudo` commands.
4. After installing, verify the dependency is available (e.g., check that the header exists, run `pkg-config --libs libname`, or do a test compile).
5. Record all installed dependencies in the session so they can be documented in the README.
 
Allowed install actions (after approval):
- `sudo apt-get install` for system packages
- `sudo apt-get update` to refresh package lists
- Building from source if a package isn't available via apt (flag this and explain first)

## Things to Never Do

- Never modify files outside the project directory.
- Never commit directly — ask the developer before doing so.
- Never install packages or run sudo without reporting and getting approval first.
- Never refactor working code while implementing missing functions — one concern at a time.
- Never generate placeholder or stub implementations that just pass (e.g., empty function bodies or `return {};`). If you can't implement it properly, say so.

## Memory & Knowledge Building

Subagents with `memory: project` should write concise notes about:
- Architectural patterns discovered (e.g., "all sinks inherit from BaseSink and override write()")
- Naming conventions (e.g., "private members use m_ prefix")
- Error handling patterns (e.g., "functions throw LogException on failure, never std::exception directly")
- Threading patterns (e.g., "Logger::instance() uses a std::call_once for initialization")
- Test patterns (e.g., "tests use a MockSink that captures output into a vector<string>")
