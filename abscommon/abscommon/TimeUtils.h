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
