/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "Logger.h"
#include "StringUtils.h"

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

            std::string logFile = Utils::ConcatPath(logDir_, std::string(chr), ".log");
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
    if (&stream_ == &std::cout)
    {
        isConsole_ = true;
        SetConsoleOutputCP(CP_UTF8);
    }
#endif
}

Logger& Logger::operator << (EndlType endl)
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
#if defined(AB_WINDOWS)
    if (isConsole_)
        stream_ << std::flush;
#endif
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
    static Color::Modifier blue(Color::FG_LIGHTBLUE);
    if (nextIsBegin_)
    {
        if (mode_ == Mode::Stream)
        {
#if !defined(AB_WINDOWS)
            stream_ << blue;
#else
            SetConsoleTextAttribute(hConsole_, blue.code_);
#endif
        }
        (*this) << "[Profile] ";
    }
    return *this;
}
#endif

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

