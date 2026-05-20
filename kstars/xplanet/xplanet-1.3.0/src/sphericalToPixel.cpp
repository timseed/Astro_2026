#include <cmath>
#include <cstdlib>
using namespace std;

#include "Options.h"
#include "View.h"
#include "xpUtil.h"

#include "libplanet/Planet.h"
#include "libprojection/ProjectionBase.h"

bool
sphericalToPixel(const double lat, const double lon, const double rad, 
                 double &X, double &Y, double &Z, Planet *planet, 
                 View *view, ProjectionBase *projection)
{
    bool returnVal = false;

    if (view != NULL)
    {
        if (planet == NULL)
        {
            RADecToXYZ(lon, lat, X, Y, Z);
        }
        else
        {
            planet->PlanetographicToXYZ(X, Y, Z, lat, lon, rad);
        }           

        Options *options = Options::getInstance();
        view->XYZToPixel(X, Y, Z, X, Y, Z);
        X += options->CenterX();
        Y += options->CenterY();
        
        // true if the point is in front of us
        returnVal = (Z > 0);
    }
    else if (projection != NULL)
    {
        returnVal = projection->sphericalToPixel(lon * planet->Flipped(), 
                                                 lat, X, Y);
        Z = 0;
    }
    else
    {
        xpExit("Both view and projection are NULL!\n", __FILE__, __LINE__);
    }

    return(returnVal);
}
