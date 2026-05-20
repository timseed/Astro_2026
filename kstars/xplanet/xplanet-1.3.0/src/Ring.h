#ifndef RING_H
#define RING_H

#include <map>

class Planet;

class Ring
{
 public:
    Ring(const double inner_radius, const double outer_radius, 
         const double *ring_brightness, const int num_bright,
         const double *ring_transparency, const int num_trans,
         const double sunlon, const double sunlat,
         const double shade, 
         std::map<double, Planet *> &planetsFromSunMap,
         Planet *p);

    ~Ring();

    // get the radius of the ring shadowing the specified location on
    // the planet
    double getShadowRadius(double lat, double lon);

    // get the brightness on the lit side
    double getBrightness(const double lon, const double r);

    // get the brightness on the dark side
    double getBrightness(const double lon, const double r, const double t);

    double getTransparency(const double r);

    // set the size of each pixel, used to window average the ring
    // brightness/transparency
    void setDistPerPixel(const double d);

    double getOuterRadius() const { return(r_out); };

 private:
    double r_out;  // outer ring radius, units of planetary radii
    double dr_b;   // resolution of brightness grid
    double dr_t;   // resolution of transparency grid

    int num_t;
    double *radius_t;
    double *transparency;
    double *brightness_dark; // brightness of the dark side
    int window_t;  // each pixel contains this many transparency points

    int num_b;
    double *radius_b;
    double *brightness;
    int window_b;  // each pixel contains this many brightness points

    Planet *planet_;
    std::map<double, Planet *> planetsFromSunMap_;

    double shade_;
    double sunLat_, sunLon_;
    double sunX_, sunY_, sunZ_;

    // get a window average of array
    double getValue(const double *array, const int size, const int window,
                    const double dr, const double r);

    // get a window average of array, accounts for shadowing by the planet
    double getValue(const double *array, const int size, const int window,
                    const double dr, const double r, const double lon);
};

#endif
