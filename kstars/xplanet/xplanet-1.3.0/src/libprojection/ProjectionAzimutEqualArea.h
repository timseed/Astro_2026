/* Contributed from the xplanet forums:
   http://xplanet.sourceforge.net/FUDforum2/index.php?t=msg&S=8ddea843fc10100539f23a5d8d4d6d4a&th=57&goto=1686#msg_1686
*/

#ifndef PROJECTIONAZIMUTEQUALAREA_H
#define PROJECTIONAZIMUTEQUALAREA_H

#include "ProjectionBase.h"

class ProjectionAzimutEqualArea : public ProjectionBase
{
 public:
    ProjectionAzimutEqualArea(const int f, const int w, const int h);
    bool pixelToSpherical(const double x, const double y, 
			  double &lon, double &lat);

    bool sphericalToPixel(double lon, double lat, double &x, double &y) const;

 private:
    int iside_;
    static void rot90degree(double &lon, double &lat);
};

#endif
