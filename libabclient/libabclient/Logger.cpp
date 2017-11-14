#include "stdafx.h"
#include "Logger.h"

#include "DebugNew.h"

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
            tm p;
            localtime_s(&p, &ttp);
            char chr[50];
            strftime(chr, 50, "%g-%m-%d-%H-%M-%S", (const tm*)&p);
            std::string logFile = logDir_ + "/" + std::string(chr) + ".log";
            instance_ = std::make_unique<Logger>(logFile);
        }
        else
            instance_ = std::make_unique<Logger>();
    }
    return *instance_.get();
}

}

