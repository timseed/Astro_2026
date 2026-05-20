#ifndef EPHEM_H
#define EPHEM_H

#include "body.h"

class Ephemeris
{
public:
    Ephemeris();
    virtual ~Ephemeris();

    virtual void GetHeliocentricXYZ(const body b, const double tjd, 
				    double &Px, double &Py, double &Pz) = 0;
};

#endif
