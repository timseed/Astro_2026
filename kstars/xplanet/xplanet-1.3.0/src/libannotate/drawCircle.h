#ifndef DRAW_CIRCLE_H
#define DRAW_CIRCLE_H

#include <map>

class Annotation;
class Planet;
class ProjectionBase;
class View;

extern void
drawCircle(const double lat, const double lon, const double rad,
           const unsigned char color[3], const int thickness, 
           const double spacing, const double magnify,
           Planet *planet, View *view, ProjectionBase *projection,
           multimap<double, Annotation *> &annotationMap);

#endif
