#ifndef EPHEMHIGH_H
#define EPHEMHIGH_H

#include <string>

#include "Ephemeris.h"

class EphemerisHigh : public Ephemeris
{
public:
    EphemerisHigh(const std::string &ephemerisFile);
    ~EphemerisHigh();

    void GetHeliocentricXYZ(const body b, const double tjd, 
                            double &Px, double &Py, double &Pz);
 private:
    void *ephem_;
};

#endif
