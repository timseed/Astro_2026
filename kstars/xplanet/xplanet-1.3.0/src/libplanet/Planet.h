#ifndef PLANET_H
#define PLANET_H

#include <string>

#include "body.h"

class Ephemeris;

class Planet
{
 public:
    static body parseBodyName(char *name);

    Planet(const double jd, const body this_body);
    ~Planet();

    void calcHeliocentricEquatorial();
    void calcHeliocentricEquatorial(const bool relativeToSun);

    void PlanetographicToXYZ(double &X, double &Y, double &Z,
                             double lat, double lon, 
                             const double rad);

    void PlanetocentricToXYZ(double &X, double &Y, double &Z,
                             const double lat, const double lon, 
                             const double rad);

    void XYZToPlanetocentric(const double X, const double Y, const double Z,
                             double &lat, double &lon);

    void XYZToPlanetocentric(const double X, const double Y, const double Z,
                             double &lat, double &lon, double &rad);

    void XYZToPlanetographic(const double X, const double Y, const double Z,
                             double &lat, double &lon);

    void XYZToPlanetographic(const double X, const double Y, const double Z,
                             double &lat, double &lon, double &rad);

    void XYZToPlanetaryXYZ(const double X, const double Y, const double Z,
                           double &pX, double &pY, double &pZ);

    void PlanetaryXYZToXYZ(const double pX, const double pY, const double pZ,
                           double &X, double &Y, double &Z);

    void PlanetocentricToPlanetographic(double &lat, double &lon) const;
    void PlanetographicToPlanetocentric(double &lat, double &lon) const;

    void getPosition(double &X, double &Y, double &Z) const;

    void getBodyNorth(double &X, double &Y, double &Z) const;
    void getOrbitalNorth(double &X, double &Y, double &Z) const;

    double Illumination(const double oX, const double oY, const double oZ);

    body Index() const { return(index_); };
    body Primary() const { return(primary_); };

    double Flattening() const { return(flattening_); };
    int Flipped() const { return(flipped_); };
    double Period() const { return(period_); };
    double Radius() const { return(radiusEq_); };
    double Radius(const double lat) const;

    bool IsInMyShadow(const double x, const double y, const double z);

 private:
    body index_;

    Ephemeris *ephem_;
    bool ephemerisHigh_;
    bool ephemerisSpice_;

    double julianDay_;
    double T2000_;            // Julian centuries from 2000
    double d2000_;            // days from 2000

    body primary_;

    double alpha0_;          // right ascension of the north pole
    double delta0_;          // declination of the north pole

    double nullMeridian_;    // orientation of zero longitude
    double nullMeridian0_;   // orientation of zero longitude at time 0
    double wdot_;            // rotation rate

    double rot_[3][3];          // rotation matrix
    double invRot_[3][3];      // inverse of e
    bool needRotationMatrix_; 

    double period_;           // orbital period, in days
    double radiusEq_;         // equatorial radius
    double radiusPol_;        // polar radius
    double flattening_;       // (Re - Rp)/Re
    double omf2_;             // (1 - flattening_)^2

    bool needShadowCoeffs_;   // used to compute shadows by ellipsoids
    double sunX_, sunY_, sunZ_;
    double ellipseCoeffC_;

    /* 
       flipped = 1 if planet's longitude increases to the east
       (e.g. earth), -1 if planet's longitude increases to the west
       (e.g. Mars),
    */
    int flipped_; 

    double X_, Y_, Z_;      // Position vector

    void CreateRotationMatrix();
    void ComputeShadowCoeffs();
};

#endif
