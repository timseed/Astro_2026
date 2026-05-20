#include <cstdlib>
#include <cstring>
#include <map>
#include <sstream>
#include <string>
using namespace std;

#include "Map.h"
#include "Options.h"
#include "PlanetProperties.h"
#include "Ring.h"
#include "xpUtil.h"

#include "libimage/Image.h"
#include "libplanet/Planet.h"

Map::Map(const int w, const int h) 
    : width_(w), height_(h), area_(w*h), 
      mapData_(NULL), nightData_(NULL),
      latArray_(NULL), lonArray_(NULL),
      cosLatArray_(NULL), cosLonArray_(NULL), 
      sinLatArray_(NULL), sinLonArray_(NULL), 
      target_(NULL), targetProperties_(NULL), 
      ring_(NULL), sunLat_(0), sunLon_(0)
{
    SetUpMap();
}

Map::Map(const int w, const int h, 
         const double sunLat, const double sunLon, 
         Planet *t, PlanetProperties *tp, Ring *r, 
         map<double, Planet *> &planetsFromSunMap) 
    : width_(w), height_(h), area_(w*h),
      latArray_(NULL), lonArray_(NULL),
      cosLatArray_(NULL), cosLonArray_(NULL), 
      sinLatArray_(NULL), sinLonArray_(NULL), 
      target_(t), targetProperties_(tp),
      ring_(r), sunLat_(sunLat), sunLon_(sunLon)
{
    SetUpMap();

    memcpy(color_, tp->Color(), 3);

    dayData_ = new unsigned char [3*area_];
    for (int i = 0; i < 3*area_; i +=3)
        memcpy(dayData_ + i, color_, 3);

    nightData_ = new unsigned char [3*area_];
    memcpy(nightData_, dayData_, 3 * area_);
    
    const double shade = tp->Shade();
    if (shade == 0)
    {
        memset(nightData_, 0, 3 * area_);
    }
    else if (shade < 1)
    {
        for (int i = 0; i < 3 * area_; i++)
            nightData_[i] = (unsigned char) (shade * nightData_[i]);
    }

    AddShadows(planetsFromSunMap);

    mapData_ = new unsigned char [3*area_];
    memcpy(mapData_, dayData_, 3 * area_);

    CreateMap();

    delete [] dayData_;
    delete [] nightData_;
}

Map::Map(const int w, const int h, 
         const double sunLat, const double sunLon, 
         const double obsLat, const double obsLon, 
         const unsigned char *day, const unsigned char *night,
         const unsigned char *bump,
         const unsigned char *specular, const unsigned char *clouds, 
         Planet *t, PlanetProperties *tp, Ring *r, 
         map<double, Planet *> &planetsFromSunMap)
    : width_(w), height_(h), area_(w*h), 
      latArray_(NULL), lonArray_(NULL), 
      cosLatArray_(NULL), cosLonArray_(NULL), 
      sinLatArray_(NULL), sinLonArray_(NULL), 
      target_(t), targetProperties_(tp),
      ring_(r), sunLat_(sunLat), sunLon_(sunLon)
{
    SetUpMap();

    memcpy(color_, tp->Color(), 3);

    try 
    {
        dayData_ = new unsigned char [3*area_];
    }
    catch (bad_alloc &e)
    {
        xpExit("Can't allocate memory for day map\n", __FILE__, __LINE__);
    }
    memcpy(dayData_, day, 3 * area_);

    nightData_ = new unsigned char [3*area_];
    const double shade = tp->Shade();

    if (shade == 0)        // night side is completely dark (shade is 0%)
    {
        memset(nightData_, 0, 3 * area_);
    }
    else if (shade == 1)   // night side is same as day side (shade is 100%)
    {
        memcpy(nightData_, dayData_, 3 * area_);
    }
    else
    {
        if (night == NULL) // no night image specified, so shade the day map
        {
            memcpy(nightData_, dayData_, 3 * area_);

            for (int i = 0; i < 3 * area_; i++)
                nightData_[i] = static_cast<unsigned char> (shade 
                                                            * nightData_[i]);
        }
        else
        {
            memcpy(nightData_, night, 3 * area_);
        }
    }

    if (bump != NULL) AddBumpMap(bump);

    if (specular != NULL) AddSpecularReflection(specular, obsLat, obsLon);

    if (clouds != NULL) OverlayClouds(clouds);

    AddShadows(planetsFromSunMap);

    mapData_ = new unsigned char [3*area_];
    memcpy(mapData_, dayData_, 3 * area_);

    CreateMap();

    delete [] dayData_;
    delete [] nightData_;
}

