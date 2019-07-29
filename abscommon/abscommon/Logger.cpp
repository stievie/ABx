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
            strftime(chr, 50, "%g-%m-%d-%H-%M-%S", p);

            std::string logFile = logDir_ + "/" + std::string(chr) + ".log";
            instance_ = std::make_unique<Logger>(logFile);
        }
        else
            instance_ = std::make_unique<Logger>();
    }
    return *instance_;
}

Logger::Logger(std::ostream& stream /* = std::cout */) :
    stream_(stream),
    mode_(Mode::Stream),
    logStart_(Utils::Tick())
{
#if defined(AB_WINDOWS)
    // Get default console font color on Windows
    hConsole_ = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO Info;
    GetConsoleScreenBufferInfo(hConsole_, &Info);
    foregroundDefault_ = Info.wAttributes;
#endif
}

Logger& Logger::operator << (endlType endl)
{
#if !defined(AB_WINDOWS)
    static Color::Modifier def(Color::FG_DEFAULT);
#endif
    nextIsBegin_ = true;
    if (mode_ == Mode::Stream)
    {
#if !defined(AB_WINDOWS)
        stream_ << def;
#else
        SetConsoleTextAttribute(hConsole_, foregroundDefault_);
#endif
    }
    stream_ << endl;
    return *this;
}

Logger& Logger::Error()
{
    static Color::Modifier red(Color::FG_LIGHTRED);
    if (nextIsBegin_)
    {
        if (mode_ == Mode::Stream)
        {
#if !defined(AB_WINDOWS)
            stream_ << red;
#else
            SetConsoleTextAttribute(hConsole_, red.code_);
#endif
        }
        (*this) << "[ERROR] ";
    }
    return *this;
}

Logger& Logger::Info()
{
    if (nextIsBegin_)
        (*this) << "[Info] ";
    return *this;
}

Logger& Logger::Warning()
{
    static Color::Modifier yellow(Color::FG_LIGHTYELLOW);
    if (nextIsBegin_)
    {
        if (mode_ == Mode::Stream)
        {
#if !defined(AB_WINDOWS)
            stream_ << yellow;
#else
            SetConsoleTextAttribute(hConsole_, yellow.code_);
#endif
        }
        (*this) << "[Warning] ";
    }
    return *this;
}

#if defined(PROFILING)
Logger& Logger::Profile()
{
    static Color::Modifier green(Color::FG_LIGHTGREEN);
    if (nextIsBegin_)
    {
        if (mode_ == Mode::Stream)
        {
#if !defined(AB_WINDOWS)
            stream_ << green;
#else
            SetConsoleTextAttribute(hConsole_, green.code_);
#endif
        }
        (*this) << "[Profile] ";
    }
    return *this;
}
#endif

#if defined(_DEBUG)
Logger& Logger::Debug()
{
    static Color::Modifier grey(Color::FG_LIGHTGREY);
    if (nextIsBegin_)
    {
        if (mode_ == Mode::Stream)
        {
#if !defined(AB_WINDOWS)
            stream_ << grey;
#else
            SetConsoleTextAttribute(hConsole_, grey.code_);
#endif
        }
        (*this) << "[Debug] ";
    }
    return *this;
}
#endif

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

    std::string msg(buff, static_cast<size_t>(ret));
    Instance() << msg;
    if (msg.back() == '\n')
        Instance().nextIsBegin_ = true;

    /* Clean up the va_list */
    va_end(myargs);

    return ret;
}

}

