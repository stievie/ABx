#include "stdafx.h"
#include "Logger.h"

#include "DebugNew.h"

namespace IO {

std::unique_ptr<Logger> Logger::instance_ = nullptr;
std::string Logger::logFile_ = "";

}

