#include <cstdlib>
#include <cmath>
#include <map>
using namespace std;

#include "Ring.h"
#include "xpUtil.h"

#include "libplanet/Planet.h"

Ring::Ring(const double inner_radius, const double outer_radius, 
           const double *ring_brightness, const int num_bright,
           const double *ring_transparency, const int num_trans,
           const double sunlon, const double sunlat,
           const double shade, 
           map<double, Planet *> &planetsFromSunMap,
           Planet *p) : planet_(p), 
                        shade_(shade), 
                        sunLat_(sunlat),
                        sunLon_(sunlon)
{
    r_out = outer_radius * 1.01; // fudge to agree better with Cassini
    dr_b = (outer_radius - inner_radius) / num_bright;
    dr_t = (outer_radius - inner_radius) / num_trans;

    const int innerPadding = 100;
    const int outerPadding = 20;
    num_b = outerPadding + num_bright + innerPadding;
    num_t = outerPadding + num_trans + innerPadding;

    // brightness and transparency arrays are from the outside in
    radius_b = new double[num_b];
    for (int i = 0; i < num_b; i++) 
        radius_b[i] = r_out - i * dr_b;

    brightness = new double[num_b];
    // drop the brightness to 0 at the outer radius
    for (int i = 0; i < outerPadding; i++)
    {
        double weight = ((double) i) / (outerPadding - 1);
        brightness[i] = weight * ring_brightness[0];
    }

    for (int i = 0; i < num_bright; i++) 
        brightness[i + outerPadding] = ring_brightness[i];

    for (int i = 0; i < innerPadding; i++)
        brightness[outerPadding + num_bright + i] = ring_brightness[num_bright-1];

    radius_t = new double[num_t];
    for (int i = 0; i < num_t; i++) 
        radius_t[i] = r_out - i * dr_t;

    transparency = new double[num_t];
    // bring the transparency up to 1 at the outer radius
    for (int i = 0; i < outerPadding; i++)
    {
        double weight = ((double) i) / (outerPadding - 1);
        transparency[i] = (1 - (1 - ring_transparency[0]) * weight);
    }

    for (int i = 0; i < num_trans; i++) 
        transparency[i + outerPadding] = ring_transparency[i];

    // bring the transparency up to 1 at the inner radius
    for (int i = 0; i < innerPadding; i++)
    {
        double weight = 1 - ((double) i) / (innerPadding - 1);
        transparency[outerPadding + num_trans + i] = (1 - (1-ring_transparency[num_trans-1]) 
                                                    * weight);
    }

    // make the rings darker as we get closer to equinox
    const double sin_sun_lat_025 = pow(abs(sin(sunLat_)), 0.25);
    for (int i = 0; i < num_b; i++)
        brightness[i] *= sin_sun_lat_025;

    brightness_dark = new double[num_t];
    for (int i = 0; i < num_t; i++) brightness_dark[i] = transparency[i] * sin_sun_lat_025;

    planet_->XYZToPlanetaryXYZ(0, 0, 0, sunX_, sunY_, sunZ_);

    planetsFromSunMap_.clear();
    planetsFromSunMap_.insert(planetsFromSunMap.begin(), planetsFromSunMap.end());
}

Ring::~Ring()
{
    delete [] radius_b;
    delete [] brightness;

    delete [] radius_t;
    delete [] transparency;
}

/*
  Given a subsolar point and a location on the planet surface, check
  if the surface location can be in shadow by the rings, and if so,
  return the ring radius.
*/
double
Ring::getShadowRadius(double lat, double lon)
{
    // If this point is on the same side of the rings as the sun,
    // there's no shadow.
    if(sunLat_ * lat >= 0) return(-1);

    const double rad = planet_->Radius(lat);
    planet_->PlanetographicToPlanetocentric(lat, lon);

    const double x = rad * cos(lat) * cos(lon);
    const double y = rad * cos(lat) * sin(lon);
    const double z = rad * sin(lat);

    const double dist = z/sunZ_;

    const double dx = x - sunX_ * dist;
    const double dy = y - sunY_ * dist;
    
    return(sqrt(dx * dx + dy * dy));
}

