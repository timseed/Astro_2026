#ifndef EPHEMSPICE_H
#define EPHEMSPICE_H

#include <complex>
#include <string>

#include "Ephemeris.h"

class EphemerisSpice : public Ephemeris
{
public:
    EphemerisSpice();
    ~EphemerisSpice();

    void GetHeliocentricXYZ(const body b, const double tjd, 
                            double &Px, double &Py, double &Pz);
};

#endif