Map::~Map()
{
    delete [] latArray_;
    delete [] lonArray_;
    delete [] cosLatArray_;
    delete [] cosLonArray_;
    delete [] sinLatArray_;
    delete [] sinLonArray_;
    delete [] mapData_;
}

void
Map::SetUpMap()
{
    mapWidth_ = TWO_PI * target_->Flipped();
    mapHeight_ = M_PI;

    startLon_ = -M_PI * target_->Flipped();
    startLat_ = M_PI_2;

    if (targetProperties_->MapBounds())
    {
        double uly, ulx, lry, lrx;
        targetProperties_->MapBounds(uly, ulx, lry, lrx);

        mapHeight_ = (uly - lry) * deg_to_rad;
        if (mapHeight_ > M_PI)
            xpWarn("Map is too high\n", __FILE__, __LINE__);

        mapWidth_ = (lrx - ulx) * deg_to_rad;
        if (mapWidth_ > TWO_PI)
            xpWarn("Map is too wide\n", __FILE__, __LINE__);

        startLon_ = ulx * deg_to_rad;
        startLat_ = uly * deg_to_rad;
    }

    delLon_ = mapWidth_/width_;
    delLat_ = mapHeight_/height_;

    delete [] lonArray_;
    lonArray_ = new double[width_];
    delete [] cosLonArray_;
    cosLonArray_ = new double[width_];
    delete [] sinLonArray_;
    sinLonArray_ = new double[width_];
    for (int i = 0; i < width_; i++) 
    {
        lonArray_[i] = (i + 0.5) * delLon_ + startLon_;
        cosLonArray_[i] = cos(lonArray_[i]);
        sinLonArray_[i] = sin(lonArray_[i]);
    }
    
    delete [] latArray_;
    latArray_ = new double[height_];
    delete [] cosLatArray_;
    cosLatArray_ = new double[height_];
    delete [] sinLatArray_;
    sinLatArray_ = new double[height_];
    for (int i = 0; i < height_; i++)
    {
        latArray_[i] = startLat_ - (i + 0.5) * delLat_;
        cosLatArray_[i] = cos(latArray_[i]);
        sinLatArray_[i] = sin(latArray_[i]);
    }
}

