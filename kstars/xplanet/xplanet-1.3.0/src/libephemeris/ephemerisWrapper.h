#ifndef EPHEMERISWRAPPER
#define EPHEMERISWRAPPER

extern void setUpEphemeris();

extern void cleanUpEphemeris();

extern void GetHeliocentricXYZ(const body index, 
                               const body primary, 
                               const double julianDay, 
                               const bool relativeToSun, 
                               double &X, double &Y, 
                               double &Z);

#endif
