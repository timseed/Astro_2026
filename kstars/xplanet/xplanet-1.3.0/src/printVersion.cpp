#include <iostream>
using namespace std;

#include "config.h"
#include "xpUtil.h"

#ifdef HAVE_LIBFREETYPE 
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#endif

#ifdef HAVE_CSPICE
#include <SpiceUsr.h>
#endif

void
printVersion()
{
    cout << "Xplanet " << VERSION << endl;
    cout << "Copyright (C) 2012 "
         << "Hari Nair <hari@alumni.caltech.edu>" << endl;
    cout << "The latest version can be found at "
         << "http://xplanet.sourceforge.net\n";
    cout << "Compiled with support for:\n";
#ifdef HAVE_AQUA
    cout << "\tMac OS X\n";
#endif

#ifdef HAVE_CYGWIN
    cout << "\tCygwin\n";
#endif

#ifdef HAVE_LIBX11 
    cout << "\tX11";
# ifdef HAVE_XSS 
    cout << " (with screensaver extensions)";
# endif
    cout << endl;
#endif

#ifdef HAVE_LIBGIF 
    cout << "\tGIF\n";
#endif

#ifdef HAVE_LIBJPEG 
    cout << "\tJPEG\n";
#endif

#ifdef HAVE_LIBPNG 
    cout << "\tPNG\n";
#endif

#ifdef HAVE_LIBPNM 
    cout << "\tPBM\n";
#endif

#ifdef HAVE_LIBTIFF 
    cout << "\tTIFF\n";
#endif

#ifdef HAVE_LIBFREETYPE 
    FT_Library library;
    const int error = FT_Init_FreeType(&library);
    if (error)
        xpExit("Can't initialize freetype library\n", __FILE__, __LINE__);

    FT_Int amajor, aminor, apatch;
    FT_Library_Version(library, &amajor, &aminor, &apatch);
    
    cout << "\tFreeType (version " 
         << amajor << "." << aminor << "." << apatch << ")\n";

    FT_Done_FreeType(library);
#endif

#ifdef HAVE_LIBPANGOFT2 
    cout << "\tPango\n";
#endif

#ifdef HAVE_CSPICE
    cout << "\t" << tkvrsn_c( "TOOLKIT" ) << endl;
#endif
}
