#ifndef LINESEGMENT_H
#define LINESEGMENT_H

#include "Annotation.h"

class LineSegment : public Annotation
{
 public:
    LineSegment(const unsigned char color[3], const int thickness,
                const double X0, const double Y0,
                const double X1, const double Y1);

    virtual ~LineSegment();

    virtual void Shift(const int x) { X0_ += x; X1_ += x; };
    virtual void Draw(DisplayBase *display);

 private:

    double X0_, Y0_, X1_, Y1_;
    double thickness_;

};

#endif
