#include <cmath>
using namespace std;

#include "Options.h"
#include "xpUtil.h"

#include "ProjectionBase.h"

ProjectionBase::ProjectionBase(const int flipped, const int w, const int h) 
    : flipped_(flipped), width_(w), height_(h)
{
    Options *options = Options::getInstance();
    init(flipped, w, h, options);
}

ProjectionBase::ProjectionBase(const int flipped, const int w, const int h,
                               const Options* o) 
    : flipped_(flipped), width_(w), height_(h)
{
    init(flipped, w, h, o);
}

void ProjectionBase::init(const int flipped, const int w, const int h,
                          const Options* options) 
{
    centerLat_ = options->Latitude();
    centerLon_ = options->Longitude() * flipped_;
    const double rotAngle = options->Rotate();
    
    centerX_ = options->CenterX();
    centerY_ = options->CenterY();
    radius_ = options->Radius();

    rotate_ = (centerLat_ != 0 || centerLon_ != 0 || rotAngle != 0);

    if (rotate_)
    {
        SetXYZRotationMatrix(rotAngle, centerLat_, -centerLon_);
        
        // set rotation matrix for grid and markers
        SetZYXRotationMatrix(-rotAngle, -centerLat_, centerLon_);
    }

    // limb darkening_, gets overridden in ORTHOGRAPHIC mode
    darkening_ = 1.0;
}

ProjectionBase::~ProjectionBase()
{
}

/*
  matrix to first rotate reference frame angle_x radians through x
  axis, then angle_y radians through y axis, and lastly angle_z
  radians through z axis.  Positive rotations are counter-clockwise
  looking down the axis.
*/
void
ProjectionBase::SetXYZRotationMatrix(const double angle_x, 
                                     const double angle_y, 
                                     const double angle_z)
{
    if (angle_x == 0 && angle_y == 0 && angle_z == 0) 
    {
        for (int j = 0; j < 3; j++)
            for (int i = 0; i < 3; i++)
                rotXYZ_[j][i] = (i == j ? 1 : 0 );
        return;
    }

    const double cosx = cos(angle_x);
    const double cosy = cos(angle_y);
    const double cosz = cos(angle_z);
    const double sinx = sin(angle_x);
    const double siny = sin(angle_y);
    const double sinz = sin(angle_z);

    rotXYZ_[0][0] =  cosy * cosz;
    rotXYZ_[0][1] =  sinx * siny * cosz + cosx * sinz;
    rotXYZ_[0][2] = -cosx * siny * cosz + sinx * sinz;
    rotXYZ_[1][0] = -cosy * sinz;
    rotXYZ_[1][1] = -sinx * siny * sinz + cosx * cosz;
    rotXYZ_[1][2] =  cosx * siny * sinz + sinx * cosz;
    rotXYZ_[2][0] =  siny;
    rotXYZ_[2][1] = -sinx * cosy;
    rotXYZ_[2][2] =  cosx * cosy;
}

/*
  matrix to first rotate reference frame angle_z radians through z
  axis, then angle_y radians through y axis, and lastly angle_x
  radians through x axis.  Positive rotations are counter- clockwise
  looking down the axis.
*/
void
ProjectionBase::SetZYXRotationMatrix(const double angle_x, 
                                     const double angle_y,
                                     const double angle_z)
{
    if (angle_x == 0 && angle_y == 0 && angle_z == 0) 
    {
        for (int j = 0; j < 3; j++)
            for (int i = 0; i < 3; i++)
                rotZYX_[j][i] = (i == j ? 1 : 0 );
        return;
    }

    const double cosx = cos(angle_x);
    const double cosy = cos(angle_y);
    const double cosz = cos(angle_z);
    const double sinx = sin(angle_x);
    const double siny = sin(angle_y);
    const double sinz = sin(angle_z);

    rotZYX_[0][0] =  cosy * cosz;
    rotZYX_[0][1] =  cosy * sinz;
    rotZYX_[0][2] = -siny;
    rotZYX_[1][0] = -cosx * sinz + sinx * siny * cosz;
    rotZYX_[1][1] =  sinx * siny * sinz + cosx * cosz;
    rotZYX_[1][2] =  sinx * cosy;
    rotZYX_[2][0] =  cosx * siny * cosz + sinx * sinz;
    rotZYX_[2][1] = -sinx * cosz + cosx * siny * sinz;
    rotZYX_[2][2] =  cosx * cosy;
}

void
ProjectionBase::RotateXYZ(double &lat, double &lon) const
{
    const double X0 = cos(lat) * cos(lon);
    const double Y0 = cos(lat) * sin(lon);
    const double Z0 = sin(lat);

    const double X1 = (rotXYZ_[0][0] * X0 
                       + rotXYZ_[0][1] * Y0 
                       + rotXYZ_[0][2] * Z0);
    const double Y1 = (rotXYZ_[1][0] * X0 
                       + rotXYZ_[1][1] * Y0
                       + rotXYZ_[1][2] * Z0);
    const double Z1 = (rotXYZ_[2][0] * X0
                       + rotXYZ_[2][1] * Y0
                       + rotXYZ_[2][2] * Z0);

    lat = asin(Z1);
    lon = atan2(Y1, X1);
}

void
ProjectionBase::RotateZYX(double &lat, double &lon) const
{
    const double X0 = cos(lat) * cos(lon);
    const double Y0 = cos(lat) * sin(lon);
    const double Z0 = sin(lat);

    const double X1 = (rotZYX_[0][0] * X0 
                       + rotZYX_[0][1] * Y0 
                       + rotZYX_[0][2] * Z0);
    const double Y1 = (rotZYX_[1][0] * X0 
                       + rotZYX_[1][1] * Y0 
                       + rotZYX_[1][2] * Z0);
    const double Z1 = (rotZYX_[2][0] * X0 
                       + rotZYX_[2][1] * Y0 
                       + rotZYX_[2][2] * Z0);

    lat = asin(Z1);
    lon = atan2(Y1, X1);
}

void
ProjectionBase::buildPhotoTable()
{
    tableSize_ = 10;
    cosAngle_ = new double[tableSize_];
    photoFunction_ = new double[tableSize_];

    for (int i = 0; i < tableSize_; i++)
    {
        cosAngle_[i] = i / (tableSize_ - 1.);
        photoFunction_[i] = photoFunction(cosAngle_[i]);
    }
}

void
ProjectionBase::destroyPhotoTable()
{
    delete [] cosAngle_;
    delete [] photoFunction_;
}

double
ProjectionBase::getPhotoFunction(const double x) const
{
    if (x < 0) return(0);

    for (int i = 0; i < tableSize_; i++)
    {
        if (cosAngle_[i] > x) 
        {
            double frac = ((x - cosAngle_[i-1])
                           /(cosAngle_[i] - cosAngle_[i-1]));
            double returnval = (photoFunction_[i-1] 
                                + frac * (photoFunction_[i] 
                                          - photoFunction_[i-1]));
            return(returnval);
        }
    }

    return(1);
}

void
ProjectionBase::setRange(const double range)
{
}
