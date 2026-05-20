#ifndef EPHEMLOW_H
#define EPHEMLOW_H

#include <complex>
#include <string>

#include "Ephemeris.h"

class EphemerisLow : public Ephemeris
{
public:
    EphemerisLow();
    ~EphemerisLow();

    void GetHeliocentricXYZ(const body b, const double tjd, 
                            double &Px, double &Py, double &Pz);
 private:
    void kepler(const double al, const std::complex<double> &z, 
                const double u, std::complex<double> &zto, double &r);

    void ellipx(const double a, const double dlm, const double e, 
                const double p, const double dia, const double omega, 
                const double dmas, 
                double &Px, double &Py, double &Pz,
                double &Vx, double &Vy, double &Vz);

    void calcHeliocentricXYZ(const double tjd, const double dmas,
                             double *a, double *dlm, double *e, 
                             double *pi, double *dinc, double *omega,
                             int *kp, double *ca, double *sa, 
                             int *kq, double *cl, double *sl,
                             double &Px, double &Py, double &Pz,
                             double &Vx, double &Vy, double &Vz);
};

#endif
