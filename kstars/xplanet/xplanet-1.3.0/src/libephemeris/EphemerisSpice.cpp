#include <SpiceUsr.h>

#include "xpUtil.h"
#include "EphemerisSpice.h"
using namespace std;

EphemerisSpice::EphemerisSpice() : Ephemeris()
{
}

EphemerisSpice::~EphemerisSpice()
{
}

void
EphemerisSpice::GetHeliocentricXYZ(const body b, const double tjd, 
                                   double &Px, double &Py, double &Pz)
{
    // seconds past J2000
    const SpiceDouble et = ((tjd - 2451545.0) * 86400);
    SpiceDouble pos[3];
    SpiceDouble lt;

    SpiceInt SUN = 10;
    SpiceInt target = naif_id[static_cast<int> (b)];

    spkgps_c(target, et, "J2000", SUN, pos, &lt);
    if (return_c()) // true if spkgps_c fails
    {
        reset_c();  // reset the SPICE error flags
        Px = 0;
        Py = 0;
        Pz = 0;
        return;
    }

    // convert from km to AU
    for (int i = 0; i < 3; i++) pos[i] /= AU_to_km;

    Px = pos[0];
    Py = pos[1];
    Pz = pos[2];
}
