#include <cmath>
#include <cstring>
#include <map>
using namespace std;

#include "body.h"
#include "Options.h"
#include "PlanetProperties.h"
#include "View.h"

#include "libannotate/LineSegment.h"
#include "libmultiple/libmultiple.h"
#include "libplanet/Planet.h"

static void
addArc(const double startTime, const double stopTime, 
       const int numTimes, const unsigned char color[3],
       const int thickness, const View *view, 
       const int width, const int height, 
       const double Prx, const double Pry, const double Prz,
       Planet *p, multimap<double, Annotation *> &annotationMap)
{
    Options *options = Options::getInstance();

    const body b = p->Index();
    const double delTime = (stopTime - startTime) / numTimes;

    for (int i = 0; i <= numTimes; i++)
    {
        bool skipThisArc = false;
        double oldX, oldY, oldZ;
        double newX, newY, newZ;

        for (int j = 0; j < 2; j ++)
        {
            const double jd = startTime + (i + j) * delTime;

            double X, Y, Z;
            Planet planet(jd, b);
            planet.calcHeliocentricEquatorial(false);
            planet.getPosition(X, Y, Z);
            
            view->XYZToPixel(X + Prx, Y + Pry, Z + Prz,
                             X, Y, Z);
            X += options->CenterX();
            Y += options->CenterY();
            if (X < -width || X > 2*width 
                || Y < -height || Y > 2*height
                || Z < 0)
            {
                skipThisArc = true;
                break;
            }

            if (j == 0)
            {
                oldX = X;
                oldY = Y;
                oldZ = Z;
            }
            else
            {
                newX = X;
                newY = Y;
                newZ = Z;
            }
        }
        
        if (skipThisArc) continue;

        LineSegment *ls = new LineSegment(color, thickness, 
                                          oldX, oldY,
                                          newX, newY);

        double midZ = 0.5 * (oldZ + newZ);
        annotationMap.insert(pair<const double, Annotation*>(midZ, ls));
    }
}

void
addOrbits(const double jd0, const View *view, 
          const int width, const int height, 
          Planet *p,  PlanetProperties *currentProperties, 
          multimap<double, Annotation *> &annotationMap)
{
    const double period = p->Period();
    if (period == 0) return;

    // Units of orbit circumference
    const double startOrbit = currentProperties->StartOrbit();
    const double stopOrbit = currentProperties->StopOrbit();

    // degrees
    const double delOrbit = currentProperties->DelOrbit();

    const unsigned char *color = currentProperties->OrbitColor();
    const int thickness = currentProperties->ArcThickness();

    double Prx=0, Pry=0, Prz=0;
    if (p->Primary() != SUN)
    {
        Planet primary(jd0, p->Primary());
        primary.calcHeliocentricEquatorial();
        primary.getPosition(Prx, Pry, Prz);
    }

    const double startTime = jd0 + startOrbit * period;
    const double stopTime = jd0 + stopOrbit * period;

    int numTimes = (int) abs(360 * startOrbit / delOrbit + 0.5);
    addArc(startTime, jd0, numTimes, color, thickness, view, width, height, 
           Prx, Pry, Prz, p, annotationMap);

    numTimes = (int) abs(360 * stopOrbit / delOrbit + 0.5);
    addArc(jd0, stopTime, numTimes, color, thickness, view, width, height, 
           Prx, Pry, Prz, p, annotationMap);
}
