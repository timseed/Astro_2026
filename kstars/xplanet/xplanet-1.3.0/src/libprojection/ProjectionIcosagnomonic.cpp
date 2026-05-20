/*
  This projection contributed by Ian Turner <vectro@vectro.org>.  His
  comments:

  I have created a new projection for XPlanet, entitled
  "icosagnomonic". It is very similar to the Fuller projection, but I
  have used the gnomonic projection instead of fuller's method to
  perform the transformation from spherical triangle to planar
  triangle (since the exact reverse transformation equation for
  Fuller's method is unknown). I have also changed Fuller's
  orientation of some of the triangles, so that the antarctic circle
  is not broken up.
*/

#include <cmath>
#include <sstream>
#include <vector>
using namespace std;

#include "Options.h"
#include "ProjectionIcosagnomonic.h"
#include "xpUtil.h"

// Aspect ratio of map.
#define TWIDE 5.25
#define THIGH 3
#define RATIO (((THIGH)*sqrt(3.0)/2)/(TWIDE))

bool
ProjectionIcosagnomonic::PointXY::sameSide(const PointXY& p1,
                                           const PointXY& p2,
                                           const PointXY& l1,
                                           const PointXY& l2)
{
    PointXY linec = l2 - l1;
    PointXY pointc = p1 - l1;
    double cp1 = linec.x*pointc.y - linec.y*pointc.x;
  
    pointc = p2 - l1;
    double cp2 = linec.x*pointc.y - linec.y*pointc.x;

    return ((cp1 < 0 && cp2 < 0) || (cp1 > 0 && cp2 > 0));
}

bool
ProjectionIcosagnomonic::PointXY::inTriangle(const PointXY& a,
                                             const PointXY& b,
                                             const PointXY& c)
{
    return (sameSide(*this, a, b, c) &&
            sameSide(*this, b, a, c) &&
            sameSide(*this, c, a, b));
}

double
ProjectionIcosagnomonic::PointXY::distance(const PointXY& p) const {
    PointXY q = p-*this;
    return sqrt(q.x*q.x+q.y*q.y);
}

// Rotate counterclockwise around the origin by the specified angle.
void
ProjectionIcosagnomonic::PointXY::rotate(double phi) {
    // We convert to a vector, add the angle, and go back.
    double mag = sqrt(x*x + y*y);
    double theta = atan2(y, x);
    
    theta += phi;
    
    x = mag * cos(theta);
    y = mag * sin(theta);
}

int
ProjectionIcosagnomonic::PointXY::cmpLine(const PointXY& a, const PointXY& b) {
    double v = (b.x - a.x)*(y - a.y) - (x - a.x)*(b.y - a.y);
    if (v < 0) return -1;
    if (v > 0) return 1;
    return 0;
}


ProjectionIcosagnomonic::PointXYZ
ProjectionIcosagnomonic::PointXYZ::crossP(const PointXYZ& a,
                                          const PointXYZ& b) {
    return PointXYZ(a.y*b.z - b.y*a.z, b.x*a.z - a.x*b.z, a.x*b.y - b.x*a.y);
}

