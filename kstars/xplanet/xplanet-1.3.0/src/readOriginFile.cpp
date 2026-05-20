#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
using namespace std;

#include "findFile.h"
#include "readOriginFile.h"
#include "xpUtil.h"

static double
interpolateCircular(double x0, double x1, 
                    const double xMax, const double interpFactor)
{
    const double larger = (x0 > x1 ? x0 : x1);
    const double smaller = (x0 < x1 ? x0 : x1);

    if ((larger - smaller) > (smaller - larger + xMax))
    {
        if (x0 < x1)
            x0 += xMax;
        else
            x1 += xMax;
    }

    double returnVal = interpFactor * (x1 - x0) + x0;
    if (returnVal > xMax)
        returnVal -= xMax;

    return(returnVal);
}

void
readOriginFile(string filename, vector<LBRPoint> &originVector)
{
    if (!findFile(filename, "origin"))
    {
        ostringstream errStr;
        errStr << "Can't open origin file " << filename << "\n";
        xpExit(errStr.str(), __FILE__, __LINE__);
    }

    checkLocale(LC_NUMERIC, "C");

    ifstream infile(filename.c_str());
    char line[256];
    while(infile.getline(line, 256))
    {
        if (line[0] == '#') continue;

        long int yyyymmdd, hhmmss;
        double r, lat, lon, localTime;

        int numRead = sscanf(line, "%ld.%ld %lf %lf %lf %lf", 
                             &yyyymmdd, &hhmmss,
                             &r, &lat, &lon, &localTime);
        int yyyymm = yyyymmdd / 100;
        int year = yyyymm/100;
        int month = abs(yyyymm - year * 100);
        int day = abs((int) yyyymmdd - yyyymm * 100);
        
        int hhmm = hhmmss / 100;
        int hour = hhmm / 100;
        int min = hhmm - hour * 100;
        int sec = hhmmss - hhmm * 100;
        
        const double julian_day = toJulian(year, month, day, hour, min, sec);

        // If only the date has been specified, set range to 0
        if (numRead < 3) r = 0;

        // If localTime isn't specified, set it to -1
        if (numRead < 6) localTime = -1;

        lat *= deg_to_rad;
        lon *= deg_to_rad;

        LBRPoint p = { julian_day, r, lat, lon, localTime };

        originVector.push_back(p);
    }
    checkLocale(LC_NUMERIC, "");
    infile.close();
}

void
readDynamicOrigin(string filename, LBRPoint &originPoint)
{
    if (!findFile(filename, "origin"))
    {
        ostringstream errStr;
        errStr << "Can't open origin file " << filename << "\n";
        xpExit(errStr.str(), __FILE__, __LINE__);
    }

    checkLocale(LC_NUMERIC, "C");
    ifstream infile(filename.c_str());
    char line[256];
    while(infile.getline(line, 256))
    {
        if (line[0] == '#') continue;

        long int yyyymmdd, hhmmss;  // not used
        double r, lat, lon, localTime = -1;
        sscanf(line, "%ld.%ld %lf %lf %lf %lf", &yyyymmdd, &hhmmss,
               &r, &lat, &lon, &localTime);
        lat *= deg_to_rad;
        lon *= deg_to_rad;

        originPoint.time = 0;
        originPoint.radius = r;
        originPoint.latitude = lat;
        originPoint.longitude = lon;
        originPoint.localTime = localTime;
    }
    checkLocale(LC_NUMERIC, "");
    infile.close();
}

void
interpolateOriginFile(const double julianDay, 
                      const vector<LBRPoint> &originVector, 
                      double &rad, 
                      double &lat, double &lon, 
                      double &localTime)
{
    const int numPoints = originVector.size();

    if (julianDay < originVector[0].time)
    {
        rad = originVector[0].radius;
        lat = originVector[0].latitude;
        lon = originVector[0].longitude;
        localTime = originVector[0].localTime;
    }
    else if (julianDay > originVector[numPoints-1].time)
    {
        rad = originVector[numPoints-1].radius;
        lat = originVector[numPoints-1].latitude;
        lon = originVector[numPoints-1].longitude;
        localTime = originVector[numPoints-1].localTime;
    }
    else
    {
        double interpFactor = 0;
        for (int i = 1; i < numPoints; i++)
        {
            if (julianDay < originVector[i].time)
            {
                const double jd0 = originVector[i-1].time;
                const double jd1 = originVector[i].time;

                interpFactor = (julianDay - jd0)/(jd1 - jd0);

                rad = (interpFactor 
                       * (originVector[i].radius - originVector[i-1].radius)
                       + originVector[i-1].radius);
                lat = (interpFactor 
                       * (originVector[i].latitude 
                          - originVector[i-1].latitude)
                       + originVector[i-1].latitude);
                lon = interpolateCircular(originVector[i-1].longitude,
                                          originVector[i].longitude,
                                          TWO_PI,
                                          interpFactor);
                localTime = interpolateCircular(originVector[i-1].localTime,
                                                originVector[i].localTime,
                                                24,
                                                interpFactor);

                break;
            }
        }
    }
}
