/*
  This projection is from Section 5.6.1 of Calabretta and Greisen
  (2002), "Representations of celestial coordinates in FITS",
  Astronomy and Astrophysics 395, 1077-1122.

  The paper is available online at
  http://www.atnf.csiro.au/~mcalabre/WCS
*/

#include <cmath>
using namespace std;

#include "ProjectionTSC.h"
#include "xpUtil.h"

ProjectionTSC::ProjectionTSC(const int f, const int w, const int h)
    : ProjectionBase(f, w, h)
{
    isWrapAround_ = false;

    xScale_ = 1;
    yScale_ = 2./3.;

    trueWidth_ = width_;
    trueHeight_ = static_cast<int> (0.75 * trueWidth_);

    xOffset_ = trueWidth_/8;
    yOffset_ = (trueHeight_ - height_)/2;

    // find the pixel values of the center of each cube face
    bool rotateSave = rotate_;
    rotate_ = false;
    for (int i = 0; i < 6; i++)
    {
        double lat_c, lon_c;
        GetCenterLatLon(i, lat_c, lon_c);
        sphericalToPixel(lon_c, lat_c, xPixel_[i], yPixel_[i]);

        xPixel_[i] += xOffset_;
        yPixel_[i] += yOffset_;
    }
    rotate_ = rotateSave;
}

void 
ProjectionTSC::GetCenterLatLon(const int face, 
                               double &lat_c, double &lon_c) const
{
    switch (face)
    {
    case 0:
        lon_c = 0;
        lat_c = M_PI_2;
        break;
    case 1:
        lon_c = 0;
        lat_c = 0;
        break;
    case 2:
        lon_c = M_PI_2;
        lat_c = 0;
        break;
    case 3:
        lon_c = M_PI;
        lat_c = 0;
        break;
    case 4:
        lon_c = -M_PI_2;
        lat_c = 0;
        break;
    case 5:
        lon_c = 0;
        lat_c = -M_PI_2;
        break;
    default:
        xpExit("Unknown face???", __FILE__, __LINE__);
        break;
    }
}

void 
ProjectionTSC::GetXiEtaZeta(const int face, 
                            const double l, const double m, const double n,
                            double &xi, double &eta, double &zeta) const
{
    switch (face)
    {
    case 0:
        xi   =  m;
        eta  = -l;
        zeta =  n;
        break;
    case 1:
        xi   =  m;
        eta  =  n;
        zeta =  l;
        break;
    case 2:
        xi   = -l;
        eta  =  n;
        zeta =  m;
        break;
    case 3:
        xi   = -m;
        eta  =  n;
        zeta = -l;
        break;
    case 4:
        xi   =  l;
        eta  =  n;
        zeta = -m;
        break;
    case 5:
        xi   =  m;
        eta  =  l;
        zeta = -n;
        break;
    default:
        xpExit("Unknown face???", __FILE__, __LINE__);
        break;
    }
}

void 
ProjectionTSC::GetLMN(const int face, 
                      const double xi, const double eta, const double zeta,
                      double &l, double &m, double &n) const
{
    switch (face)
    {
    case 0:
        m =  xi;
        l = -eta;
        n =  zeta;
        break;
    case 1:
        m = xi;
        n = eta;
        l = zeta;
        break;
    case 2:
        l = -xi;
        n =  eta;
        m =  zeta;
        break;
    case 3:
        m = -xi;
        n =  eta;
        l = -zeta;
        break;
    case 4:
        l =  xi;
        n =  eta;
        m = -zeta;
        break;
    case 5:
        m =  xi;
        l =  eta;
        n = -zeta;
        break;
    default:
        xpExit("Unknown face???", __FILE__, __LINE__);
        break;
    }
}

int
ProjectionTSC::GetFace(const double x, const double y) const
{
    int returnVal = -1;

    double dist = trueWidth_;
    for (int i = 0; i < 6; i++)
    {
        const double dx = x - xPixel_[i];
        const double dy = y - yPixel_[i];
        const double thisDist = sqrt(dx*dx + dy*dy);

        if (thisDist < dist)
        {
            dist = thisDist;
            returnVal = i;
        }
    }

    // check that the point is on the cube face
    if (fabs(x - xPixel_[returnVal]) > trueWidth_/8) return(-1);
    if (fabs(y - yPixel_[returnVal]) > trueHeight_/6) return(-1);

    return(returnVal);
}

bool
ProjectionTSC::pixelToSpherical(const double x, const double y,
                                double &lon, double &lat)
{
    const double offsetX = x + width_/2 - centerX_ + xOffset_;
    const double offsetY = y + height_/2 - centerY_ + yOffset_;

    const double X = TWO_PI * (offsetX/trueWidth_ - 0.5)/xScale_;
    const double Y = M_PI * (0.5 - offsetY/trueHeight_)/yScale_;

    const int face = GetFace(offsetX, offsetY);
    if (face < 0) return(false);

    double lat_c, lon_c;
    GetCenterLatLon(face, lat_c, lon_c);

    const double chi = 4 * (X - lon_c) / M_PI;
    const double psi = 4 * (Y - lat_c) / M_PI;
    const double zeta = 1/sqrt(1 + chi*chi + psi*psi);

    const double xi = chi * zeta;
    const double eta = psi * zeta;

    double l, m, n;
    GetLMN(face, xi, eta, zeta, l, m, n);
    lat = asin(n);
    lon = atan2(m, l);

    if (fabs(lon) > M_PI) return(false);
 
    if (rotate_) RotateXYZ(lat, lon);

    if (lon > M_PI) lon -= TWO_PI;
    else if (lon < -M_PI) lon += TWO_PI;

    return(true);
}

bool
ProjectionTSC::sphericalToPixel(double lon, double lat,
                                double &x, double &y) const
{
    if (rotate_) RotateZYX(lat, lon);

    const double l = cos(lat) * cos(lon);
    const double m = cos(lat) * sin(lon);
    const double n = sin(lat);

    int face = -1;
    if (fabs(l) >= fabs(m) && fabs(l) >= fabs(n))
    {
        face = ((l < 0) ? 3 : 1);
    }
    else if (fabs(m) >= fabs(l) && fabs(m) >= fabs(n))
    {
        face = ((m < 0) ? 4 : 2);
    }
    else if (fabs(n) >= fabs(l) && fabs(n) >= fabs(m))
    {
        face = ((n < 0) ? 5 : 0);
    }

    double lon_c, lat_c;
    GetCenterLatLon(face, lat_c, lon_c);

    double xi, eta, zeta;
    GetXiEtaZeta(face, l, m, n, xi, eta, zeta);

    const double chi = xi/zeta;
    const double psi = eta/zeta;

    const double X = lon_c + chi * M_PI / 4;
    const double Y = lat_c + psi * M_PI / 4;

    x = trueWidth_ * (X * xScale_ / TWO_PI + 0.5);
    y = trueHeight_ * (0.5 - Y * yScale_ / M_PI);

    x += (centerX_ - width_/2 - xOffset_);
    y += (centerY_ - height_/2 - yOffset_);

    if (y < 0 || y >= height_) return(false);

    return(true);
}
