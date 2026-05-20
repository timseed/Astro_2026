#ifndef SPHERICAL_TO_PIXEL_H
#define SPHERICAL_TO_PIXEL_H

class Planet;
class PlanetProperties;
class ProjectionBase;
class View;

extern bool
sphericalToPixel(const double lat, const double lon, const double rad, 
		 double &X, double &Y, double &Z, Planet *planet, 
		 View *view, ProjectionBase *projection);

#endif
