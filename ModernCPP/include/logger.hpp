#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace common {

// C++20 개념: 출력 가능한 타입
template<typename T>
concept Printable = requires(std::ostream& os, T value) {
    { os << value } -> std::same_as<std::ostream&>;
};

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    // C++20 삼방향 비교 연산자 사용
    static void log(Level level, std::string_view message) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::ostringstream oss;
        oss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
        
        switch(level) {
            case Level::DEBUG:   oss << "[DEBUG] "; break;
            case Level::INFO:    oss << "[INFO] "; break;
            case Level::WARNING: oss << "[WARNING] "; break;
            case Level::ERROR:   oss << "[ERROR] "; break;
        }
        
        oss << message;
        std::cout << oss.str() << std::endl;
    }

    // C++20 템플릿 단축 문법과 concept 사용
    template<Printable T>
    static void debug(const T& message) {
        log(Level::DEBUG, std::to_string(message));
    }

    static void debug(std::string_view message) {
        log(Level::DEBUG, message);
    }

    static void info(std::string_view message) {
        log(Level::INFO, message);
    }

    static void warning(std::string_view message) {
        log(Level::WARNING, message);
    }

    static void error(std::string_view message) {
        log(Level::ERROR, message);
    }
};

} // namespace common

#endif // LOGGER_HPP
