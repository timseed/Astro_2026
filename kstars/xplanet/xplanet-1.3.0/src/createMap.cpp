#include <cmath>
#include <cstring>
#include <map>
#include <sstream>
#include <string>
using namespace std;

#include "body.h"
#include "findFile.h"
#include "Map.h"
#include "Options.h"
#include "PlanetProperties.h"
#include "Ring.h"
#include "xpUtil.h"

#include "libimage/Image.h"
#include "libplanet/Planet.h"

extern void
loadSSEC(Image *&image, const unsigned char *&rgb, string &imageFile, 
         const int imageWidth, const int imageHeight);

static void
loadRGB(Image *&image, const unsigned char *&rgb, string &imageFile, 
        const string &name, const int imageWidth, const int imageHeight,
        const int shift)
{
    bool foundFile = findFile(imageFile, "images");
    if (foundFile) 
    {
        image = new Image;
        foundFile = image->Read(imageFile.c_str());
    }
    
    if (foundFile)
    {
        if ((image->Width() != imageWidth)
            || (image->Height() != imageHeight))
        {
            ostringstream errStr;
            errStr << "Resizing " << name << " map\n"
                   << "For better performance, all image maps should "
                   << "be the same size as the day map\n";
            xpWarn(errStr.str(), __FILE__, __LINE__);
            image->Resize(imageWidth, imageHeight);
        }
        if (shift != 0) image->Shift(shift);
        rgb = image->getRGBData();
    }
    else
    {
        ostringstream errStr;
        errStr << "Can't load map file " << imageFile << "\n";
        xpWarn(errStr.str(), __FILE__, __LINE__);
    }
}

Map *
createMap(const double sLat, const double sLon, 
          const double obsLat, const double obsLon, 
          const int width, const int height, 
          const double pR,
          Planet *planet, Ring *ring, 
          map<double, Planet *> &planetsFromSunMap,
          PlanetProperties *planetProperties)
{
    Map *m = NULL;

    string imageFile(planetProperties->DayMap());

    Image *day = new Image;

    bool foundFile = false;
    if (imageFile.compare("none") != 0) 
    {
       findFile(imageFile, "images");
       foundFile = day->Read(imageFile.c_str());
    }

    if (!foundFile)
    {
        // If the day map isn't found, assume the other maps won't be
        // found either

        int xsize = (int) (pR*4);
        if (xsize < 128) xsize = 128;
        if (xsize > width) xsize = width;
        int ysize = xsize / 2;
        m = new Map(xsize, ysize, sLat, sLon, 
                    planet, planetProperties,
                    ring, planetsFromSunMap);
    }
    else
    {
        int imageWidth = day->Width();
        int imageHeight = day->Height();

        int ishift = 0;
        Options *options = Options::getInstance();
        if (options->GRSSet() && planet->Index() == JUPITER)
        {
            double shift = (fmod(planet->Flipped()
                                 * (options->GRSLon()/360 
                                    + 0.5), 1.0));
            shift *= imageWidth;
            ishift = static_cast<int> (-shift);
            if (ishift != 0 && day != NULL) day->Shift(ishift);
        }

        const unsigned char *dayRGB = day->getRGBData();

        Image *night = NULL;
        const unsigned char *nightRGB = NULL;
        Image *bump = NULL;
        const unsigned char *bumpRGB = NULL;
        Image *cloud = NULL;
        const unsigned char *cloudRGB = NULL;
        Image *specular = NULL;
        const unsigned char *specularRGB = NULL;

        imageFile = planetProperties->NightMap();
        if (!imageFile.empty() && planetProperties->Shade() < 1) 
            loadRGB(night, nightRGB, imageFile, "night", 
                    imageWidth, imageHeight, ishift);

        imageFile = planetProperties->BumpMap();
        if (!imageFile.empty())
            loadRGB(bump, bumpRGB, imageFile, "bump", 
                    imageWidth, imageHeight, ishift);
        
        imageFile = planetProperties->SpecularMap();
        if (!imageFile.empty())
            loadRGB(specular, specularRGB, imageFile, "specular",
                    imageWidth, imageHeight, ishift);
        
        imageFile = planetProperties->CloudMap();
        if (!imageFile.empty())
        {
            if (planetProperties->SSECMap())
            {
                loadSSEC(cloud, cloudRGB, imageFile, imageWidth, imageHeight);
            }
            else
            {
                loadRGB(cloud, cloudRGB, imageFile, "cloud", 
                        imageWidth, imageHeight, ishift);
            }
        }
        
        m = new Map(imageWidth, imageHeight, 
                    sLat, sLon, obsLat, obsLon, 
                    dayRGB, nightRGB, bumpRGB, specularRGB, cloudRGB, 
                    planet, planetProperties, ring, planetsFromSunMap);
        
        delete night;
        delete bump;
        delete cloud;
        delete specular;

        // If the map dimensions are each a power of two, the map size
        // will be reduced to get rid of high-frequency noise
        const double log2 = log(2.0);
        double e = log((double) imageWidth) / log2;
        double remainder = fabs(e - floor(e+0.5));
        if (remainder < 1e-3)
        {
            e = log((double) imageHeight) / log2;
            remainder = fabs(e - floor(e+0.5));
            if (remainder < 1e-3) 
            {
                // optimal size for image is about 4*pR x 2*pR
                const double ratio = day->Height()/pR;
                const int factor = (int) (log(ratio)/log2) - 1;
                m->Reduce(factor);
            }
        }
    }

    delete day;

    return(m);
}    
