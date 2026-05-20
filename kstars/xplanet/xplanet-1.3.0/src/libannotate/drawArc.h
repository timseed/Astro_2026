#ifndef DRAW_ARC_H
#define DRAW_ARC_H

#include <map>

class Annotation;
class Planet;
class ProjectionBase;
class View;

extern void
drawArc(const double lat1, const double lon1, const double rad1,
        const double lat2, const double lon2, const double rad2,
        const unsigned char color[3], const int thickness, 
        const double spacing, const double magnify,
        Planet *planet, View *view, ProjectionBase *projection,
        std::multimap<double, Annotation *> &annotationMap);

#endif
