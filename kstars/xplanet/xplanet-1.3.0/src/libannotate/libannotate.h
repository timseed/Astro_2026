#ifndef LIBANNOTATE_H
#define LIBANNOTATE_H

#include "config.h"

#include "Icon.h"
#include "LineSegment.h"
#include "Symbol.h"
#include "Text.h"

#include <map>

class Annotation;
class Planet;
class PlanetProperties;
class ProjectionBase;
class View;

extern void
addArcs(View *view, multimap<double, Annotation *> &annotationMap);

extern void
addArcs(PlanetProperties *planetProperties, Planet *planet, 
        View *view, ProjectionBase *projection, 
        multimap<double, Annotation *> &annotationMap);

extern void
addMarkers(View *view, const int width, const int height, 
           map<double, Planet *> &planetsFromSunMap, 
           multimap<double, Annotation *> &annotationMap);

extern void
addMarkers(PlanetProperties *planetProperties, Planet *planet,
           const double pR, 
           const double X, const double Y, const double Z,
           View *view, ProjectionBase *projection, 
           const int width, const int height, 
           map<double, Planet *> &planetsFromSunMap, 
           std::multimap<double, Annotation *> &annotationMap);

extern bool
calculateSatellitePosition(time_t tv_sec, const int id,
                           double &lat, double &lon, double &rad);

extern void
addSatellites(PlanetProperties *planetProperties, Planet *planet, 
              View *view, ProjectionBase *projection, 
              std::multimap<double, Annotation *> &annotationMap);

extern void
loadSatelliteVector(PlanetProperties *planetProperties);

#ifdef HAVE_CSPICE

extern void 
addSpiceObjects(map<double, Planet *> &planetsFromSunMap,
                View *view, ProjectionBase *projection,
                multimap<double, Annotation *> &annotationMap);

extern void
processSpiceKernels(const bool load);

extern bool
calculateSpicePosition(double jd, 
                       const int naifInt, Planet *relative,
                       const int relativeInt,
                       double &X, double &Y, double &Z);

#endif
#endif
