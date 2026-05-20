#include "Map.h"
#include "Options.h"
#include "PlanetProperties.h"
#include "View.h"
#include "xpUtil.h"

#include "libdisplay/libdisplay.h"
#include "libplanet/Planet.h"

void
drawEllipsoid(const double pX, const double pY, const double pR, 
              const double oX, const double oY, const double oZ, 
              const double X, const double Y, const double Z,
              DisplayBase *display, 
              const View *view, const Map *map, Planet *planet,
              PlanetProperties *planetProperties)
{
    double lat, lon;
    unsigned char color[3];

    const int j0 = 0;
    const int j1 = display->Height();
    const int i0 = 0;
    const int i1 = display->Width();

    // P1 (Observer) is at (oX, oY, oZ), or (0, 0, 0) in view
    // coordinates
    // P2 (current pixel) is on the line from P1 to (vX, vY, vZ)
    // P3 (Planet center) is at (X, Y, Z) in heliocentric rectangular
    // Now find the intersection of the line with the planet's ellipsoid. 

    // Define a new coordinate system: planetary XYZ
    // X = 0 lon, Z = north pole, Y = Z x X
    // Convert P1, P2, P3 to planetary XYZ:
    // 1) Convert to planetocentric
    // 2) Convert to planetary XYZ (units of planetary radius)

    double p1X = 0, p1Y = 0, p1Z = 0;
    double p2X = 0, p2Y = 0, p2Z = 0;
    double p3X = 0, p3Y = 0, p3Z = 0;
            
    const double ratio = 1/(1 - planet->Flattening());

    planet->XYZToPlanetaryXYZ(oX, oY, oZ, p1X, p1Y, p1Z);
    p1Z *= ratio;

    const double planetRadius = planetProperties->Magnify();
    const double c = 2 * (dot(p1X - p3X, p1Y - p3Y, p1Z - p3Z, 
                              p1X - p3X, p1Y - p3Y, p1Z - p3Z) 
                          - planetRadius * planetRadius);

    Options *options = Options::getInstance();

    // compute the value of the determinant at the center of the body
    view->PixelToViewCoordinates(options->CenterX() - pX, 
                                 options->CenterY() - pY, 
                                 p2X, p2Y, p2Z);
    
    view->RotateToXYZ(p2X, p2Y, p2Z, p2X, p2Y, p2Z);
    planet->XYZToPlanetaryXYZ(p2X, p2Y, p2Z, p2X, p2Y, p2Z);
    p2Z *= ratio;
    const double centerA = 2 * (dot(p2X - p1X, p2Y - p1Y, p2Z - p1Z, 
                                    p2X - p1X, p2Y - p1Y, p2Z - p1Z));
    const double centerB = 2 * (dot(p2X - p1X, p2Y - p1Y, p2Z - p1Z, 
                                    p1X - p3X, p1Y - p3Y, p1Z - p3Z));
    const double centerDet = centerB * centerB - centerA * c;

    for (int j = j0; j < j1; j++)
    {
        for (int i = i0; i < i1; i++)
        {
            const double dX = options->CenterX() - i;
            const double dY = options->CenterY() - j;

            view->PixelToViewCoordinates(dX, dY, p2X, p2Y, p2Z);

            view->RotateToXYZ(p2X, p2Y, p2Z, p2X, p2Y, p2Z);
            planet->XYZToPlanetaryXYZ(p2X, p2Y, p2Z, p2X, p2Y, p2Z);
            p2Z *= ratio;

            const double a = 2 * (dot(p2X - p1X, p2Y - p1Y, p2Z - p1Z, 
                                      p2X - p1X, p2Y - p1Y, p2Z - p1Z));
            const double b = 2 * (dot(p2X - p1X, p2Y - p1Y, p2Z - p1Z, 
                                      p1X - p3X, p1Y - p3Y, p1Z - p3Z));
            const double determinant = b*b - a * c;

            if (determinant < 0) continue;

            double u = -(b + sqrt(determinant));
            u /= a;

            // if the intersection point is behind the observer, don't
            // plot it
            if (u < 0) continue;

            // coordinates of the intersection point
            double iX, iY, iZ;
            iX = p1X + u * (p2X - p1X);
            iY = p1Y + u * (p2Y - p1Y);
            iZ = p1Z + u * (p2Z - p1Z);

            iZ /= ratio;

            planet->PlanetaryXYZToXYZ(iX, iY, iZ, iX, iY, iZ);
            planet->XYZToPlanetographic(iX, iY, iZ, lat, lon);

            map->GetPixel(lat, lon, color);
            double darkening = ndot(X - iX, Y - iY, Z - iZ, 
                                    X - oX, Y - oY, Z - oZ);
            if (darkening < 0) 
                darkening = 0;
            else
                darkening = photoFunction(darkening);

            for (int k = 0; k < 3; k++) 
                color[k] = static_cast<unsigned char> (color[k] 
                                                       * darkening);
            double opacity = 1;
            if (pR * determinant/centerDet < 10)
            {
                opacity = 1 - pow(1-determinant/centerDet, pR);
            }
            display->setPixel(i, j, color, opacity);
        }
    }
}
