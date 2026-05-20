#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

#include <unistd.h>

#include "keywords.h"
#include "Options.h"
#include "PlanetProperties.h"
#include "xpUtil.h"

#include "DisplayMSWin.h"

#include "libimage/Image.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

DisplayMSWin::DisplayMSWin(const int tr) : DisplayBase(tr)
{
    fullWidth_ = GetSystemMetrics(SM_CXSCREEN);
    fullHeight_ = GetSystemMetrics(SM_CYSCREEN);

    Options *options = Options::getInstance();
    switch (options->DisplayMode())
    {
    case WINDOW:
        xpWarn("-window option not supported for MS Windows.\n",
               __FILE__, __LINE__);
        // fall through
    case ROOT:
        if (options->GeometrySelected())
        {
            width_ = options->getWidth();
            height_ = options->getHeight();
        }
        else
        {
            width_ = fullWidth_;
            height_ = fullHeight_;
        }
        break;
    }
  
    if (!options->CenterSelected())
    {
        if (width_ % 2 == 0)
            options->CenterX(width_/2 - 0.5);
        else
            options->CenterX(width_/2);

        if (height_ % 2 == 0)
            options->CenterY(height_/2 - 0.5);
        else
            options->CenterY(height_/2);
    }

    allocateRGBData();
}

DisplayMSWin::~DisplayMSWin()
{
}

void 
DisplayMSWin::renderImage(PlanetProperties *planetProperties[])
{
    drawLabel(planetProperties);

    string outputFilename(TmpDir());
    outputFilename += "\\XPlanet.bmp";

    Options *options = Options::getInstance();
    if (options->GeometrySelected()) PlaceImageOnRoot();

    Image i(fullWidth_, fullHeight_, rgb_data, alpha);
    if (!i.Write(outputFilename.c_str()))
    {
        ostringstream errStr;
        errStr << "Can't create image file " << outputFilename << "\n";
        xpExit(errStr.str(), __FILE__, __LINE__);
    }
    
    if (options->Verbosity() > 1)
    {
        ostringstream msg;
        msg << "Created image file " << outputFilename << "\n";
        xpMsg(msg.str(), __FILE__, __LINE__);
    }
    
    // Tell Windows to update the desktop wallpaper
    SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, 
                         (char *) outputFilename.c_str(), 
                         SPIF_UPDATEINIFILE);

    if (!options->SaveDesktopFile())
        unlinkFile(outputFilename.c_str());
}

string
DisplayMSWin::TmpDir()
{
    Options *options = Options::getInstance();

    string returnstring = options->TmpDir();
    if (returnstring.empty())
    {
        char tmpdir[MAX_PATH];
        GetWindowsDirectory(tmpdir, MAX_PATH);
        returnstring.assign(tmpdir);
    }
    return(returnstring);
}
