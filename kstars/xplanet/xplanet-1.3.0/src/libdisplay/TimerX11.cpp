#include <cstdlib>
#include <sstream>
using namespace std;

#include <sys/time.h>
#include <unistd.h>

#include "config.h"

#include "keywords.h"
#include "Options.h"
#include "xpUtil.h"

#include "TimerX11.h"

#include "libdisplay/DisplayX11.h"

Display *TimerX11::display_ = NULL;

TimerX11::TimerX11(const int w, const unsigned long h, 
                   const unsigned long i) : Timer(w, h, i)
{
    display_ = XOpenDisplay(NULL);

#ifdef HAVE_XSS
    screenSaverInfo_ = NULL;
    if (display_)
    {
        const int screen_num = DefaultScreen(display_);
        root_ = RootWindow(display_, screen_num);
        int event_base, error_base;
        
        if (XScreenSaverQueryExtension(display_, &event_base, &error_base))
            screenSaverInfo_ = XScreenSaverAllocInfo();
    }
#endif
}

TimerX11::~TimerX11()
{
    XCloseDisplay(display_);

#ifdef HAVE_XSS
    XFree(screenSaverInfo_);
#endif
}

// Sleep for sleep_time seconds.  Also check if this is a window
// that's been closed, in which case the program should quit.
bool
TimerX11::SleepForTime(time_t sleep_time)
{
    if (sleep_time <= 0) 
        return(true);

    gettimeofday(&currentTime_, NULL);
    nextUpdate_ = sleep_time + currentTime_.tv_sec;
        
    Options *options = Options::getInstance();
    if (static_cast<int> (sleep_time) > 1)
    {
        if (options->Verbosity() > 0)
        {
            ostringstream msg;
            msg << "sleeping for " << static_cast<int> (sleep_time) 
                << " seconds until " << ctime((time_t *) &nextUpdate_);
            xpMsg(msg.str(), __FILE__, __LINE__);
        }
    }

    if (options->DisplayMode() == WINDOW)
    {
        Window window = DisplayX11::WindowID();
        Atom wmDeleteWindow = XInternAtom(display_, 
                                          "WM_DELETE_WINDOW", 
                                          False);
        
        XSetWMProtocols(display_, window, &wmDeleteWindow, 1);
        XSelectInput(display_, window, KeyPressMask);
        XEvent event;

        // Check every 1/10th of a second if there's been a request to
        // kill the window
        while (currentTime_.tv_sec < nextUpdate_)
        {
            if (XCheckTypedWindowEvent(display_, window, 
                                       ClientMessage, &event) == True) 
            {
                if ((unsigned int) event.xclient.data.l[0] == wmDeleteWindow)
                    return(false);
            }
            else if (XCheckTypedWindowEvent(display_, window,
                                            KeyPress, &event) == True)
            {
                KeySym keysym;
                char keybuf;
                XLookupString(&(event.xkey), &keybuf, 1, &keysym, NULL);
                if (keybuf == 'q' || keybuf == 'Q') 
                    return(false);
            }

            usleep(100000);  // sleep for 1/10 second
            gettimeofday(&currentTime_, NULL);
        }
    }
    else
    {
        // Check every second if we've reached the time for the next
        // update.
        while (currentTime_.tv_sec < nextUpdate_)
        {
            sleep(1);
            gettimeofday(&currentTime_, NULL);
        }
    }

    return(true);
}


// returns false if the program should exit after this sleep
bool
TimerX11::Sleep()
{
    // Sleep until the next update
    gettimeofday(&currentTime_, NULL);
    if (!SleepForTime(nextUpdate_ - currentTime_.tv_sec)) 
        return(false);

#ifdef HAVE_XSS
    // If the display has not been idle for idlewait_
    // milliseconds, keep sleeping.  Check every second until the
    // display has been idle for long enough.
    if (idlewait_ > 0) 
    {
        unsigned long idle = GetSystemIdleTime();
        Options *options = Options::getInstance();
        if (options->Verbosity() > 0)
        {
            ostringstream msg;
            msg << "Idle time is " << idle/1000 << " second";
            if (idle/1000 != 1) msg << "s";
            msg << endl;
            xpMsg(msg.str(), __FILE__, __LINE__);
        }
        while (idle < idlewait_) 
        {
            gettimeofday(&currentTime_, NULL);
            if (!SleepForTime((idlewait_ - idle) / 1000))
                return(false);
            idle = GetSystemIdleTime();
        }
    }
    
    // If the display has been idle for longer than hibernate_
    // milliseconds, keep sleeping.  Check every second until
    // something happens.
    if (hibernate_ > 0)
    {
        unsigned long idle = GetSystemIdleTime();
        Options *options = Options::getInstance();
        if (options->Verbosity() > 0 && idle > hibernate_)
            xpMsg("Hibernating ...\n", __FILE__, __LINE__);

        while (idle > hibernate_) 
        {
            if (!SleepForTime(1)) 
                return(false);
            idle = GetSystemIdleTime();
        }
    }
#endif
    return(true);
}

// return the system idle time in milliseconds
unsigned long
TimerX11::GetSystemIdleTime()
{
    unsigned long idle = 0;
#ifdef HAVE_XSS
    if (screenSaverInfo_ != NULL)
    {
        XScreenSaverQueryInfo(display_, root_, screenSaverInfo_);
        idle = screenSaverInfo_->idle;
    }
#endif
    return(idle);
}