void
Map::AddBumpMap(const unsigned char *bump)
{
    double scale = 0.1 * targetProperties_->BumpScale() / 255.;
    double shade = targetProperties_->BumpShade();
    unsigned char shaded[3];

    double *height =  new double[area_];
    for (int i = 0; i < area_; i++) 
        height[i] = bump[3*i] * scale;

    // Sun's direction
    double sunloc[3];
    sunloc[0] = cos(sunLat_) * cos(sunLon_);
    sunloc[1] = cos(sunLat_) * sin(sunLon_);
    sunloc[2] = sin(sunLat_);

    int ipos = width_;
    for (int j = 1; j < height_ - 1; j++)
    {
        for (int i = 0; i < width_; i++)
        {
            double hplat = 1 + height[ipos - width_];
            double hmlat = 1 + height[ipos + width_];
            
            double hplon = 1 + height[ipos + 1];
            double hmlon = 1 + height[ipos - 1];
            
            double x1 = hplat * cosLatArray_[j-1] * cosLonArray_[i];
            double y1 = hplat * cosLatArray_[j-1] * sinLonArray_[i];
            double z1 = hplat * sinLatArray_[j-1];
                
            double x0 = hmlat * cosLatArray_[j+1] * cosLonArray_[i];
            double y0 = hmlat * cosLatArray_[j+1] * sinLonArray_[i];
            double z0 = hmlat * sinLatArray_[j+1];

            double dhdlatf[3]; // Finite difference dh/dlat in XYZ
            dhdlatf[0] = x1 - x0;
            dhdlatf[1] = y1 - y0;
            dhdlatf[2] = z1 - z0;

            int ii = (i + 1) % width_;
            x1 = hplon * cosLatArray_[j] * cosLonArray_[ii];
            y1 = hplon * cosLatArray_[j] * sinLonArray_[ii];
            z1 = hplon * sinLatArray_[j];
                
            ii = (i - 1 + width_) % width_;
            x0 = hmlon * cosLatArray_[j] * cosLonArray_[ii];
            y0 = hmlon * cosLatArray_[j] * sinLonArray_[ii];
            z0 = hmlon * sinLatArray_[j];

            double dhdlonf[3]; // Finite difference dh/dlon in XYZ
            dhdlonf[0] = x1 - x0;
            dhdlonf[1] = y1 - y0;
            dhdlonf[2] = z1 - z0;
            dhdlonf[0] *= target_->Flipped();
            dhdlonf[1] *= target_->Flipped();

            // Find the normal to the topography
            double normt[3];
            cross(dhdlonf, dhdlatf, normt);

            // normalize it
            double len = sqrt(dot(normt, normt));
            if (len > 0)
                for (int k = 0; k < 3; k++)
                    normt[k] /= len;
            
            // Find the shading due to topography and the curvature of
            // the planet
            double shadt = 0.5 * (1 + ndot(sunloc, normt));

            // This is the normal at the surface of a sphere
            double normal[3];
            normal[0] = cosLatArray_[j] * cosLonArray_[i];
            normal[1] = cosLatArray_[j] * sinLonArray_[i];
            normal[2] = sinLatArray_[j];

            // Find the shading due to the curvature of the planet
            double shadn = 0.5 * (1 + ndot(sunloc, normal));

            // This should be the shading due to topography
            double shading = shadt/shadn;

            if (shading < 0) shading = 0;
            else if (shading > 1) shading = 1;

            unsigned char *day = dayData_ + 3*ipos;
            unsigned char *night = nightData_ + 3*ipos;
            if (shade >= 0 && shade <= 1)
            {
                for (int k = 0; k < 3; k++)
                    shaded[k] = static_cast<unsigned char>(day[k] * shade);
                night = shaded;
            }
            for (int k = 0; k < 3; k++) 
                day[k] = static_cast<unsigned char>((day[k]-night[k]) 
                                                    * shading + night[k]);
            ipos++;
        }
    }
    delete [] height;
}

void
Map::AddSpecularReflection(const unsigned char *specular,
                           const double obsLat, const double obsLon)
{
    double tc, dist;
    calcGreatArc(sunLat_, sunLon_, obsLat, obsLon, tc, dist);

    double sin_lat1 = sin(sunLat_);
    double cos_lat1 = cos(sunLat_);
    double sin_tc = sin(tc);
    double cos_tc = cos(tc);
    double sin_d2 = sin(dist/2);
    double cos_d2 = cos(dist/2);
    
    double mid_lat = asin(sin_lat1 * cos_d2 
                          + cos_lat1 * sin_d2 * cos_tc);
    double dlon = atan2(sin_tc * sin_d2 * cos_lat1, 
                        cos_d2 - sin_lat1 * sin(mid_lat));
    double mid_lon = fmod(sunLon_ - dlon + M_PI, TWO_PI) - M_PI;

    double midpoint[3];
    midpoint[0] = cos(mid_lat) * cos(mid_lon);
    midpoint[1] = cos(mid_lat) * sin(mid_lon);
    midpoint[2] = sin(mid_lat);
    
    double point[3];
    int ipos = 0;
    for (int j = 0; j < height_; j++)
    {
        for (int i = 0; i < width_; i++)
        {
            point[0] = cosLatArray_[j] * cosLonArray_[i];
            point[1] = cosLatArray_[j] * sinLonArray_[i];
            point[2] = sinLatArray_[j];

            double x = 0.96 * dot(point, midpoint);
            if (x > 0.8) 
            {
                // I just picked these numbers to make it look good (enough)
                x = pow(x, 24);
                
                double brightness = x * specular[ipos];
                double b255 = brightness/255;
                for (int k = 0; k < 3; k++)
                    dayData_[ipos + k] = (unsigned char) 
                        (brightness + (1 - b255) * dayData_[ipos + k]);
            }
            ipos += 3;
        }
    }   
}

