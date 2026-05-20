#include <cmath>
using namespace std;

#include "xpUtil.h"
#include "View.h"

/*
  Creation of the rotation matrix is based on Section 5.7 of Computer
  Graphics: Principles and Practice, by Foley et al.
*/
View::View(const double PRPx, const double PRPy, const double PRPz, 
           const double VRPx, const double VRPy, const double VRPz, 
           const double VUPx, const double VUPy, const double VUPz, 
           const double dpp, const double rotate_angle) : distPerPixel_(dpp)
{
    // P1 = Observer location
    // P2 = Target planet
    // P3 = Up direction

    double P1[3] = { PRPx, PRPy, PRPz };
    double P2[3] = { VRPx, VRPy, VRPz };
    double P3[3] = { VUPx, VUPy, VUPz };

    double P2P1[3];
    for (int i = 0; i < 3; i++)
        P2P1[i] = (P2[i] - P1[i]);

    double P3P1[3];
    for (int i = 0; i < 3; i++)
        P3P1[i] = (P3[i] - P1[i]);

    // define the unit vector along P1P2
    double length = sqrt(dot(P2P1, P2P1));
    double Rz[3];
    for (int i = 0; i < 3; i++) Rz[i] = P2P1[i]/length;

    distToPlane_ = length;

    // define the unit vector perpendicular to the plane P1P2P3.  This
    // is the cross product of P1P2 and P1P3.
    double Rx[3];
    cross(P3P1, P2P1, Rx);
    length = sqrt(dot(Rx, Rx));
    for (int i = 0; i < 3; i++) Rx[i] /= length;

    // define the unit vector perpendicular to the other two
    double Ry[3];
    cross(Rz, Rx, Ry);
    length = sqrt(dot(Ry, Ry));
    for (int i = 0; i < 3; i++) Ry[i] /= length;

    const double cos_rot = cos(rotate_angle);
    const double sin_rot = sin(rotate_angle);

    // set the rotation matrix
    for (int i = 0; i < 3; i++)
    {
        rotate[0][i] =  Rx[i] * cos_rot + Ry[i] * sin_rot;
        rotate[1][i] = -Rx[i] * sin_rot + Ry[i] * cos_rot;
        rotate[2][i] =  Rz[i];
    }

    // set the translation matrix
    translate[0] = -PRPx;
    translate[1] = -PRPy;
    translate[2] = -PRPz;

    invertMatrix(rotate, inv_rotate);
}

View::~View()
{
}

// Rotate the point in the View coordinate system to XYZ
void
View::RotateToXYZ(const double X, const double Y, const double Z, 
                  double &newX, double &newY, double &newZ) const
{
    double P[3] = { X, Y, Z };

    newX = dot(inv_rotate[0], P) - translate[0];
    newY = dot(inv_rotate[1], P) - translate[1];
    newZ = dot(inv_rotate[2], P) - translate[2];
}

// Rotate the point in heliocentric XYZ to the viewing coordinate system.
void
View::RotateToViewCoordinates(const double X, const double Y, const double Z,
                              double &newX, double &newY, double &newZ) const
{
    double P[3] = { X, Y, Z };
    for (int i = 0; i < 3; i++)
        P[i] += translate[i];
    
    newX = dot(rotate[0], P);
    newY = dot(rotate[1], P);
    newZ = dot(rotate[2], P);
}

// Given heliocentric equatorial (X, Y, Z), return (X, Y, Z) such that
// the X and Y coordinates are pixel offsets from the projection
// center.  X increases to the right and Y increases downward.  The Z
// coordinate tells us the distance from the viewing plane, which is
// useful when deciding the order to draw objects; higher Z values get
// drawn first and are covered up by lower Z values.
void
View::XYZToPixel(const double X, const double Y, const double Z, 
                 double &newX, double &newY, double &newZ) const
{
    RotateToViewCoordinates(X, Y, Z, newX, newY, newZ);
    
    newX *= -(distToPlane_/(newZ*distPerPixel_));
    newY *= -(distToPlane_/(newZ*distPerPixel_));
}

// Given pixel position (X, Y), return (X, Y, Z) in view coordinates
void
View::PixelToViewCoordinates(const double X, const double Y,
                             double &newX, double &newY, double &newZ) const
{
    newX = X * distPerPixel_;
    newY = Y * distPerPixel_;
    newZ = distToPlane_;
}
