#include <clocale>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#include "findFile.h"
#include "keywords.h"
#include "Options.h"
#include "parse.h"
#include "PlanetProperties.h"
#include "Satellite.h"
#include "sphericalToPixel.h"
#include "View.h"
#include "xpUtil.h"

#include "drawArc.h"
#include "drawCircle.h"

#include "libannotate/libannotate.h"
#include "libplanet/Planet.h"
#include "libprojection/ProjectionBase.h"

static vector<Satellite> satelliteVector;

bool
calculateSatellitePosition(time_t tv_sec, const int id,
                           double &lat, double &lon, double &rad)
{
    Options *options = Options::getInstance();
    if (options->LightTime())
    {
        double tX, tY, tZ;
        double jd = options->JulianDay();

        Planet earth(jd, EARTH);
        earth.calcHeliocentricEquatorial();
        earth.getPosition(tX, tY, tZ);

        double oX, oY, oZ;
        options->getOrigin(oX, oY, oZ);

        // Now get the position relative to the origin
        double dX = tX - oX;
        double dY = tY - oY;
        double dZ = tZ - oZ;
        double dist = sqrt(dX*dX + dY*dY + dZ*dZ);

        double light_time = dist * AU_to_km / 299792.458;
        tv_sec -= static_cast<time_t> (light_time);
    }

    for (unsigned int i = 0; i < satelliteVector.size(); i++)
    {
        if (id == satelliteVector[i].getID())
        {
            satelliteVector[i].loadTLE();
            satelliteVector[i].getSpherical(tv_sec, lat, lon, rad);
            return(true);
        }
    }

    ostringstream msg;
    msg << "Can't find satellite # " << id << ".\n";
    xpMsg(msg.str(), __FILE__, __LINE__);
    return(false);
}

