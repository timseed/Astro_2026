#include "config.h"

#include "keywords.h"
#include "Options.h"
#include "xpUtil.h"

#include "DisplayBase.h"

#ifdef HAVE_AQUA
#include "DisplayMacAqua.h"
#endif

#ifdef HAVE_CYGWIN
#include "DisplayMSWin.h"
#endif

#ifdef HAVE_LIBX11
#include "DisplayX11.h"
#endif

#include "DisplayOutput.h"

DisplayBase *getDisplay(const int times_run)
{
    Options *options = Options::getInstance();

    if (options->DisplayMode() == OUTPUT)
	return(new DisplayOutput(times_run));

#ifdef HAVE_LIBX11
    Display *d = XOpenDisplay(NULL);
    if (d != NULL) 
    {
        XCloseDisplay(d);
	return(new DisplayX11(times_run));
    }
#endif

#ifdef HAVE_AQUA        
    return(new DisplayMacAqua(times_run));
#endif

#ifdef HAVE_CYGWIN
    return(new DisplayMSWin(times_run));
#endif

    xpExit("Can't open display\n", __FILE__, __LINE__);
    return(NULL);
}
