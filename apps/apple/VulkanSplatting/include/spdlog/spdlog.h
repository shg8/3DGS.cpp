#ifndef spdlog_h
#define spdlog_h

#include <stdio.h>

class spdlog {
public:

    template<typename... Args>
    static void debug(const char* fmt, Args... args) {
        log("DEBUG", fmt, args...);
    }

    template<typename... Args>
    static void info(const char* fmt, Args... args) {
        log("INFO", fmt, args...);
    }

    template<typename... Args>
    static void warn(const char* fmt, Args... args) {
        log("WARN", fmt, args...);
    }

    template<typename... Args>
    static void error(const char* fmt, Args... args) {
        log("ERROR", fmt, args...);
    }

    template<typename... Args>
    static void critical(const char* fmt, Args... args) {
        log("CRITICAL", fmt, args...);
    }

private:

    template<typename T>
    static std::string to_string(T value)
    {
        std::ostringstream os;
        os << value;
        return os.str();
    }

    template<typename... Args>
    static std::string fmt(const std::string& format, Args... args)
    {
        std::string result = format;
        std::string::size_type pos;
        int count = 0;
        std::string argStrings[] = { to_string(args)... };

        while ((pos = result.find("{}")) != std::string::npos)
        {
            if (count < sizeof...(args))
            {
                result.replace(pos, 2, argStrings[count++]);
            }
            else
            {
                throw std::runtime_error("Not enough arguments provided for format string");
            }
        }

        return result;
    }

    template<typename... Args>
    static void log(const char* level, const char* fmt, Args... args) {
        std::string message = spdlog::fmt(fmt, args...);
        printf("[%s] %s\n", level, message.c_str());
    }

};

#endif
