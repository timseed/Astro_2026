#include "config.h"

#include "Timer.h"

#ifdef HAVE_LIBX11
#include "TimerX11.h"
#endif

#ifdef HAVE_AQUA
#include "TimerMacAqua.h"
#endif

Timer *getTimer(const int wait, const unsigned long hibernate, 
                const unsigned long idlewait)
{
#ifdef HAVE_LIBX11
    Display *d = XOpenDisplay(NULL);
    if (d != NULL) 
    {
        XCloseDisplay(d);
        return(new TimerX11(wait, hibernate, idlewait));
    }
#endif

#ifdef HAVE_AQUA
    return(new TimerMacAqua(wait, hibernate, idlewait));
#endif

    return(new Timer(wait, hibernate, idlewait));
}
