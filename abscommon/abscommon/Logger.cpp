#include "stdafx.h"
#include "Logger.h"

namespace IO {

std::unique_ptr<Logger> Logger::instance_ = nullptr;
std::string Logger::logDir_ = "";

Logger& Logger::Instance()
{
    if (!instance_)
    {
        if (!logDir_.empty())
        {
            std::chrono::time_point<std::chrono::system_clock> time_point;
            time_point = std::chrono::system_clock::now();
            std::time_t ttp = std::chrono::system_clock::to_time_t(time_point);

            struct tm* p;
            p = localtime(&ttp);
            char chr[50] = { 0 };
            strftime(chr, 50, "(%g-%m-%d %H:%M:%S)", p);

            std::string logFile = logDir_ + "/" + std::string(chr) + ".log";
            instance_ = std::make_unique<Logger>(logFile);
        }
        else
            instance_ = std::make_unique<Logger>();
    }
    return *instance_.get();
}

int Logger::PrintF(const char *__restrict __format, ...)
{
    int ret;

    /* Declare a va_list type variable */
    va_list myargs;

    /* Initialise the va_list variable with the ... after fmt */

    va_start(myargs, __format);

    char buff[1024] = { 0 };
    /* Forward the '...' to vprintf */
    ret = vsprintf(buff, __format, myargs);

    std::string msg(buff, ret);
    Instance() << msg;
    if (msg.back() == '\n')
        Instance().nextIsBegin_ = true;

    /* Clean up the va_list */
    va_end(myargs);

    return ret;
}

}

