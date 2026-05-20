#ifndef BUILDPLANETMAP_H
#define BUILDPLANETMAP_H

#include <map>

#include "body.h"

class Planet;

extern Planet *
findPlanetinMap(std::map<double, Planet *> &planetMap, body b);

extern void
buildPlanetMap(const double jd, std::map<double, Planet *> &planetMap);

extern void
buildPlanetMap(const double jd, 
	       const double oX, const double oY, const double oZ, 
	       const bool light_time, std::map<double, Planet *> &planetMap);

extern void
destroyPlanetMap();

#endif
