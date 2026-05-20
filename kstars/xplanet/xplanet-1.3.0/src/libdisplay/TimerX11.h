#ifndef TIMERX11_H
#define TIMERX11_H

#include <ctime>

#include "config.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef HAVE_XSS
#include <X11/extensions/scrnsaver.h>
#endif

#include "Timer.h"

class TimerX11 : public Timer
{
 public:
    TimerX11(const int w, const unsigned long h, const unsigned long i);
    ~TimerX11();

    static Display *const DisplayID() { return(display_); };
    bool Sleep();

 private:
    static Display *display_;

    bool SleepForTime(time_t sleep);

#ifdef HAVE_XSS
    Window root_;
    XScreenSaverInfo* screenSaverInfo_;
#endif

    unsigned long GetSystemIdleTime();
};

#endif