static void
readSatelliteFile(const char *line, Planet *planet, 
                  View *view, ProjectionBase *projection,
                  PlanetProperties *planetProperties, 
                  multimap<double, Annotation *> &annotationMap)
{
    int i = 0;
    while (isDelimiter(line[i]))
    {
        i++;
        if (static_cast<unsigned int> (i) > strlen(line)) return;
    }
    if (isEndOfLine(line[i])) return;

    Options *options = Options::getInstance();

    unsigned char color[3];
    memcpy(color, planetProperties->MarkerColor(), 3);

    int align = RIGHT;
    vector<double> altcirc;
    string font("");
    int fontSize = -1;
    string image;
    string name("");
    ofstream outputFile;
    Satellite *satellite = NULL;
    int symbolSize = 2;
    double spacing = 0.1;
    bool syntaxError = false;
    string timezone;

    int thickness = planetProperties->ArcThickness();

    int trailType = ORBIT;
    int trailStart = 0;
    int trailEnd = 0;
    int trailInterval = 1;

    bool transparency = false;
    unsigned char transparent_pixel[3];

    while (static_cast<unsigned int> (i) < strlen(line))
    {
        char *returnString = NULL;
        int val = parse(i, line, returnString);

        switch (val)
        {
        case ALIGN:
            if (returnString == NULL) break;
            switch (returnString[0])
            {
            case 'r':
            case 'R':
                align = RIGHT;
                break;
            case 'l':
            case 'L':
                align = LEFT;
                break;
            case 'a':
            case 'A':
                align = ABOVE;
                break;
            case 'b':
            case 'B':
                align = BELOW;
                break;
            case 'c':
            case 'C':
                align = CENTER;
                break;
            default:
                xpWarn("Unrecognized option for align in satellite file\n",
                       __FILE__, __LINE__);
                syntaxError = true;
                break;
            }
            break;
        case CIRCLE:
        {
            checkLocale(LC_NUMERIC, "C");
            double angle;
            sscanf(returnString, "%lf", &angle);
            if (angle < 0) angle *= -1;
            if (angle > 90) angle = 90;
            angle = 90 - angle;
            altcirc.push_back(angle * deg_to_rad);
            checkLocale(LC_NUMERIC, "");
        }
        break;
        case COLOR:
        {
            int r, g, b;
            if (sscanf(returnString, "%d,%d,%d", &r, &g, &b) == 3)
            {
                color[0] = static_cast<unsigned char> (r & 0xff);
                color[1] = static_cast<unsigned char> (g & 0xff);
                color[2] = static_cast<unsigned char> (b & 0xff);
            }
            else
            {
                xpWarn("need three values for color\n", __FILE__, __LINE__);
                syntaxError = true;
            }
        }
        break;
        case FONT:
            font.assign(returnString);
            break;
        case FONTSIZE:
            sscanf(returnString, "%d", &fontSize);
            if (fontSize <= 0)
            {
                xpWarn("fontSize must be positive.\n", __FILE__, __LINE__);
                syntaxError = true;
            }
            break;
        case IMAGE:
            image.assign(returnString);
            break;
        case LATLON:
        {
            int id;
            sscanf(returnString, "%d", &id);
            vector<Satellite>::iterator ii = satelliteVector.begin();
            while (ii != satelliteVector.end())
            {
                if (ii->getID() == id)
                {
                    satellite = &(*ii);
                    if (name.empty()) name.assign(satellite->getName());
                    if (options->Verbosity() > 3)
                    {
                        ostringstream msg;
                        msg << "Found satellite # " << id 
                            << " (" << satellite->getName() << ")\n";
                        xpMsg(msg.str(), __FILE__, __LINE__);
                    }
                    break;
                }
                ii++;
            }
        }
        break;
        case NAME:
            name.assign(returnString);
            break;
        case OUTPUT:
            outputFile.open(returnString, ios::app);
            if (outputFile.fail())
            {
                ostringstream errMsg;
                errMsg << "Can't open satellite output file: " 
                       << returnString << endl;
                xpWarn(errMsg.str(), __FILE__, __LINE__);
            }
            break;
        case SPACING:
            checkLocale(LC_NUMERIC, "C");
            sscanf(returnString, "%lf", &spacing);
            if (spacing < 0) 
            {
                xpWarn("spacing must be positive\n", __FILE__, __LINE__);
                spacing = 0.1;
                syntaxError = true;
            }
            checkLocale(LC_NUMERIC, "");
            break;
        case THICKNESS:
            sscanf(returnString, "%d", &thickness);
            if (thickness < 1)
            {
                xpWarn("thickness must be positive.\n", 
                       __FILE__, __LINE__);
                syntaxError = true;
            }
            break;
        case TRAIL:
        {
            char *ptr = returnString;
            while (ptr[0] != ',') 
            {
                if (ptr[0] == '\0') 
                {
                    syntaxError = true;
                    break;
                }
                ptr++;
            }

            if (syntaxError) break;

            if (!sscanf(++ptr, "%d,%d,%d", &trailStart, &trailEnd,
                        &trailInterval) == 3)
            {
                xpWarn("Need four values for trail{}!\n", 
                       __FILE__, __LINE__);
                syntaxError = true;
            }
            else
            {
                switch (returnString[0])
                {
                case 'g':
                case 'G':
                    trailType = GROUND;
                    break;
                case 'o':
                case 'O':
                    trailType = ORBIT;
                    break;
                default:
                    xpWarn("Unknown type of orbit trail!\n", 
                           __FILE__, __LINE__);
                    syntaxError = true;
                    break;
                }
                if (trailInterval < 1) trailInterval = 1;
            }
        }
        break;
        case TRANSPARENT:
        {
            int r, g, b;
            if (sscanf(returnString, "%d,%d,%d", &r, &g, &b) == 3)
            {
                transparent_pixel[0] = static_cast<unsigned char> (r & 0xff);
                transparent_pixel[1] = static_cast<unsigned char> (g & 0xff);
                transparent_pixel[2] = static_cast<unsigned char> (b & 0xff);
            }
            else
            {
                xpWarn("Need three values for transparency pixel!\n", 
                       __FILE__, __LINE__);
                syntaxError = true;
            }
            transparency = true;
        }
        break;
        case UNKNOWN:
            syntaxError = true;
        default:
        case DELIMITER:
            break;
        }

        if (val != DELIMITER && options->Verbosity() > 3)
        {
            ostringstream msg;
            msg << "value is " << keyWordString[val - '?'];
            if (returnString != NULL)
                msg << ", returnString is " << returnString;
            msg << endl;
            xpMsg(msg.str(), __FILE__, __LINE__);
        }

        delete [] returnString;

        if (syntaxError)
        {
            ostringstream errStr;
            errStr << "Syntax error in satellite file\n";
            errStr << "line is \"" << line << "\"" << endl;
            xpWarn(errStr.str(), __FILE__, __LINE__);
            return;
        }

        if (val == ENDOFLINE) break;
    }

    if (satellite == NULL) 
    {
        ostringstream errStr;
        errStr << "No satellite found for  \"" << line << "\"" << endl;
        xpWarn(errStr.str(), __FILE__, __LINE__);
        return;
    }

    // Load TLE data here since select_ephemeris() in libsgp4sdp4
    // changes it.  This is in case the user wants to have two entries
    // with the same satellite.
    satellite->loadTLE();

    time_t startTime = static_cast<time_t> (options->TVSec() + trailStart * 60);
    time_t endTime = static_cast<time_t> (options->TVSec() + trailEnd * 60);
    time_t interval = static_cast<time_t> (trailInterval * 60);

    if (startTime > endTime)
    {
        time_t tmp = startTime;
        startTime = endTime;
        endTime = tmp;
    }

    double lat, lon, rad;
    satellite->getSpherical(startTime, lat, lon, rad);

    if (outputFile.is_open() && endTime > startTime)
    {
        outputFile << "BEGIN " << satellite->getName() << "\t" 
                   << satellite->getID() << endl;
        char line[128];
        snprintf(line, 128, "%12s%12s%12s%12s%30s\n",
                 "lat", "lon", "alt", "UNIX time", "UTC ");
        outputFile << line;
    }

    for (time_t t = startTime + interval; t <= endTime; t += interval)
    {
        const double prevLat = lat;
        const double prevLon = lon;
        double prevRad = rad;

        satellite->getSpherical(t, lat, lon, rad);

        if (outputFile.is_open()) 
        {
            char line[128];
            snprintf(line, 128, "%12.3f%12.3f%12.3f%12ld%30s",
                     lat/deg_to_rad, lon/deg_to_rad, 6378.14 * (rad - 1),
                     t, asctime(gmtime((time_t *) &t)));
            outputFile << line;
        }

        if (trailType == GROUND)
        {
            rad = 1;
            prevRad = 1;
        }

        drawArc(prevLat, prevLon, prevRad, lat, lon, rad, color, thickness, 
                spacing * deg_to_rad, planetProperties->Magnify(),
                planet, view, projection, annotationMap);
    }

    if (outputFile.is_open())
    {
        if (endTime > startTime)
        {
            outputFile << "END   " << satellite->getName() << "\t" 
                       << satellite->getID() << endl;
        }
        outputFile.close();
    }

    satellite->getSpherical(options->TVSec(), lat, lon, rad);
    if (trailType == GROUND) rad = 1;

    double X, Y, Z;
    if (sphericalToPixel(lat, lon, rad * planetProperties->Magnify(), 
                         X, Y, Z, planet, view, projection))
    {
        const int ix = static_cast<int> (floor(X + 0.5));
        const int iy = static_cast<int> (floor(Y + 0.5));
            
        int xOffset = 0;
        int yOffset = 0;
        if (image.empty())
        {
            Symbol *sym = new Symbol(color, ix, iy, symbolSize);
            annotationMap.insert(pair<const double, Annotation*>(Z, sym));
            xOffset = symbolSize;
            yOffset = symbolSize;
        }
        else if (image.compare("none") != 0)
        {
            unsigned char *transparent = (transparency ? transparent_pixel : NULL);
            Icon *icon = new Icon(ix, iy, image, transparent);
            annotationMap.insert(pair<const double, Annotation*>(Z, icon));
            xOffset = icon->Width() / 2;
            yOffset = icon->Height() / 2;
        }
            
        if (!name.empty())
        {
            Text *t = new Text(color, ix, iy, xOffset, yOffset, align, name);
            if (!font.empty()) t->Font(font);
            if (fontSize > 0) t->FontSize(fontSize);
                
            annotationMap.insert(pair<const double, Annotation*>(Z, t));
        }
    }

    vector<double>::iterator a = altcirc.begin();
    while (a != altcirc.end())
    {
        // Given the angle of the spacecraft above the horizon,
        // compute the great arc distance from the sub-spacecraft
        // point
        const double r = *a - asin(sin(*a)/rad);
        drawCircle(lat, lon, r, color, thickness, spacing * deg_to_rad, 
                   planetProperties->Magnify(), planet, view,
                   projection, annotationMap);
        a++;
    }
}