double 
Ring::getBrightness(const double lon, const double r)
{
    return(getValue(brightness, num_b, window_b, dr_b, r, lon));
}

double 
Ring::getBrightness(const double lon, const double r, const double t)
{
    double returnval;
    if (t == 1.0) 
    {
        returnval = 0;
    }
    else 
    {
        returnval = getValue(brightness_dark, num_t, window_t, dr_t, r, lon);
    }
    return(returnval);
}

double
Ring::getTransparency(const double r)
{
    return(getValue(transparency, num_t, window_t, dr_t, r));
}

double
Ring::getValue(const double *array, const int size, const int window,
               const double dr, const double r)
{
    int i = static_cast<int> ((r_out - r)/dr);

    if (i < 0 || i >= size) return(-1.0);

    int j1 = i - window;
    int j2 = i + window;
    if (j1 < 0) j1 = 0;
    if (j2 >= size) j2 = size - 1;

    double sum = 0;
    for (int j = j1; j < j2; j++) sum += array[j];
    sum /= (j2 - j1);

    return(sum);
}

double
Ring::getValue(const double *array, const int size, const int window,
               const double dr, const double r, const double lon)
{
    int i = static_cast<int> ((r_out - r)/dr);

    if (i < 0 || i >= size) return(-1.0);

    int j1 = i - window;
    int j2 = i + window;
    if (j1 < 0) j1 = 0;
    if (j2 >= size) j2 = size - 1;

    bool shaded[j2-j1];
    for (int j = 0; j < j2-j1; j++) shaded[j] = false;

    const double cosLon = cos(lon);
    const double sinLon = sin(lon);
    
    // planet's distance from the sun
    double cX, cY, cZ;
    planet_->getPosition(cX, cY, cZ);
    double p_dist = sqrt(cX*cX + cY*cY + cZ*cZ);

    for (map<double, Planet *>::iterator it0 = planetsFromSunMap_.begin(); 
         it0 != planetsFromSunMap_.end(); it0++)
    {
        Planet *p = it0->second;
        if (p->Index() == planet_->Index())
        {
            // Only check behind the planet for its own shadow
            if (cos(lon-sunLon_) > -0.45) break;
        }
        else if (p->Primary() == planet_->Index())
        {
            // Check that the planet-sun-satellite angle is
            // smaller than the angular size of the rings seen
            // from the sun

            double pX, pY, pZ;
            p->getPosition(pX, pY, pZ);
            
            double sep = abs(acos(ndot(pX, pY, pZ, cX, cY, cZ)));
            
            if (sep > r_out * planet_->Radius() / p_dist) continue;
        }
        else
        {
            // Don't worry about this body shadowing the rings
            continue;
        }

        double r0 = r;
        for (int j = j1; j < j2; j++) 
        {
            if (shaded[j-j1]) continue;

            const double rX =  r0 * cosLon;
            const double rY = -r0 * sinLon;
            const double rZ =  0;
            
            double X, Y, Z;
            planet_->PlanetaryXYZToXYZ(rX, rY, rZ, X, Y, Z);
    
            // convert this point on the rings to this planet's
            // XYZ coordinate system
            double thisX, thisY, thisZ;
            p->XYZToPlanetaryXYZ(X, Y, Z, thisX, thisY, thisZ);
            
            if (p->IsInMyShadow(thisX, thisY, thisZ)) shaded[j-j1] = true;

            r0 += dr;
        }
        if (p->Index() == planet_->Index()) break;
    }

    double sum = 0;
    for (int j = j1; j < j2; j++)
    {
        if (shaded[j-j1])
            sum += (shade_ * array[j]);
        else
            sum += array[j];
    }
    sum /= (j2 - j1);

    return(sum);
}

// units for dist_per_pixel is planetary radii
void
Ring::setDistPerPixel(const double dist_per_pixel)
{
    window_b = static_cast<int> (dist_per_pixel / dr_b + 0.5);
    window_t = static_cast<int> (dist_per_pixel / dr_t + 0.5);

    window_b = window_b/2 + 1;
    window_t = window_t/2 + 1;
}