void
Map::OverlayClouds(const unsigned char *clouds)
{
    unsigned char *new_clouds = new unsigned char [3 * area_];
    memcpy(new_clouds, clouds, 3 * area_);

    const double gamma = targetProperties_->CloudGamma();
    if (gamma != 1)
    {
        unsigned char *map = new unsigned char[256];
        if (gamma > 0)
        {
            for (int i = 0; i < 256; i++)
            {
                map[i] = ((unsigned char) 
                          (pow(((double) i) / 255, 
                               1.0/gamma) * 255));
            }
        }
        else
        {
            memset(map, 0, 256);
        }

        for (int i = 0; i < 3 * area_; i++)
            new_clouds[i] = map[clouds[i]];

        delete [] map;
    }

    const double shade = targetProperties_->Shade();
    int ipos = 0;
    for (int i = 0; i < area_; i++)
    {
        if ((int) new_clouds[ipos] >= targetProperties_->CloudThreshold()) 
        {
            for (int j = ipos; j < ipos + 3; j++)
            {
                const unsigned char cloudVal = new_clouds[j];
                const double opacity = ((double) cloudVal) / 255;
                double color = (opacity * cloudVal 
                                + (1-opacity) * dayData_[j]);
                dayData_[j] = (unsigned char) color;
                color = (opacity * shade * cloudVal
                         + (1-opacity) * nightData_[j]);
                nightData_[j] = (unsigned char) color;
            }
        }
        ipos += 3;
    }
    
    Options *options = Options::getInstance();
    if (options->MakeCloudMaps())
    {
        string outputDir(options->TmpDir());

        string outputFilename = options->OutputBase();
        if (outputFilename.empty()) outputFilename.assign("clouds");
        outputFilename += options->OutputExtension();

        string dayFile = outputDir + "day_" + outputFilename;
        Image d(width_, height_, dayData_, NULL);
        d.Quality(options->JPEGQuality());
        if (!d.Write(dayFile.c_str()))
        {
            ostringstream errStr;
            errStr << "Can't create " << dayFile << "\n";
            xpExit(errStr.str(), __FILE__, __LINE__);
        }

        string nightFile = outputDir + "night_" + outputFilename;
        Image n(width_, height_, nightData_, NULL);
        n.Quality(options->JPEGQuality());
        if (!n.Write(nightFile.c_str()))
        {
            ostringstream errStr;
            errStr << "Can't create " << nightFile << "\n";
            xpExit(errStr.str(), __FILE__, __LINE__);
        }

        if (options->Verbosity() > 0)
        {
            ostringstream msg;
            msg << "Wrote " << dayFile << " and " << nightFile 
                << ".\n";
            xpMsg(msg.str(), __FILE__, __LINE__);
        }
        exit(EXIT_SUCCESS);
    }

    delete [] new_clouds;
}

void
Map::AddShadows(map<double, Planet *> &planetsFromSunMap)
{
    double tX, tY, tZ;
    target_->getPosition(tX, tY, tZ);
    const double sun_dist = sqrt(tX*tX + tY*tY + tZ*tZ);

    Planet *Sun = planetsFromSunMap.begin()->second;

    // The target body's angular radius as seen from the sun
    const double size = target_->Radius() / sun_dist;

    // The Sun's angular radius as seen from the target body
    const double sun_size = Sun->Radius() / sun_dist;

    Options *options = Options::getInstance();

    for (map<double, Planet *>::iterator it0 = planetsFromSunMap.begin(); 
         it0 != planetsFromSunMap.end(); it0++)
    {
        Planet *p = it0->second;
        if (p->Index() == target_->Index() 
            || p->Index() == SUN) continue;

        double pX, pY, pZ;
        p->getPosition(pX, pY, pZ);

        const double p_sun_dist = it0->first;

        // If this body is farther from the sun than the target body
        // we're finished since the map is stored in order of
        // heliocentric distance
        if (p_sun_dist > sun_dist) return;

        // This planet's angular radius as seen from the sun
        const double p_size = p->Radius() / p_sun_dist;

        // compute the angular separation of the two bodies as seen
        // from the Sun
        double t_vec[3] = { tX/sun_dist, tY/sun_dist, tZ/sun_dist };
        double p_vec[3] = { pX/p_sun_dist, pY/p_sun_dist, pZ/p_sun_dist };
        const double sep = acos(dot(t_vec, p_vec));

        // If the separation is bigger than the sum of the apparent radii,
        // there's no shadow
        if (sep > 1.1 * (size + p_size)) continue;

        if (options->Verbosity() > 1)
        {
            ostringstream msg;
            msg << "separation between " << body_string[p->Index()] 
                << " and " << body_string[target_->Index()] << " is " 
                << sep/deg_to_rad << "\n";
            msg << "Computing shadow from " 
                << body_string[p->Index()] << "\n";
            xpMsg(msg.str(), __FILE__, __LINE__);
        }

        AddShadow(p, sun_size);
    }
}

