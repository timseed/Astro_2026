#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
using namespace std;

#include <sys/time.h>
#include <unistd.h>

#include "config.h"

#include "buildPlanetMap.h"
#include "findBodyXYZ.h"
#include "keywords.h"
#include "Options.h"
#include "PlanetProperties.h"
#include "readOriginFile.h"
#include "setPositions.h"
#include "xpUtil.h"

#include "libannotate/libannotate.h"
#include "libdisplay/libdisplay.h"
#include "libdisplay/libtimer.h"
#include "libephemeris/ephemerisWrapper.h"
#include "libplanet/Planet.h"
#include "libmultiple/RayleighScattering.h"

extern void
drawMultipleBodies(DisplayBase *display, Planet *target,
                   const double upX, const double upY, const double upZ, 
                   map<double, Planet *> &planetsFromSunMap,
                   PlanetProperties *planetProperties[]);

extern void
drawProjection(DisplayBase *display, Planet *target,
               const double upX, const double upY, const double upZ, 
               map<double, Planet *> &planetsFromSunMap,
               PlanetProperties *planetProperties);

extern void
readConfigFile(string configFile, PlanetProperties *planetProperties[]);

extern void 
setPositions(const vector<LBRPoint> &originVector, 
             const vector<LBRPoint>::iterator &iterOriginVector,
             Planet *&target, map<double, Planet *> &planetMap,
             PlanetProperties *planetProperties[]);

