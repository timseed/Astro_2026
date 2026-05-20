#include "Map.h"
#include "Options.h"
#include "PlanetProperties.h"
#include "View.h"
#include "xpUtil.h"

#include "libdisplay/libdisplay.h"
#include "libmultiple/libmultiple.h"
#include "libmultiple/RayleighScattering.h"
#include "libplanet/Planet.h"

void
drawSphere(const double pX, const double pY, const double pR, 
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
    // Now find the intersection of the line with the planet's sphere.
    // This algorithm is from
    // http://astronomy.swin.edu.au/~pbourke/geometry/sphereline

    const double p1X = 0, p1Y = 0, p1Z = 0;
    double p2X, p2Y, p2Z;
    double p3X, p3Y, p3Z;

    view->RotateToViewCoordinates(X, Y, Z, p3X, p3Y, p3Z);

    const double planetRadius = planet->Radius() * planetProperties->Magnify();
    const double c = 2 * (dot(p1X - p3X, p1Y - p3Y, p1Z - p3Z, 
                              p1X - p3X, p1Y - p3Y, p1Z - p3Z) 
                          - planetRadius * planetRadius);
            
    Options *options = Options::getInstance();

    // compute the value of the determinant at the center of the body
    view->PixelToViewCoordinates(options->CenterX() - pX, 
                                 options->CenterY() - pY, 
                                 p2X, p2Y, p2Z);
    
    const double centerA = 2 * (dot(p2X - p1X, p2Y - p1Y, p2Z - p1Z, 
                                    p2X - p1X, p2Y - p1Y, p2Z - p1Z));
    const double centerB = 2 * (dot(p2X - p1X, p2Y - p1Y, p2Z - p1Z, 
                                    p1X - p3X, p1Y - p3Y, p1Z - p3Z));
    const double centerDet = centerB * centerB - centerA * c;

    double plX, plY, plZ;
    planet->getPosition(plX, plY, plZ);

    RayleighScattering *rayleighDisk = NULL;
    RayleighScattering *rayleighLimb = NULL;
    double rayleighScale = planetProperties->RayleighScale();
    if (rayleighScale > 0)
    {
        rayleighDisk = new RayleighScattering(planetProperties->RayleighFile());
        rayleighLimb = new RayleighScattering(planetProperties->RayleighFile());

        double radiansPerPixel = options->FieldOfView() / display->Width();
        double dX = plX - oX;
        double dY = plY - oY;
        double dZ = plZ - oZ;
        double targetDist = sqrt(dX*dX + dY*dY + dZ*dZ);

        double kmPerPixel = radiansPerPixel * targetDist * AU_to_km;
        double minRes = 2 * rayleighLimb->getScaleHeightKm() 
            * planetProperties->RayleighLimbScale();
        if (kmPerPixel > minRes)
        {
            delete rayleighLimb;
            rayleighLimb = NULL;
        }
    }

    for (int j = j0; j < j1; j++)
    {
        for (int i = i0; i < i1; i++)
        {
            const double dX = options->CenterX() - i;
            const double dY = options->CenterY() - j;

            view->PixelToViewCoordinates(dX, dY, p2X, p2Y, p2Z);

            const double a = 2 * (dot(p2X - p1X, p2Y - p1Y, p2Z - p1Z, 
                                      p2X - p1X, p2Y - p1Y, p2Z - p1Z));
            const double b = 2 * (dot(p2X - p1X, p2Y - p1Y, p2Z - p1Z, 
                                      p1X - p3X, p1Y - p3Y, p1Z - p3Z));
            const double determinant = b*b - a * c;

            double u;
            double iX, iY, iZ;

            if (rayleighLimb != NULL)
            {
                u = -b/a;
                iX = p1X + u * (p2X - p1X);
                iY = p1Y + u * (p2Y - p1Y);
                iZ = p1Z + u * (p2Z - p1Z);
                view->RotateToXYZ(iX, iY, iZ, iX, iY, iZ);

                double lon, lat, rad;
                planet->XYZToPlanetographic(iX, iY, iZ, lat, lon, rad);
                if (rad >= 1 || pR * determinant/centerDet < 10)
                {
                    double incidence = acos(ndot(iX-plX, iY-plY, iZ-plZ, 
                                                 -iX, -iY, -iZ));
                    double phase = acos(ndot(oX-iX, oY-iY, oZ-iZ, 
                                             -iX, -iY, -iZ));

                    double tanht = planet->Radius() * AU_to_km * 1e3 * (rad-1);
                    if (tanht < 0) tanht = 0;
                    if (planetProperties->RayleighLimbScale() > 0)
                        tanht /= planetProperties->RayleighLimbScale();
                    rayleighLimb->calcScatteringLimb(incidence, tanht, phase);

                    display->getPixel(i, j, color);
                    double opacity[3] = { 0, 0, 0 };
                    for (int ic = 0; ic < 3; ic++)
                    {
                        double thisColor = rayleighLimb->getColor(ic);
                        thisColor = (rayleighScale * 255 * thisColor);
                        if (thisColor > 255) thisColor = 255;
                        color[ic] = thisColor;
                        opacity[ic] = thisColor / 255;
                    }
                    display->setPixel(i, j, color, opacity);
                }
                rayleighLimb->clear();
            }

            if (determinant < 0) continue;

            u = -(b + sqrt(determinant));
            u /= a;

            // if the intersection point is behind the observer, don't
            // plot it
            if (u < 0) continue;

            // coordinates of the intersection point
            iX = p1X + u * (p2X - p1X);
            iY = p1Y + u * (p2Y - p1Y);
            iZ = p1Z + u * (p2Z - p1Z);

            view->RotateToXYZ(iX, iY, iZ, iX, iY, iZ);
            planet->XYZToPlanetographic(iX, iY, iZ, lat, lon);

            map->GetPixel(lat, lon, color);

            if (rayleighDisk != NULL)
            {
                double incidence = acos(ndot(iX-plX, iY-plY, iZ-plZ, 
                                             -iX, -iY, -iZ));
                double emission = acos(ndot(iX-plX, iY-plY, iZ-plZ, 
                                            oX-iX, oY-iY, oZ-iZ));
                double phase = acos(ndot(oX-iX, oY-iY, oZ-iZ, 
                                         -iX, -iY, -iZ));
                
                double emsScale = 1;
                if (planetProperties->RayleighEmissionWeight() > 0)
                    emsScale = pow(sin(emission), 
                                   planetProperties->RayleighEmissionWeight());

                rayleighDisk->calcScatteringDisk(incidence, emission, phase);
                for (int ic = 0; ic < 3; ic++)
                {
                    double thisColor = rayleighDisk->getColor(ic) * emsScale;
                    thisColor = (rayleighScale * 255 * thisColor + color[ic]);
                    if (thisColor > 255) thisColor = 255;
                    color[ic] = thisColor;
                }
                rayleighDisk->clear();
            } 

            double darkening = 1;
            if (planet->Index() != SUN && rayleighScale <= 0)
            {
                darkening = ndot(X - iX, Y - iY, Z - iZ, 
                                 X - oX, Y - oY, Z - oZ);
                if (darkening < 0) 
                    darkening = 0;
                else
                    darkening = photoFunction(darkening);
            }

            for (int k = 0; k < 3; k++) 
                color[k] = static_cast<unsigned char> (color[k] 
                                                       * darkening);
            double opacity = 1;
            if (pR * determinant/centerDet < 10)
                opacity = 1 - pow(1-determinant/centerDet, pR);
            display->setPixel(i, j, color, opacity);
        }
    }

    delete rayleighDisk;
    delete rayleighLimb;
}
