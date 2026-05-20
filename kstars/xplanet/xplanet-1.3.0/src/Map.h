#ifndef MAP_H
#define MAP_H

#include <map>

class Planet;
class PlanetProperties;
class Ring;

class Map
{
 public:
    Map(const int w, const int h);

    // Use this constructor if there are no image maps
    Map(const int w, const int h, 
        const double sunLat, const double sunLon, 
        Planet *t, PlanetProperties *tp, Ring *r, 
        std::map<double, Planet *> &planetsFromSunMap);

    Map(const int w, const int h, 
        const double sunLat, const double sunLon, 
        const double obsLat, const double obsLon,
        const unsigned char *day, const unsigned char *night, 
        const unsigned char *bump, 
        const unsigned char *specular, const unsigned char *clouds, 
        Planet *t, PlanetProperties *tp, Ring *r, 
        std::map<double, Planet *> &planetsFromSunMap);

    ~Map();

    void Reduce(const int factor);
    void GetPixel(const double lat, double lon, unsigned char pixel[3]) const;
    double Width() const { return(width_); };
    double Height() const { return(height_); };

    double StartLat() const { return(startLat_); };
    double StartLon() const { return(startLon_); };

    double MapHeight() const { return(mapHeight_); };
    double MapWidth() const { return(mapWidth_); };

    bool Write(const char *filename) const; 

 private:
    int width_, height_, area_;
    bool mapbounds_;

    unsigned char color_[3];

    unsigned char *mapData_;
    unsigned char *dayData_;
    unsigned char *nightData_;

    double *latArray_;
    double *lonArray_;

    double *cosLatArray_;
    double *cosLonArray_;
    double *sinLatArray_;
    double *sinLonArray_;

    double delLon_, delLat_;
    double mapHeight_, mapWidth_;
    double startLon_, startLat_;

    Planet *target_;
    PlanetProperties *targetProperties_;
    Ring *ring_;

    const double sunLat_;
    const double sunLon_;

    void SetUpMap();

    void AddBumpMap(const unsigned char *bump);
    void AddSpecularReflection(const unsigned char *specular,
                               const double obsLat, const double obsLon);
    void OverlayClouds(const unsigned char *clouds);
    void AddShadows(std::map<double, Planet *> &planetsFromSunMap);
    void AddShadow(Planet *p, const double sun_size);
    double Overlap(const double elong, const double sun_radius, 
                   const double p_radius);

    double OverlapEllipse(const double elong, const double sunRadius, 
                          const double planetRadius, 
                          const double X, const double Y, const double Z,
                          const double sunX, const double sunY, 
                          const double sunZ, const double ratio,
                          Planet *planet);
    void CreateMap();
    void CopyBlock(unsigned char *dest, unsigned char *src, 
                   const int x1, const int y1,
                   int x2, int y2);
};

#endif
