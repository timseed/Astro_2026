#ifndef LIBMULTIPLE_H
#define LIBMULTIPLE_H

#include <map>

class Annotation;
class DisplayBase;
class Map;
class Planet;
class PlanetProperties;
class Ring;
class View;

extern void
addOrbits(const double jd0, const View *view, 
          const int width, const int height, 
          Planet *p, PlanetProperties *currentProperties, 
          std::multimap<double, Annotation *> &annotationMap);

extern void
drawEllipsoid(const double pX, const double pY, const double pR, 
              const double oX, const double oY, const double oZ, 
              const double X, const double Y, const double Z,
              DisplayBase *display, 
              const View *view, const Map *map, Planet *planet,
              PlanetProperties *planetProperties);

extern void
drawRings(Planet *p, DisplayBase *display, View *view, Ring *ring, 
          const double X, const double Y, const double R, 
          const double obs_lat, const double obs_lon, 
          const bool lit_side, const bool draw_far_side);

extern void
drawSphere(const double pX, const double pY, const double pR, 
           const double oX, const double oY, const double oZ, 
           const double X, const double Y, const double Z,
           DisplayBase *display, 
           const View *view, const Map *map, Planet *planet,
           PlanetProperties *planetProperties);

extern void
drawSunGlare(DisplayBase *display, const double X, const double Y, 
             const double R, const unsigned char *color);

extern void
drawStars(DisplayBase *display, View *view);

#endif
