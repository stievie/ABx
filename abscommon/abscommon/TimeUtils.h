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

#pragma once

#include <stdint.h>
#include <sys/timeb.h>
#include <time.h>
#include <string>

namespace Utils {

struct TimeSpan
{
    uint32_t months = 0;
    uint32_t days = 0;
    uint32_t hours = 0;
    uint32_t minutes = 0;
    uint32_t seconds = 0;
    TimeSpan(uint32_t sec)
    {
        time_t secs(sec); // you have to convert your input_seconds into time_t
        struct tm* ptm;
        ptm = gmtime(&secs); // convert to broken down time

        if (ptm->tm_yday > 31)
        {
            months = ptm->tm_yday / 31;
            days = ptm->tm_yday - (months * 31);
        }
        else
            days = ptm->tm_yday;
        hours = ptm->tm_hour;
        minutes = ptm->tm_min;
        seconds = ptm->tm_sec;
    }
};

}