// Add the shadow cast by Planet p on the map
void
Map::AddShadow(Planet *p, const double sun_size)
{
    double pX, pY, pZ;
    p->getPosition(pX, pY, pZ);

    double tX, tY, tZ;
    target_->getPosition(tX, tY, tZ);

    const double ratio = 1/(1 - p->Flattening());
    double sunX, sunY, sunZ;
    p->XYZToPlanetaryXYZ(0, 0, 0, sunX, sunY, sunZ);
    sunZ *= ratio;

    const double border = sin(targetProperties_->Twilight() * deg_to_rad);

    for (int j = 0; j < height_; j++)
    {
        const double lat = latArray_[j];
        const double radius = target_->Radius(lat);

        for (int i = 0; i < width_; i++)
        {
            const double lon = lonArray_[i];

            double X, Y, Z;
            target_->PlanetographicToXYZ(X, Y, Z, lat, lon, radius);

            // if this point is on the night side, skip it
            if (ndot(tX - X, tY - Y, tZ - Z, tX, tY, tZ) < -border) continue;

            const double sun_dist = sqrt(X*X + Y*Y + Z*Z);

            const double p_dist = sqrt((pX - X) * (pX - X)
                                       + (pY - Y) * (pY - Y)
                                       + (pZ - Z) * (pZ - Z));

            // angular size of the shadowing body seen from this point
            const double p_size = p->Radius() / p_dist;

            // compute separation between shadowing body and the Sun
            // as seen from this spot on the planet's surface
            const double S[3] = { -X/sun_dist, -Y/sun_dist, -Z/sun_dist };
            const double P[3] = { (pX - X)/p_dist, (pY - Y)/p_dist, 
                                  (pZ - Z)/p_dist };
            const double sep = acos(dot(S, P)); 

            // If the separation is bigger than the sum of the
            // apparent radii, this point isn't in shadow
            if (sep > (sun_size + p_size)) continue;

            // compute the covered fraction of the Sun's disk
            double covered;
            if ((p->Index() == JUPITER || p->Index() == SATURN)
                && target_->Primary() == p->Index())
            {
                // if a satellite of Jupiter or Saturn is in its
                // primary's shadow
                covered = OverlapEllipse(sep, sun_size, p_size, X, Y, Z, 
                                         sunX, sunY, sunZ, ratio, p);
            }
            else
            {
                covered = Overlap(sep, sun_size, p_size);
            }

            if (covered >= 0)
            {
                int ipos = 3 * (j * width_ + i);
                for (int k = 0; k < 3; k++)
                {
                    dayData_[ipos] = (unsigned char) ((1 - covered)
                                                      * dayData_[ipos]
                                                      + covered 
                                                      * nightData_[ipos]);
                    ipos++;
                }
            }
        }
    }
}

// The Sun's center is at S.  The planet's center is at P.  The disks
// intersect at points A and B.  Point I is the intersection of the
// lines SP and AB.  In order to find the overlap area we need to find
// the areas of the triangles ASI and API as well as the areas of the
// sectors covered by angles ASI and API.
double
Map::Overlap(const double elong, const double sun_radius, 
             const double p_radius)
{
    // First consider special cases 

    // The centers are separated by more than the sum of the radii, so
    // no intersection
    if (elong > (sun_radius + p_radius)) return(0);

    // The centers are separated by less than the difference of the
    // radii, so one circle is contained within the other
    if (elong < fabs(sun_radius - p_radius))
    {
        // Sun is completely covered
        if (sun_radius < p_radius) return(1);

        // Annular eclipse
        const double ratio = p_radius/sun_radius;
        return(ratio*ratio);
    }
    
    const double d = elong;
    const double r0 = sun_radius;
    const double r1 = p_radius;
    const double r0sq = r0*r0;
    const double r1sq = r1*r1;

    const double cos_ASI = (d*d + r0sq - r1sq) / (2*r0*d);
    const double ASI = acos(cos_ASI);
    const double sin_ASI = sin(ASI);

    const double cos_API = (d*d + r1sq - r0sq) / (2*r1*d);
    const double API = acos(cos_API);
    const double sin_API = sin(API);

    // It's okay if this "area" is negative.  In that case angle ASI
    // is obtuse and we want to add area_ASI to the sector ASI instead
    // of subtract.  
    const double area_ASI = cos_ASI * sin_ASI;
    const double sect_ASI = ASI;

    const double area_API = cos_API * sin_API;
    const double sect_API = API;

    double coverage = (r0sq * (sect_ASI - area_ASI) 
                       + r1sq * (sect_API - area_API));
    coverage /= (M_PI * r0sq);

    return(coverage);
}

