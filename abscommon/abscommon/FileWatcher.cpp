/**
 * Copyright 2020 Stefan Ascher
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


#include "FileWatcher.h"
#include "Subsystems.h"
#include "Scheduler.h"

namespace IO {

FileWatcher::~FileWatcher()
{
    watching_ = false;
}

void FileWatcher::Start()
{
    if (watching_)
        return;
    watching_ = true;
    Update();
}

void FileWatcher::Stop()
{
    watching_ = false;
}

void FileWatcher::Update()
{
    if (!watching_)
        return;
    auto lastWriteTime = fs::last_write_time(path_);
    if (lastWriteTime > lastTime_)
    {
        if (lastTime_ != fs::file_time_type::min())
        {
            if (onChanged_)
                onChanged_();
        }
        lastTime_ = lastWriteTime;
    }
    auto* shed = GetSubsystem<Asynch::Scheduler>();
    shed->Add(Asynch::CreateScheduledTask(FILEWATCHER_INTERVAL, std::bind(&FileWatcher::Update, shared_from_this())));
}

}
