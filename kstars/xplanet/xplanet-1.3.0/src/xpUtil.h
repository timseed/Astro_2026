#ifndef XPUTIL_H
#define XPUTIL_H

#include <cmath>
#include <ctime>
#include <string>

#ifndef M_PI
#define M_PI           3.14159265358979323846  /* pi */
#endif

#ifndef M_PI_2
#define M_PI_2         1.57079632679489661923  /* pi/2 */
#endif

const double deg_to_rad = M_PI/180.;
const double TWO_PI = 2 * M_PI;
const double AU_to_km = 149597870.66;
const double FAR_DISTANCE = 1e6;

const int MAX_LINE_LENGTH = 512;

extern void cross(const double a[3], const double b[3], double c[3]);

extern double dot(const double A0, const double A1, const double A2, 
                  const double B0, const double B1, const double B2);

extern double ndot(const double A0, const double A1, const double A2, 
                   const double B0, const double B1, const double B2);

extern double dot(const double a[3], const double b[3]);
extern double ndot(const double a[3], const double b[3]);

extern double normalize(double a[3]);

extern void invertMatrix(const double in[3][3], double out[3][3]);

extern time_t get_tv_sec(double jd);

extern void RADecToXYZ(double RA, double Dec, double &X, double &Y, 
                       double &Z);

extern void fromJulian(double jd, int &year, int &month, int &day, 
                       int &hour, int &min, double &sec);
extern std::string fromJulian(double date);

extern double toJulian(int year, int month, int day, 
                       int hour, int min, int sec);

extern double delT(const double jd);

extern void rotateX(double &X, double &Y, double &Z, const double theta);

extern void rotateZ(double &X, double &Y, double &Z, const double theta);

extern void removeFromEnvironment(const char *name);

extern void unlinkFile(const char *name);

extern void getWeights(const double t, const double u, double weights[4]);

extern void calcGreatArc(const double lat1, const double lon1,
                         const double lat2, const double lon2,
                         double &trueCourse, double &dist);

extern double kepler(const double e, double M);

extern void precessB1950J2000(double &X, double &Y, double &Z);

extern double photoFunction(const double x);

extern void xpExit(const std::string &message, const char *file, 
                   const int line);

extern void xpWarn(const std::string &message, const char *file, 
                   const int line);

extern void xpMsg(const std::string &message, const char *file, 
                  const int line);

extern void strftimeUTF8(std::string &timeString);

extern char * checkLocale(const int category, const char *locale);

#endif
