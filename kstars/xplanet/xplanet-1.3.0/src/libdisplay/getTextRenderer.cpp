#include "config.h"
#include "Options.h"
#include "xpUtil.h"

#include "TextRenderer.h"

#ifdef HAVE_LIBFREETYPE
#include "TextRendererFT2.h"
#endif

#ifdef HAVE_LIBPANGOFT2
#include "TextRendererPangoFT2.h"
#endif

TextRenderer *getTextRenderer(DisplayBase *display)
{
#ifdef HAVE_LIBPANGOFT2
    Options *options = Options::getInstance();
    if (options->Pango())
        return(new TextRendererPangoFT2(display));
#endif
#ifdef HAVE_LIBFREETYPE
    return(new TextRendererFT2(display));
#endif
    return(new TextRenderer(display));
}
