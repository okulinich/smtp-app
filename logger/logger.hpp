#pragma once

#include <iostream>
#include <sstream>

enum loglevel_e { logERROR, logWARNING, logINFO, logDEBUG };

class SimpleLogger
{
public:
    SimpleLogger(loglevel_e _loglevel = logERROR) {
        time_t now = time(0);
        char *timeStr = ctime(&now);

        if (timeStr)
        {
            timeStr[strlen(timeStr) - 1] = '\0';
            _buffer << '[' << timeStr << "] ";
        }

        if (_loglevel == logERROR)
            _buffer << "ERROR: ";
        else if (_loglevel == logWARNING)
            _buffer << "WARNING: ";
        else if (_loglevel == logINFO)
            _buffer << "INFO: ";
        else if (_loglevel == logDEBUG)
            _buffer << "DEBUG: ";
    }

    template <typename T>
    SimpleLogger & operator<<(T const & value)
    {
        _buffer << value;
        return *this;
    }

    ~SimpleLogger()
    {
        _buffer << std::endl;
        // This is atomic according to the POSIX standard
        // http://www.gnu.org/s/libc/manual/html_node/Streams-and-Threads.html
        std::cerr << _buffer.str();
    }

private:
    std::ostringstream _buffer;
};

#define LOG_ERROR SimpleLogger(logERROR)
#define LOG_WARNING SimpleLogger(logWARNING)
#define LOG_INFO SimpleLogger(logINFO)
#define LOG_DEBUG SimpleLogger(logDEBUG)
