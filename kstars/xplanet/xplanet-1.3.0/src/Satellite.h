#ifndef SATELLITE_H
#define SATELLITE_H

namespace sgp4sdp4
{
#include "libsgp4sdp4/sgp4sdp4.h"
}

class Satellite
{
 public:
    Satellite(char tle_line[3][80]);

    ~Satellite();
    bool isGoodData() const;
    int getID() const;
    const char * getName() const;
    void getSpherical(const time_t tv_sec, double &lat, double &lon,
                      double &alt);
    void loadTLE();

    void printTLE() const;

    bool operator == (const Satellite &sat) const;
 
 private:
    bool good;     // if TLE is in the right format
    char tle_entry[3][80];
     
    sgp4sdp4::tle_t tle;
};

#endif