void
loadSatelliteVector(PlanetProperties *planetProperties)
{
    vector<string> satfiles = planetProperties->SatelliteFiles();
    vector<string>::iterator ii = satfiles.begin();

    satelliteVector.clear();

    while (ii != satfiles.end()) 
    {
        string tleFile = *ii + ".tle";

        const bool foundFile = findFile(tleFile, "satellites");
        if (foundFile)
        {
            ifstream inFile(tleFile.c_str());
            char lines[3][80];
            while (inFile.getline(lines[0], 80) != NULL)
            {
                if ((inFile.getline(lines[1], 80) == NULL) 
                    || (inFile.getline(lines[2], 80) == NULL))
                {
                    ostringstream errStr;
                    errStr << "Malformed TLE file (" << tleFile << ")?\n";
                    xpWarn(errStr.str(), __FILE__, __LINE__);
                    break;
                }
                
                Satellite sat(lines);
                
                if (!sat.isGoodData()) 
                {
                    ostringstream errStr;
                    errStr << "Bad TLE data in " << tleFile << endl;
                    xpWarn(errStr.str(), __FILE__, __LINE__);
                    continue;
                }
                
                satelliteVector.push_back(sat);
            }
            
            inFile.close();
        }
        else
        {
            ostringstream errStr;
            errStr << "Can't load satellite TLE file " << tleFile << endl;
            xpWarn(errStr.str(), __FILE__, __LINE__);
        }
        ii++;
    }
}

void
addSatellites(PlanetProperties *planetProperties, Planet *planet, 
              View *view, ProjectionBase *projection, 
              multimap<double, Annotation *> &annotationMap)
{
    if (planet->Index() != EARTH) return;

    vector<string> satfiles = planetProperties->SatelliteFiles();
    vector<string>::iterator ii = satfiles.begin();

    while (ii != satfiles.end()) 
    {
        string satFile(*ii);
        bool foundFile = findFile(satFile, "satellites");
        if (foundFile)
        {
            ifstream inFile(satFile.c_str());
            char *line = new char[MAX_LINE_LENGTH];
            while (inFile.getline (line, MAX_LINE_LENGTH, '\n') != NULL)
                readSatelliteFile(line, planet, view, projection,
                                  planetProperties, annotationMap);
            
            inFile.close();
            delete [] line;
        }
        else
        {
            ostringstream errStr;
            errStr << "Can't load satellite file " << satFile << endl;
            xpWarn(errStr.str(), __FILE__, __LINE__);
        }
        ii++;
    }
}
