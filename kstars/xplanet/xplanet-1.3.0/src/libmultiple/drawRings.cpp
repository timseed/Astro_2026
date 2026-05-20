#include <cmath>
#include <cstdio>
using namespace std;

#include "Options.h"
#include "Ring.h"
#include "View.h"
#include "xpUtil.h"

#include "libplanet/Planet.h"
#include "libdisplay/libdisplay.h"

/*
  This routine gets the coefficients for the equation of the
  equatorial plane, which is in the form
  
  Ax + By + Cz + D = 0

  The equation of a plane containing the 3 points P1, P2, P3 is

  | x  - x1   y  - y1   z  - z1 |
  | x2 - x1   y2 - y1   z2 - z1 | = 0
  | x3 - x1   y3 - y1   z3 - z1 |

  Solving for A, B, C, and D, we get

  A = [(y2 - y1)(z3 - z1) - (y3 - y1)(z2 - z1)]
  B = [(x3 - x1)(z2 - z1) - (x2 - x1)(z3 - z1)]
  C = [(x2 - x1)(y3 - y1) - (x3 - x1)(y2 - y1)]
  D = -[x1 * A + y1 * B + z1 * C] 
*/
static void 
getEquatorialPlane(Planet *p, View *view, double &A, double &B, 
                   double &C, double &D)
{
    double x1, x2, x3;
    double y1, y2, y3;
    double z1, z2, z3;

    p->PlanetocentricToXYZ(x1, y1, z1, 0, 0, 1);
    p->PlanetocentricToXYZ(x2, y2, z2, 0, 120 * deg_to_rad, 1);
    p->PlanetocentricToXYZ(x3, y3, z3, 0, -120 * deg_to_rad, 1);

    view->RotateToViewCoordinates(x1, y1, z1, x1, y1, z1);
    view->RotateToViewCoordinates(x2, y2, z2, x2, y2, z2);
    view->RotateToViewCoordinates(x3, y3, z3, x3, y3, z3);

    A = ((y2 - y1) * (z3 - z1) - (y3 - y1) * (z2 - z1));
    B = ((x3 - x1) * (z2 - z1) - (x2 - x1) * (z3 - z1));
    C = ((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1));
    D = -(x1 * A + y1 * B + z1 * C);
}

void
drawRings(Planet *p, DisplayBase *display, View *view, Ring *ring, 
          const double X, const double Y, const double R, 
          const double obs_lat, const double obs_lon, 
          const bool lit_side, const bool draw_far_side)
{
    double A, B, C, D;
    getEquatorialPlane(p, view, A, B, C, D);

    double pX, pY, pZ;
    p->getPosition(pX, pY, pZ);
    view->RotateToViewCoordinates(pX, pY, pZ, pX, pY, pZ);
    const double dist_to_planet = sqrt(pX * pX + pY * pY + pZ * pZ);
    
    Options *options = Options::getInstance();

    const int height = display->Height();
    const int width = display->Width();

    const double outer_radius = ring->getOuterRadius() * R;

    int j0 = (int) (floor(Y - outer_radius));
    int j1 = (int) (ceil(Y + outer_radius + 1));
    int i0 = (int) (floor(X - outer_radius));
    int i1 = (int) (ceil(X + outer_radius + 1));

    if (j0 < 0) j0 = 0;
    if (j1 > height) j1 = height;
    if (i0 < 0) i0 = 0;
    if (i1 > width) i1 = width;

    unsigned char ring_color[3] = {255, 224, 209};
    unsigned char pixel[3];

    double dist_per_pixel = 1/R;
    double min_dist_per_pixel = dist_per_pixel;
    dist_per_pixel /= fabs(sin(obs_lat));

    j0 = 0;
    j1 = height;
    i0 = 0;
    i1 = width;

    for (int j = j0; j < j1; j++)
    {
        for (int i = i0; i < i1; i++)
        {
            view->PixelToViewCoordinates(options->CenterX() - i, 
                                         options->CenterY() - j, 
                                         pX, pY, pZ);

            // Find the intersection of the line from the observer to
            // the point on the view plane passing through the ring
            // plane
            const double u = -D / (A * pX + B * pY + C * pZ);

            // if the intersection point is behind the observer, don't
            // plot it
            if (u < 0) continue;

            // The view coordinates of the point in the ring plane
            double rX, rY, rZ;
            rX = u * pX;
            rY = u * pY;
            rZ = u * pZ;

            const double dist_to_point = sqrt(rX * rX + rY * rY + rZ * rZ);
            if ((draw_far_side && dist_to_point <= dist_to_planet) 
                || (!draw_far_side && dist_to_point > dist_to_planet)) 
                continue;
            
            // convert to heliocentric XYZ
            view->RotateToXYZ(rX, rY, rZ, rX, rY, rZ);

            // find lat & lon of ring pixel
            double lat, lon = options->Longitude();
            double dist;
            p->XYZToPlanetographic(rX, rY, rZ, lat, lon, dist);

            double dpp = dist_per_pixel * fabs(cos(obs_lon - lon));
            dpp *= dist_to_point/dist_to_planet;
            if (dpp < min_dist_per_pixel) dpp = min_dist_per_pixel;
            ring->setDistPerPixel(dpp);

            double t = ring->getTransparency(dist);
            
            if (t < 0) continue;

            double b;
            if (lit_side)
                b = ring->getBrightness(lon, dist);
            else
                b = ring->getBrightness(lon, dist, t);

            if (b < 0) continue;

            for (int k = 0; k < 3; k++) 
                pixel[k] = (unsigned char) (b * ring_color[k]);

            display->setPixel(i, j, pixel, 1 - t);
        }
    }
}
