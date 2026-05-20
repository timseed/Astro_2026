#include <sstream>
using namespace std;

#include "config.h"

#include "body.h"
#include "findFile.h"
#include "Options.h"
#include "xpUtil.h"

#include "EphemerisHigh.h"
#include "EphemerisLow.h"
#ifdef HAVE_CSPICE
#include "EphemerisSpice.h"
#endif
#include "libmoons/libmoons.h"

static Ephemeris *ephemHigh  = NULL;
static Ephemeris *ephemLow   = NULL;
static Ephemeris *ephemSpice = NULL;

void
setUpEphemeris()
{
    Options *options = Options::getInstance();
    string ephemerisFile(options->JPLFile());
    if (!ephemerisFile.empty())
    {
        bool foundFile = findFile(ephemerisFile, "ephemeris");
        if (foundFile)
        {
            ephemHigh = new EphemerisHigh(ephemerisFile.c_str());
        }
        else
        {
            ostringstream errStr;
            errStr << "Can't load ephemeris file " 
                   << ephemerisFile << "\n";
            xpExit(errStr.str(), __FILE__, __LINE__);
        }
    }
    else
    {
        ephemLow = new EphemerisLow();
    }

#ifdef HAVE_CSPICE
    ephemSpice = new EphemerisSpice();
#endif
}

void
cleanUpEphemeris()
{
    delete ephemHigh;
    delete ephemLow;
    delete ephemSpice;
    ephemHigh  = NULL;
    ephemLow   = NULL;
    ephemSpice = NULL;
}

void
GetHeliocentricXYZ(const body index, const body primary, 
                   const double julianDay, const bool relativeToSun, 
                   double &X, double &Y, double &Z)
{
#ifdef HAVE_CSPICE
    Options *options = Options::getInstance();
    vector<int> spiceList = options->SpiceEphemeris();
    for (unsigned int i = 0; i < spiceList.size(); i++)
    {
        if (naif_id[index] == spiceList[i])
        {
            ephemSpice->GetHeliocentricXYZ(index, julianDay, X, Y, Z);
            if (!relativeToSun && primary != SUN)
            {
                double Prx, Pry, Prz;
                GetHeliocentricXYZ(primary, SUN, julianDay,
                                   true, Prx, Pry, Prz);
                X -= Prx;
                Y -= Pry;
                Z -= Prz;
            }
            return;
        }
    }
#endif

    Ephemeris *thisEphem = NULL;
    if (ephemHigh != NULL)
        thisEphem = ephemHigh;
    else
        thisEphem = ephemLow;

    if (primary == SUN)
    {
        thisEphem->GetHeliocentricXYZ(index, julianDay, X, Y, Z);
    }
    else if (primary == EARTH && thisEphem == ephemHigh)
    {
        // Lunar ephemeris is part of JPL's Digital Ephemeris
        thisEphem->GetHeliocentricXYZ(index, julianDay, X, Y, Z);
        if (!relativeToSun)
        {
            double Prx, Pry, Prz;
            GetHeliocentricXYZ(EARTH, SUN, julianDay,
                               true, Prx, Pry, Prz);
            X -= Prx;
            Y -= Pry;
            Z -= Prz;
        }
    }
    else
    {
        switch(primary)
        {
        case EARTH:
            moon(julianDay, X, Y, Z);
            break;
        case MARS:
            marsat(julianDay, index, X, Y, Z);
            break;
        case JUPITER:
            jupsat(julianDay, index, X, Y, Z);
            break;
        case SATURN:
            satsat(julianDay, index, X, Y, Z);
            break;
        case URANUS:
            urasat(julianDay, index, X, Y, Z);
            break;
        case NEPTUNE:
            nepsat(julianDay, index, X, Y, Z);
            break;
        case PLUTO:
            plusat(julianDay, X, Y, Z);
            break;
        default:
            break;
        }
        if (relativeToSun)
        {
            double Prx, Pry, Prz;
            GetHeliocentricXYZ(primary, SUN, julianDay,
                               true, Prx, Pry, Prz);
            X += Prx;
            Y += Pry;
            Z += Prz;
        }
    }
}