int
main(int argc, char **argv)
{
    if (setlocale(LC_ALL, "") == NULL)
    {
        ostringstream errMsg;
        errMsg << "Warning: setlocale(LC_ALL, \"\") failed! "
               << "Check your LANG environment variable "
               << "(currently ";
        char *lang = getenv("LANG");
        if (lang == NULL)
        {
            errMsg << "NULL";
        }
        else
        {
            errMsg << "\"" << lang << "\"";
        }
        errMsg << "). Setting to \"C\".\n";
        setlocale(LC_ALL, "C");
        cerr << errMsg.str();
    }

    Options *options = Options::getInstance();
    options->parseArgs(argc, argv);

    if (options->Fork())
    {
        pid_t pid = fork();
        switch (pid)
        {
        case 0:
            // This is the child process
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            setsid();
            break;
        case -1:
            xpExit("fork() failed!\n", __FILE__, __LINE__);
            break;
        default:
            // This is the parent process
            if (options->Verbosity() > 1)
            {
                ostringstream msg;
                msg << "Forked child process, PID is " << pid << "\n";
                xpMsg(msg.str(), __FILE__, __LINE__);
            }
            return(EXIT_SUCCESS);
        }
    }

    if (options->RayleighFile().length() > 0)
    {
	RayleighScattering rayleigh(options->RayleighFile());
	rayleigh.createTables();
	return(EXIT_SUCCESS);
    }

    setUpEphemeris();

    PlanetProperties *planetProperties[RANDOM_BODY];
    for (int i = 0; i < RANDOM_BODY; i++)
        planetProperties[i] = new PlanetProperties((body) i);

    // Load the drawing info for each planet
    readConfigFile(options->ConfigFile(), planetProperties);

#ifdef HAVE_CSPICE
    // Load any SPICE kernels
    processSpiceKernels(true);
#endif

    // If an origin file has been specified, read it
    const bool origin_file = !options->OriginFile().empty();
    vector<LBRPoint> originVector;
    if (origin_file) 
    {
        readOriginFile(options->OriginFile(), originVector);
        if (!options->InterpolateOriginFile())
            options->NumTimes(originVector.size());
    }

    vector<LBRPoint>::iterator iterOriginVector = originVector.begin();

    // Initialize the timer
    Timer *timer = getTimer(options->getWait(), options->Hibernate(),
                            options->IdleWait());

    int times_run = 0;

    while (1)
    {
        // Set the time for the next update
        timer->Update();

        // Run any commands specified with -prev_command
        if (!options->PrevCommand().empty())
        {
            if (system(options->PrevCommand().c_str()) != 0)
            {
                ostringstream errStr;
                errStr << "Can't execute " << options->PrevCommand() 
                       << "\n";
                xpWarn(errStr.str(), __FILE__, __LINE__);
            }
        }

        // Load artificial satellite orbital elements
        if (!planetProperties[EARTH]->SatelliteFiles().empty())
            loadSatelliteVector(planetProperties[EARTH]);

        // delete the markerbounds file, since we'll create a new one
        string markerBounds(options->MarkerBounds());
        if (!markerBounds.empty())
            unlinkFile(markerBounds.c_str());

        // Set the time to the current time, if desired
        if (options->UseCurrentTime())
        {
            struct timeval time;
            gettimeofday(&time, NULL);
            
            time_t t  = time.tv_sec;
            int year  = gmtime(static_cast<time_t *> (&t))->tm_year + 1900;
            int month = gmtime(static_cast<time_t *> (&t))->tm_mon + 1;
            int day   = gmtime(static_cast<time_t *> (&t))->tm_mday;
            int hour  = gmtime(static_cast<time_t *> (&t))->tm_hour;
            int min   = gmtime(static_cast<time_t *> (&t))->tm_min;
            int sec   = gmtime(static_cast<time_t *> (&t))->tm_sec;
            const double julianDay = toJulian(year, month, day, 
                                              hour, min, sec);
            options->setTime(julianDay);
        }

        // Calculate the positions of the planets & moons.  The map
        // container sorts on the key, so the bodies will be ordered
        // by heliocentric distance.  This makes calculating shadows
        // easier.
        map<double, Planet *> planetsFromSunMap;
        buildPlanetMap(options->JulianDay(), planetsFromSunMap);

        // set the observer and target XYZ positions
        Planet *target;
        setPositions(originVector, iterOriginVector, target, planetsFromSunMap,
                     planetProperties);

        // Set the "up" vector.  This points to the top of the screen.
        double upX, upY, upZ;
        setUpXYZ(target, planetsFromSunMap, upX, upY, upZ);

        // Initialize display device
        DisplayBase *display = getDisplay(times_run);

        if (options->ProjectionMode() == MULTIPLE)
        {
            drawMultipleBodies(display, target, 
                               upX, upY, upZ, 
                               planetsFromSunMap,
                               planetProperties);
        }
        else
        {
            drawProjection(display, target, 
                           upX, upY, upZ, 
                           planetsFromSunMap,
                           planetProperties[target->Index()]);
        }

        display->renderImage(planetProperties);
        delete display;

        destroyPlanetMap();

        times_run++;

        if (!options->PostCommand().empty())
        {
            if (system(options->PostCommand().c_str()) != 0)
            {
                ostringstream errStr;
                errStr << "Can't execute " << options->PostCommand()
                       << "\n";
                xpWarn(errStr.str(), __FILE__, __LINE__);
            }
        }

        if (options->NumTimes() > 0 && times_run >= options->NumTimes())
            break;

        if (origin_file && !options->InterpolateOriginFile())
        {
            // If we've run through the origin file, break out of the
            // while(1) loop.
            iterOriginVector++;
            if (iterOriginVector == originVector.end()) break;
        }

        if (!options->UseCurrentTime())
        {
            // Set the time to the next update
            options->incrementTime(options->getTimeWarp() 
                                   * options->getWait());
        }

        // Sleep until the next update.  If Sleep() returns false,
        // then quit.
        if (!timer->Sleep()) break;
    }

#ifdef HAVE_CSPICE
    // unload any SPICE kernels
    processSpiceKernels(false);
#endif

    delete timer;

    for (int i = 0; i < RANDOM_BODY; i++) delete planetProperties[i];

    cleanUpEphemeris();

    return(EXIT_SUCCESS);
}
