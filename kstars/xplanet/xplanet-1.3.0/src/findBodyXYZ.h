#ifndef FINDBODYXYZ_H
#define FINDBODYXYZ_H

#include "body.h"

extern void
findBodyXYZ(const double julianDay,
            const body bodyIndex, const int bodyID, 
            double &X, double &Y, double &Z);

extern void
findBodyVelocity(const double julianDay, 
                 const body bodyIndex, const int bodyID, 
                 const body relativeIndex, const int relativeID, 
                 double &vX, double &vY, double &vZ);

#endif
