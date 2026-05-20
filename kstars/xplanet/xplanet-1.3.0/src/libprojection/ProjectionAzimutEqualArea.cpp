/* Contributed from the xplanet forums:
   http://xplanet.sourceforge.net/FUDforum2/index.php?t=msg&S=8ddea843fc10100539f23a5d8d4d6d4a&th=57&goto=1686#msg_1686
*/

#include <cmath>
using namespace std;

#include "ProjectionAzimutEqualArea.h"
#include "xpUtil.h"

ProjectionAzimutEqualArea::ProjectionAzimutEqualArea(const int f, const int w, 
						     const int h) 
    : ProjectionBase(f, w, h)
{
    isWrapAround_ = false;
    radius_ = sqrt(2 * radius_);

    // The rendered globe is contained in a square with sides of length
    // iside and upper left corner at (istart, jstart).
    iside_ = (int) (radius_ * height_);
}

void
ProjectionAzimutEqualArea::rot90degree(double &lon, double &lat)
{
    double	x = cos(lat) * cos(lon);

    double	y = sin(lat);
    double	r2 = sqrt(1 - x * x);

    double	coslon;
    if (fabs(y) < r2)	coslon = y / r2;
    else if (y > 0)	coslon =  1;
    else		coslon = -1;

    lat = asin(x);

    if (sin(lon) < 0)	lon = acos(coslon);
    else		lon = -acos(coslon);
}

bool
ProjectionAzimutEqualArea::pixelToSpherical(const double x, const double y, 
					    double &lon, double &lat)
{
    const double X = 2.0 * (x - centerX_) / iside_;
    const double Y = -2.0 * (y - centerY_) / iside_;

    const double r = sqrt(X*X + Y*Y);
    if (r > 1) return(false);

    lat = M_PI_2 - 2 * asin(r);
    lon = atan2(-X, Y);

    rot90degree(lon, lat);

    if (rotate_) RotateXYZ(lat, lon);

    if (lon > M_PI) lon -= TWO_PI;
    else if (lon < -M_PI) lon += TWO_PI;

    return(true);
}

bool
ProjectionAzimutEqualArea::sphericalToPixel(double lon, double lat,
					    double &x, double &y) const
{
    if (rotate_) RotateZYX(lat, lon);

    rot90degree(lon, lat);

    double r = sin((M_PI_2 - lat) / 2);

    double X = -sin(lon) * r;
    double Y = cos(lon) * r;

    x = centerX_ + X * iside_/2;
    if (x < 0 || x >= width_) return(false);

    y = centerY_ - Y * iside_/2;
    if (y < 0 || y >= height_) return(false);

    return(true);
}
