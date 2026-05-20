#ifndef TIMERMACAQUA_H
#define TIMERMACAQUA_H

#include <ctime>

#include "config.h"

#include "Timer.h"

class TimerMacAqua : public Timer
{
 public:
    TimerMacAqua(const int w, const unsigned long h, const unsigned long i);
    ~TimerMacAqua();

    bool Sleep();

 private:
    unsigned long GetSystemIdleTime();
};

#endif