/*
  Use for satellites shadowed by Jupiter or Saturn.  Assume that the
  shadowing ellipse is much bigger than the sun.

  To estimate coverage of the solar disk: treat shadowing planet limb
  as a straight edge.
*/
double
Map::OverlapEllipse(const double elong, const double sunRadius, 
                    const double planetRadius, 
                    const double X, const double Y, const double Z,
                    const double sunX, const double sunY,
                    const double sunZ, const double ratio,
                    Planet *planet)
{
    const double p1X = sunX, p1Y = sunY, p1Z = sunZ;
    double p2X, p2Y, p2Z; // point in shadow
    double p3X = 0, p3Y = 0, p3Z = 0; // shadowing planet center

    planet->XYZToPlanetaryXYZ(X, Y, Z, p2X, p2Y, p2Z);
    p2Z *= ratio;

    double u = dot(p3X - p1X, p3Y - p1Y, p3Z - p1Z,
                   p2X - p1X, p2Y - p1Y, p2Z - p1Z);
    u /= dot(p2X - p1X, p2Y - p1Y, p2Z - p1Z, 
             p2X - p1X, p2Y - p1Y, p2Z - p1Z);

    // coordinates of the closest point to shadowing planet's limb
    double iX, iY, iZ;
    iX = p1X + u * (p2X - p1X);
    iY = p1Y + u * (p2Y - p1Y);
    iZ = p1Z + u * (p2Z - p1Z);
    
    iZ /= ratio;

    double lat, lon;
    planet->PlanetaryXYZToXYZ(iX, iY, iZ, iX, iY, iZ);
    planet->XYZToPlanetographic(iX, iY, iZ, lat, lon);

    const double Re = planet->Radius(lat);
    const double Rs = sunRadius/planetRadius;
    const double sep = elong/planetRadius;

    // d ranges from -1 to 1
    const double d = (Re - sep) / Rs;
    double coverage;
    if (d < -1)
    {
        // The centers are separated by more than the sum of the
        // radii, so no intersection
        coverage = 0;
    }
    else if (d > 1)
    {
        // The centers are separated by less than the difference of
        // the radii, so Sun is completely covered
        coverage = 1;
    }
    else
    {
        coverage = (d * sqrt(1 - d*d) + asin(d) + M_PI_2)/M_PI;
    }
    return(coverage);
}

void
Map::Reduce(const int factor)
{
    if (factor < 1) return;

    Options *options = Options::getInstance();
    if (options->Verbosity() > 1)
    {
        ostringstream msg;
        msg << "Shrinking map by 2^" << factor << "\n";;
        xpMsg(msg.str(), __FILE__, __LINE__);
    }

    int scale = 1;
    for (int i = 0; i < factor; i++) scale *= 2;

    int w = width_ / scale;
    int h = height_ / scale;

    int min_width = 16;

    if (w < min_width) 
    {
        w = min_width;
        h = min_width/2;
        scale = width_/w;
    }

    const double scale2 = scale*scale;

    const int new_area = w * h;

    unsigned char *new_data = new unsigned char [3 * new_area];
    memset(new_data, 0, 3 * new_area);

    double *average = new double [3 * new_area];
    for (int i = 0; i < 3*new_area; i++) average[i] = 0;

    int ipos = 0;
    for (int j = 0; j < height_; j++)
    {
        const int js = j / scale;
        for (int i = 0; i < width_; i++)
        {
            const int is = i/scale;
            for (int k = 0; k < 3; k++)
                average[3*(js*w+is)+k] += (double) (mapData_[3*ipos+k]);

            ipos++;
        }
    }

    for (int i = 0; i < 3*new_area; i++)
        new_data[i] = (unsigned char) (average[i]/scale2);

    delete [] average;
    delete [] mapData_;

    mapData_ = new_data;

    width_ = w;
    height_ = h;

    area_ = w * h;

    SetUpMap();
}