double
ProjectionIcosagnomonic::PointXYZ::dotP(const PointXYZ& a,
                                        const PointXYZ& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

bool
ProjectionIcosagnomonic::PointLL::sameSide(const PointLL& p1,
                                           const PointLL& p2,
                                           const PointLL& l1,
                                           const PointLL& l2)
{
    PointXYZ cp = PointXYZ::crossP(l1, l2);
    double dp1 = PointXYZ::dotP(cp, p1);
    double dp2 = PointXYZ::dotP(cp, p2);

    return (dp1 * dp2 >= 0);
}

bool
ProjectionIcosagnomonic::PointLL::inTriangle(const PointLL& a,
                                             const PointLL& b,
                                             const PointLL& c)
{
    return (sameSide(*this, a, b, c) &&
            sameSide(*this, b, a, c) &&
            sameSide(*this, c, a, b));
}

double
ProjectionIcosagnomonic::PointLL::bearing(const PointLL& b) const {
    const PointLL & a = *this;

    double y = sin(b.lon-a.lon)*cos(b.lat);
    double x = cos(a.lat)*sin(b.lat) - sin(a.lat)*cos(b.lat)*cos(b.lon-a.lon);

    return atan2(y, x);
}

void
ProjectionIcosagnomonic::Polygon::scale(double x, double y) {
    for (PointList::iterator i = V.begin(); i != V.end(); i ++) {
        i->x *= x;
        i->y *= y;
    }
}

bool 
ProjectionIcosagnomonic::Polygon::contains(PointXY p) const {
    // Winding number algorithm from 
    // http://geometryalgorithms.com/Archive/algorithm_0103/algorithm_0103.htm
    int wn = 0;

    // loop through all edges of the polygon
    for (PointList::const_iterator i = V.begin(); i != V.end(); i ++) {
        PointList::const_iterator j = i;
        j ++;
        
        if (i->y <= p.y) {         // start y <= P.y
            if (j->y > p.y)      // an upward crossing
                if (p.cmpLine(*i, *j) > 0)  // P left of edge
                    ++wn;            // have a valid up intersect
        }
        else {                       // start y > P.y (no test needed)
            if (j->y <= p.y)     // a downward crossing
                if (p.cmpLine(*i, *j) < 0)  // P right of edge
                    --wn;            // have a valid down intersect
        }
    }

    return wn;
}

ProjectionIcosagnomonic::Triangle::Triangle(PointXY a, PointXY b, PointXY c,
                                            PointLL la, PointLL lb, PointLL lc)
    : cA(a), cB(b), cC(c), sA(la), sB(lb), sC(lc) {
    centerXY = PointXY((a.x + b.x + c.x)/3, (a.y + b.y + c.y)/3);
    centerLL = sCentroid(la, lb, lc);

    Options o;
    o.Latitude(centerLL.lat);
    o.Longitude(centerLL.lon);
    double dim = cB.distance(cA);

    o.CenterX(0);
    o.CenterY(0);

    // Reversed because (0,0) in upper left.
    PointXY dA(cA.x-centerXY.x, centerXY.y-cA.y);
    double d1 = atan2(dA.y, dA.x) - M_PI/2;// Angle to make map N-S.
    double d2 = centerLL.bearing(la);      // Angle to make point A go north.

    rotation = d1+d2;
     
    o.AddProjectionParameter(33.5*deg_to_rad);

    P = new ProjectionGnomonic(1, (int)dim, (int)dim, &o);
}

ProjectionIcosagnomonic::Triangle::Triangle(const Triangle& T)
    : cA(T.cA), cB(T.cB), cC(T.cC),sA(T.sA), sB(T.sB), sC(T.sC),
      centerXY(T.centerXY), centerLL(T.centerLL),
      rotation(T.rotation), P(new ProjectionGnomonic(*(T.P))) {}

ProjectionIcosagnomonic::Triangle::~Triangle() {
    delete P;
}

/* Finds the spherical centroid of the triangle, by converting to Cartesian
 * coordinates, finding the Cartesian centroid, normalizing to radius 1,
 * and converting back to lat/lon. 
 * 
 * This algorithm is inexact in general, but I think it works for
 * equilateral spherical triangles, which is what we're dealing with. */
ProjectionIcosagnomonic::PointLL
ProjectionIcosagnomonic::Triangle::sCentroid(PointLL a, PointLL b, PointLL c) {
    PointXYZ ca(a);
    PointXYZ cb(b);
    PointXYZ cc(c);

    PointXYZ center1 = PointXYZ((ca.x+cb.x+cc.x)/3,
                                (ca.y+cb.y+cc.y)/3,
                                (ca.z+cb.z+cc.z)/3);
    double scale = sqrt(center1.x*center1.x+
                        center1.y*center1.y+
                        center1.z*center1.z);
    PointXYZ center2 = PointXYZ(center1.x / scale,
                                center1.y / scale,
                                center1.z / scale);

    return PointLL(center2);
}

bool
ProjectionIcosagnomonic::Triangle::contains(PointXY p) const {
    return p.inTriangle(cA, cB, cC);
}

bool
ProjectionIcosagnomonic::Triangle::contains(PointLL p) const {
    return p.inTriangle(sA, sB, sC);
}

bool
ProjectionIcosagnomonic::Triangle::pixelToSpherical(PointXY p,
                                                    double &lon,
                                                    double &lat) const {
    if (!contains(p))
        return false;

    PointXY q(p - centerXY);

    q.rotate(rotation);
    
    return P->pixelToSpherical(q.x, q.y, lon, lat);
}

bool
ProjectionIcosagnomonic::Triangle::sphericalToPixel(double lon,
                                                    double lat,
                                                    double &x,
                                                    double &y) const {
    if (!Triangle::contains(PointLL(lat, lon)))
        return false;

    PointXY p;
    P->sphericalToPixel(lon, lat, p.x, p.y);
    p.rotate(-rotation);
    p += centerXY;
    x = p.x;
    y = p.y;

    return contains(PointXY(x, y));
}

bool
ProjectionIcosagnomonic::ClippedTriangle::contains(PointXY p) const {
    return Triangle::contains(p) && clip.contains(p);
}

bool
ProjectionIcosagnomonic::ClippedTriangle::contains(PointLL p) const {
    double x, y;
    bool success = Triangle::sphericalToPixel(p.lon, p.lat, x, y);
    return success && contains(PointXY(x, y));
}

ProjectionIcosagnomonic::ProjectionIcosagnomonic(const int f, const int w,
                                                 const int h) 
    : ProjectionBase(f, w, h)
{
    isWrapAround_ = false;

    if (w * RATIO > h) {
        _w = h / RATIO;
        _h = h;
    } else {
        _w = w;
        _h = w * RATIO;
    }

    baseh = _h / THIGH;
    basew = _w / TWIDE;
    
    offset = PointXY(centerX_ - _w/2, centerY_ - _h/2);

    makeTriangles();
}

ProjectionIcosagnomonic::~ProjectionIcosagnomonic() {
    for (TList::iterator i = T.begin(); i != T.end(); i ++) {
        delete *i;
    }
}

static int posmin(int a, int b, int c) {
    if (a < b && a < c && a >= 0) {
        return a;
    } else if (b < c && b >= 0) {
        return b;
    } else if (c >= 0) {
        return c;
    } else
        return 0;
}

void
ProjectionIcosagnomonic::makeTriangles() {
    // Relates icosahedron vertices to points on the sphere.
    static const PointLL icosa[] =
        { PointLL(+64.700000 * deg_to_rad,   10.536200 * deg_to_rad),  //  0
          PointLL( +2.300882 * deg_to_rad,   -5.245390 * deg_to_rad),  //  1
          PointLL(+10.447378 * deg_to_rad,   58.157710 * deg_to_rad),  //  2
          PointLL(+39.100000 * deg_to_rad,  122.300000 * deg_to_rad),  //  3
          PointLL(+50.103201 * deg_to_rad, -143.478490 * deg_to_rad),  //  4
          PointLL(+23.717925 * deg_to_rad,  -67.132330 * deg_to_rad),  //  5
          PointLL(-39.100000 * deg_to_rad,  -57.700000 * deg_to_rad),  //  6
          PointLL(-50.103200 * deg_to_rad,   36.521500 * deg_to_rad),  //  7
          PointLL(-23.717925 * deg_to_rad,  112.867670 * deg_to_rad),  //  8
          PointLL( -2.300900 * deg_to_rad,  174.754600 * deg_to_rad),  //  9
          PointLL(-10.447345 * deg_to_rad, -121.842290 * deg_to_rad),  // 10
          PointLL(-64.700000 * deg_to_rad, -169.463800 * deg_to_rad),  // 11
          PointLL(+70.750433 * deg_to_rad,   26.020416 * deg_to_rad),  // 12
          PointLL(+60.846374 * deg_to_rad,   43.157248 * deg_to_rad),  // 13
          PointLL( +0.000000 * deg_to_rad,    0.000000 * deg_to_rad) };// 14

    // Relates plane vertices to icosahedron vertices and to map coordinates.
    static const struct { PointXY p; int v; } rect[] = 
        { { PointXY(0.5, 0),  7 },   //  0
          { PointXY(1.5, 0),  1 },   //  1
          { PointXY(2.5, 0),  5 },   //  2  
          { PointXY(3.5, 0),  1 },   //  3
          { PointXY(4.5, 0),  1 },   //  4
          { PointXY(0  , 1),  7 },   //  5
          { PointXY(1  , 1),  2 },   //  6
          { PointXY(2  , 1),  0 },   //  7
          { PointXY(3  , 1),  5 },   //  8
          { PointXY(4  , 1),  6 },   //  9
          { PointXY(5  , 1),  7 },   // 10
          { PointXY(-.5, 2),  7 },   // 11
          { PointXY(0.5, 2),  8 },   // 12
          { PointXY(1.5, 2),  3 },   // 13
          { PointXY(2.5, 2),  4 },   // 14
          { PointXY(3.5, 2), 10 },   // 15
          { PointXY(4.5, 2), 11 },   // 16
          { PointXY(5.5, 2),  8 },   // 17
          { PointXY(0  , 3), 11 },   // 18
          { PointXY(1  , 3),  9 },   // 19
          { PointXY(2  , 3),  9 },   // 20
          { PointXY(3  , 3),  9 },   // 21
          { PointXY(4  , 3),  9 },   // 22
          { PointXY(5  , 3),  9 },   // 23
          { PointXY(1  , 3),  8 } }; // 24 -- special duplicate of 19.

    // Relates triangles to their planar vertices.
    const int num_triangles = 23;

    // clip is clipping polygon number or -1 for whole triangle.
    static const struct { int v1, v2, v3, clip; } tri[] = 
        { {  0,  1,  6, -1 },   // 0
          {  1,  6,  7, -1 },   // 1
          {  1,  2,  7, -1 },   // 2
          {  3,  8,  9, -1 },   // 3
          {  4,  9, 10, -1 },   // 4
          {  5,  6, 12, -1 },   // 5
          {  6, 12, 13, -1 },   // 6 
          {  6,  7, 13, -1 },   // 7
          {  7, 13, 14, -1 },   // 8
          {  7,  8, 14, -1 },   // 9
          {  8, 14, 15, -1 },   // 10
          {  8,  9, 15, -1 },   // 11
          {  9, 10, 16, -1 },   // 12
          {  9, 15, 16, -1 },   // 13
          { 10, 16, 17,  3 },   // 14
          { 11, 12, 18,  2 },   // 15
          { 12, 18, 19,  4 },   // 16
          { 12, 13, 19,  0 },   // 17
          { 13, 14, 20, -1 },   // 18
          { 13, 24, 20,  1 },   // 19
          { 14, 15, 21, -1 },   // 20
          { 15, 16, 22, -1 },   // 21
          { 16, 17, 23,  5 } }; // 22

    for (int i = 0; i < num_triangles; i ++) {
        int pv1 = tri[i].v1;
        int pv2 = tri[i].v2;
        int pv3 = tri[i].v3;
        PointXY pp1 = PointXY(rect[pv1].p.x*basew, rect[pv1].p.y*baseh);
        PointXY pp2 = PointXY(rect[pv2].p.x*basew, rect[pv2].p.y*baseh);
        PointXY pp3 = PointXY(rect[pv3].p.x*basew, rect[pv3].p.y*baseh);
        PointLL ip1 = icosa[rect[pv1].v];
        PointLL ip2 = icosa[rect[pv2].v];
        PointLL ip3 = icosa[rect[pv3].v];

        if (tri[i].clip >= 0) {
            Polygon clip = makeClippingPath(tri[i].clip);
            clip.scale(basew, baseh);

            T.push_back(new ClippedTriangle(pp1, pp2, pp3,
                                            ip1, ip2, ip3, clip));
        } else {
            T.push_back(new Triangle(pp1, pp2, pp3, ip1, ip2, ip3));
        }
        
        int findx = posmin((int)(rect[pv1].p.x*2),
                           (int)(rect[pv2].p.x*2),
                           (int)(rect[pv3].p.x*2));
        int findy = posmin((int)rect[pv1].p.y,
                           (int)rect[pv2].p.y, 
                           (int)rect[pv3].p.y);
        // Triangles always span exactly two 'x' and one 'y'
        // (not including clipping).
        TFinder[findx][findy].push_back(T.back());
        TFinder[findx+1][findy].push_back(T.back());
    }
}

ProjectionIcosagnomonic::Polygon
ProjectionIcosagnomonic::makeClippingPath(unsigned int n) {
    Polygon::PointList pp;
    switch(n) {
    case 0:
        pp.push_back(PointXY(0.5, 2));
        pp.push_back(PointXY(1, 3));
        pp.push_back(PointXY(1, 2 + 1/3.));
        pp.push_back(PointXY(1.5, 2));
        break;
    case 1:
        pp.push_back(PointXY(1.5, 2));
        pp.push_back(PointXY(1.5, 2 + 2/3.));
        pp.push_back(PointXY(2, 3));
        break;
    case 2:
        pp.push_back(PointXY(0, 2));
        pp.push_back(PointXY(0.25, 2.5));
        pp.push_back(PointXY(0.5, 2));
        break;
    case 3:
        pp.push_back(PointXY(5, 1));
        pp.push_back(PointXY(5.25, 1.5));
        pp.push_back(PointXY(5, 2));
        pp.push_back(PointXY(4.5, 2));
        break;
    case 4:
        pp.push_back(PointXY(0.5, 2));
        pp.push_back(PointXY(0.25, 2.5));
        pp.push_back(PointXY(0.3, 3));
        pp.push_back(PointXY(1, 3));
        break;
    case 5:
        pp.push_back(PointXY(4.5, 2));
        pp.push_back(PointXY(5, 2));
        pp.push_back(PointXY(4.65, 2.3));
        break;

    default: break;
    }

    Polygon p(pp);

    return p;
}

bool
ProjectionIcosagnomonic::pixelToSpherical(const double x, const double y, 
                                          double &lon, double &lat)
{
    PointXY p(x, y);
    p -= offset;

    int fx = (int)(p.x / (basew/2));
    int fy = (int)(p.y / baseh);
    if (fx < 0 || fy < 0 || fx >= FINDCOLS || fy >= FINDROWS)
        return false;
    
    const TList& box = TFinder[fx][fy];
    for (TList::const_iterator i = box.begin(); i != box.end(); i ++) {
        if ((*i)->pixelToSpherical(p, lon, lat)) {

            if (rotate_) RotateXYZ(lat, lon);
            
            if (lon > M_PI) lon -= TWO_PI;
            else if (lon < -M_PI) lon += TWO_PI;
            
            return true;
        }
    }

    return(false);
}

bool
ProjectionIcosagnomonic::sphericalToPixel(double lon, double lat,
                                          double &x, double &y) const
{
    if (rotate_) RotateZYX(lat, lon);
  
    PointLL p(lat, lon);

    for (TList::const_iterator i = T.begin(); i != T.end(); i ++) {
        if ((*i)->contains(p)) {
            if ((*i)->sphericalToPixel(lon, lat, x, y)) {
                x += offset.x;
                y += offset.y;
                return true;
            }
        }
    }

    return(false);
}
