#pragma once
// log4cplus/helpers/exception.h
#ifndef LOG4CPLUS_EXCEPTION_H
#define LOG4CPLUS_EXCEPTION_H

#include <exception>
#include <string>

namespace log4cplus {
    namespace helpers {

        class Exception : public std::exception {
        public:
            explicit Exception(const std::string& message)
                : message_(message) {
            }

            const char* what() const noexcept override {
                return message_.c_str();
            }

        private:
            std::string message_;
        };

    }  // namespace helpers
}  // namespace log4cplus

#endif  // LOG4CPLUS_EXCEPTION_H