void 
Map::GetPixel(const double lat, double lon, unsigned char pixel[3]) const
{
    lon = fmod(lon, TWO_PI);
    if (lon > M_PI) lon -= TWO_PI;

    double x = (lon - startLon_)/delLon_;
    if (targetProperties_->MapBounds())
    {
        if (x < 0 || x >= width_)
        {
            memcpy(pixel, color_, 3);
            return;
        }
    }

    if (x < -0.5) x = -0.5;
    if (x >= width_ - 0.5) x = width_ - 0.5;

    int ix0 = (int) (floor(x));
    int ix1 = ix0 + 1;
    if (ix0 < 0) ix0 = width_ - 1;
    if (ix1 >= width_) ix1 = 0;

    double y = (startLat_ - lat)/delLat_;
    if (targetProperties_->MapBounds())
    {
        if (y < 0 || y >= height_)
        {
            memcpy(pixel, color_, 3);
            return;
        }
    }

    if (y < -0.5) y = -0.5;
    if (y >= height_ - 0.5) y = height_ - 0.5;

    int iy0 = (int) (floor(y));
    int iy1 = iy0 + 1;
    if (iy0 < 0) iy0 = 0;
    if (iy1 >= height_) iy1 = height_ - 1;

    const double t = x - floor(x);
    const double u = 1 - (y - floor(y));

    double weight[4];
    getWeights(t, u, weight);

    unsigned char *pixels[4];
    pixels[0] = mapData_ + 3 * (iy0 * width_ + ix0);
    pixels[1] = mapData_ + 3 * (iy0 * width_ + ix1);
    pixels[2] = mapData_ + 3 * (iy1 * width_ + ix0);
    pixels[3] = mapData_ + 3 * (iy1 * width_ + ix1);

    memset(pixel, 0, 3);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 3; j++)
            pixel[j] += (unsigned char) (weight[i] * pixels[i][j]);
    }
}

void
Map::CopyBlock(unsigned char *dest, unsigned char *src,
               const int x1, const int y1, int x2, int y2)
{
    for (int j = y1; j < y2; j++)
    {
        memcpy(dest + 3 * (width_ * j + x1), src + 3 * (width_ * j + x1), 
               3 * (x2 - x1));
    }
}

bool
Map::Write(const char *filename) const
{
    Options *options = Options::getInstance();

    Image image(width_, height_, mapData_, NULL);
    image.Quality(options->JPEGQuality());
    const bool success = image.Write(filename);
    return success;
}

