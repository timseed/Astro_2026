#ifndef PROJECTIONICOSAGNOMONIC_H
#define PROJECTIONICOSAGNOMONIC_H

#include "ProjectionBase.h"
#include "ProjectionGnomonic.h"
#include "xpUtil.h"

#define FINDROWS  3
#define FINDCOLS 12

class ProjectionIcosagnomonic : public ProjectionBase
{
public:
    ProjectionIcosagnomonic(const int f, const int w, const int h);
    ~ProjectionIcosagnomonic();

    virtual bool pixelToSpherical(const double x, const double y, 
                                  double &lon, double &lat);

    virtual bool sphericalToPixel(double lon, double lat,
                                  double &x, double &y) const;

private:
    ProjectionIcosagnomonic(const ProjectionIcosagnomonic&);

    class PointXYZ;

    struct PointXY { 
        PointXY() : x(0), y(0) {};
        PointXY(const double _x, const double _y) : x(_x), y(_y) {};
      
        PointXY operator-(const PointXY& p) const
            { return PointXY(this->x - p.x, this->y - p.y); }
        PointXY operator+(const PointXY& p) const
            { return PointXY(this->x + p.x, this->y + p.y); }
        PointXY operator-=(const PointXY& p)
            { return *this = *this - p; }
        PointXY operator+=(const PointXY& p)
            { return *this = *this + p; }

        /* Returns true iff p1 and p2 are on the same side of the line
         * passing through l1 and l2, or if p1 is on same line. */
        static bool sameSide(const PointXY& p1, const PointXY& p2,
                             const PointXY& l1, const PointXY& l2);

        /* Returns true iff *this is in the triangle defined by the three
         * given points. */
        bool inTriangle(const PointXY&, const PointXY&, const PointXY&);

        double distance(const PointXY& P) const; // Cartesian

        void rotate(double angle);

        // For the line including a and b, returns a positive number
        // if our point is to the left, a negative number if it is to the
        // right, or zero if it is on the line. We can't just cast because
        // we might improperly round to zero.
        int cmpLine(const PointXY& a, const PointXY& b);

        double x, y;
    };

    class PointLL { 
    public:
        PointLL() : lat(0), lon(0) {};
        PointLL(const double _at, const double _on) : lat(_at), lon(_on) {};
        PointLL(const PointXYZ& p) : lat(asin(p.z)), lon(atan2(p.y, p.x)) {};
        
        /* Returns true iff p1 and p2 are on the same side of the 
         * great circle passing through l1 and l2. l1 and l2 must not be
         * antipodal. Returns true also if p1 is on the great circle. */
        static bool sameSide(const PointLL& p1, const PointLL& p2,
                             const PointLL& l1, const PointLL& l2);
        
        bool inTriangle(const PointLL&, const PointLL&, const PointLL&);

        double bearing(const PointLL& P) const;

        double lat, lon;
    };

    struct PointXYZ {
        PointXYZ(double a, double b, double c) : x(a), y(b), z(c) {};
        PointXYZ(const PointLL& p)
            : x(cos(p.lon)*cos(p.lat)),
              y(sin(p.lon)*cos(p.lat)),
              z(sin(p.lat)) {};
        
        static PointXYZ crossP(const PointXYZ& a, const PointXYZ& b);
        static double dotP(const PointXYZ&, const PointXYZ&);
        
        double x, y, z;
    };

    class Polygon {
    public:
        typedef vector<PointXY> PointList;

        Polygon(const PointList& v) : V(v) {
            V.push_back(V.front());
        };

        void scale(double x, double y);
        
        bool contains(PointXY p) const;

    private:
        PointList V;
    };

    class Triangle {
    public:
        Triangle(PointXY a, PointXY b, PointXY c,
		 PointLL la, PointLL lb, PointLL lc);
        Triangle(const Triangle& T);
        virtual ~Triangle();
      
        /* x, y are relative to center of triangle. */
        bool pixelToSpherical(PointXY p,
                              double &lon, double &lat) const;
        bool sphericalToPixel(double lon, double lat,
                              double& x, double& y) const;
      
        virtual bool contains(PointXY p) const;
        virtual bool contains(PointLL p) const;

    private:
        PointLL sCentroid(PointLL, PointLL, PointLL);

        PointXY cA, cB, cC;
        PointLL sA, sB, sC;
        PointXY centerXY;
        PointLL centerLL;

        double rotation;

        ProjectionGnomonic* P;
    };

    class ClippedTriangle : public Triangle {
    public:
        ClippedTriangle(PointXY a,  PointXY b,  PointXY c,
			PointLL la, PointLL lb, PointLL lc,
			Polygon path)
            : Triangle(a, b, c, la, lb, lc), clip(path) {};

        ClippedTriangle(const ClippedTriangle& T)
            : Triangle(T), clip(T.clip) {};
	
        virtual bool contains(PointXY p) const;
        virtual bool contains(PointLL p) const;

    private:
        Polygon clip;
    };

    void makeTriangles();
    Polygon makeClippingPath(unsigned int);

    double _w, _h, basew, baseh;
    PointXY offset;
    typedef Triangle* TriangleRef;
    typedef vector<TriangleRef> TList;
    TList T;

    /* This array keeps track of what triangles exist in different regions
     * of the map, which makes looking up triangles in pixelToSpherical much
     * more efficient. */
    TList TFinder[FINDCOLS][FINDROWS];
};

#endif