void
Map::CreateMap()
{
    double sunloc[3];
    sunloc[0] = cos(sunLat_) * cos(sunLon_);
    sunloc[1] = cos(sunLat_) * sin(sunLon_);
    sunloc[2] = sin(sunLat_);

    double point[3];
    const double border = sin(targetProperties_->Twilight() * deg_to_rad);
    
    if (border == 0)
    {
        // number of rows at top and bottom that are in polar day/night
        const int ipolar = abs(static_cast<int>(sunLat_/delLat_));

        if (sunLat_ < 0) // North pole is dark
            CopyBlock(mapData_, nightData_, 0, 0, width_, ipolar);
        else             // South pole is dark
            CopyBlock(mapData_, nightData_, 0, height_ - ipolar, 
                      width_, height_);

        // subsolar longitude pixel column - this is where it's noon
        int inoon = static_cast<int> (width_/2 
                                      * (sunLon_ * target_->Flipped() 
                                         / M_PI - 1));
        while (inoon < 0) inoon += width_;
        while (inoon >= width_) inoon -= width_;

        for (int i = ipolar; i < height_ - ipolar; i++)
        {
            double length_of_day;

            /* compute length of daylight as a fraction of the day at
               the current latitude.  Based on Chapter 42 of
               Astronomical Formulae for Calculators by Meeus. */

            double H0 = tan(latArray_[i]) * tan(sunLat_);
            if (H0 > 1) 
                length_of_day = 1;
            else if (H0 < -1) 
                length_of_day = 0;
            else 
                length_of_day = 1 - (acos(H0) / M_PI);

            // idark = number of pixels that are in darkness at the
            // current latitude
            int idark = static_cast<int> (width_ * (1 - length_of_day));

            // ilight = number of pixels from noon to the terminator
            int ilight = (width_ - idark)/2;

            // start at the evening terminator
            int start_row = i * width_;
            int ipos = inoon + ilight;

            for (int j = 0; j < idark; j++)
            {
                if (ipos >= width_) ipos -= width_;
                memcpy(mapData_ + 3 * (start_row + ipos),
                       nightData_ + 3 * (start_row + ipos), 3);
                ipos++;
            }
        }       
    }
    else
    {
        // break the image up into a 100x100 grid
        const int sections = 100;

        int jstep = height_/sections;
        if (jstep == 0) jstep = 1;

        int istep = width_/sections;
        if (istep == 0) istep = 1; 

        for (int j = 0; j < height_; j += jstep)
        {
            int uly = j;
            int lry = uly + jstep;
            if (lry >= height_) lry = height_ - 1;
            double cos_lat = cosLatArray_[(uly + lry)/2];
            double sin_lat = sinLatArray_[(uly + lry)/2];

            for (int i = 0; i < width_; i += istep)
            {
                int ulx = i;
                int lrx = ulx + istep;
                if (lrx >= width_) lrx = width_ - 1;
                double cos_lon = cosLonArray_[(ulx + lrx)/2];
                double sin_lon = sinLonArray_[(ulx + lrx)/2];
            
                point[0] = cos_lat * cos_lon;
                point[1] = cos_lat * sin_lon;
                point[2] = sin_lat;

                double x = dot(point, sunloc);

                if (x < -2*border)  // NIGHT
                {
                    CopyBlock(mapData_, nightData_, ulx, uly, lrx+1, lry+1);
                }
                else if (x < 2*border ) // TWILIGHT
                {
                    for (int jj = uly; jj <= lry; jj++)
                    {
                        for (int ii = ulx; ii <= lrx; ii++)
                        {
                            point[0] = cosLatArray_[jj] * cosLonArray_[ii];
                            point[1] = cosLatArray_[jj] * sinLonArray_[ii];
                            point[2] = sinLatArray_[jj];

                            double dayweight = ((border + dot(point, sunloc))
                                                / (2 * border));
                            int ipos = 3 * (jj * width_ + ii);
                            if (dayweight < 0)
                            {
                                memcpy(mapData_ + ipos, nightData_ + ipos, 3);
                            }
                            else if (dayweight < 1) 
                            {
                                dayweight = (1 - cos(dayweight * M_PI)) / 2;
                                for (int k = 0; k < 3; k++)
                                {
                                    double color = (dayweight * dayData_[ipos] 
                                                    + ((1 - dayweight) 
                                                       * nightData_[ipos]));
                                    mapData_[ipos++] = (unsigned char) color;
                                }
                            }
                        } // for ( ii = ... ) block
                    }     // for ( jj = ... ) block
                }         // end TWILIGHT block
            }
        }
    } // end (border > 0) block

    // draw the shadow of Saturn's rings on the planet
    if (target_->Index() == SATURN)
    {
        for (int j = 0; j < height_; j++)
        {
            // If this point is on the same side of the rings as
            // the sun, there's no shadow.  
            if (sunLat_ * latArray_[j] > 0) continue;

            const double lat = latArray_[j];
            for (int i = 0; i < width_; i++)
            {
                const double lon = lonArray_[i];
                point[0] = cosLatArray_[j] * cosLonArray_[i];
                point[1] = cosLatArray_[j] * sinLonArray_[i];
                point[2] = sinLatArray_[j];
                
                // If it's night, skip this one
                if (dot(point, sunloc) < -2*border) continue;
                
                const double ring_radius = ring_->getShadowRadius(lat, lon);
                const double ring_radius2 = ring_->getShadowRadius(lat + delLat_,
                                                                   lon + delLon_);

                const double dpp = 2*fabs(ring_radius2 - ring_radius);
                ring_->setDistPerPixel(dpp);

                double t = ring_->getTransparency(ring_radius); 
                if (t > 0)
                {
                    int ipos = 3 * (j * width_ + i);
                    for (int k = 0; k < 3; k++)
                    {
                        double color = (t * mapData_[ipos] 
                                        + ((1 - t) * nightData_[ipos]));
                        mapData_[ipos++] = (unsigned char) color;
                    }
                }
            }
        }
    }
}
