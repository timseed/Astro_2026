#include <clocale>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
using namespace std;

#include "findFile.h"
#include "keywords.h"
#include "Options.h"
#include "parse.h"
#include "PlanetProperties.h"
#include "sphericalToPixel.h"
#include "xpUtil.h"

#include "drawArc.h"

#include "libannotate/libannotate.h"
#include "libplanet/Planet.h"
#include "libprojection/ProjectionBase.h"

class View;

static void
readArcFile(const char *line, Planet *planet, 
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
    double magnify;
    int thickness;
    if (planetProperties != NULL)
    {
        memcpy(color, planetProperties->ArcColor(), 3);
        magnify = planetProperties->Magnify();
        thickness = planetProperties->ArcThickness();
    }
    else
    {
        memset(color, 128, 3);
        magnify = 1.0;
        thickness = options->ArcThickness();
    }

    double coords[4];
    int numCoords = 0;
    int numRad = 0;
    double radius[2] = { -1, -1 };
    double spacing = options->ArcSpacing();
    bool syntaxError = false;

    while (static_cast<unsigned int> (i) < strlen(line))
    {
        char *returnString = NULL;
        int val = parse(i, line, returnString);

        switch (val)
        {
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
                xpWarn("Need three values for color\n", __FILE__, __LINE__);
                syntaxError = true;
            }
        }
        break;
        case LATLON:
            checkLocale(LC_NUMERIC, "C");
            if (numCoords < 4)
            {
                sscanf(returnString, "%lf", &coords[numCoords]);
                if (numCoords % 2 == 0)
                {
                    if (coords[numCoords] < -90 || coords[numCoords] > 90)
                    {
                        ostringstream errMsg;
                        errMsg << "Latitude value must be between -90 "
                               << "and 90 degrees\n";
                        xpWarn(errMsg.str(), __FILE__, __LINE__);
                        syntaxError = true;
                    }
                }
                else 
                {
                    if (coords[numCoords] < -360 || coords[numCoords] > 360)
                    {
                        ostringstream errMsg;
                        errMsg << "Longitude value must be between -360 "
                               << "and 360 degrees\n";
                        xpWarn(errMsg.str(), __FILE__, __LINE__);
                        syntaxError = true;
                    }
                }
                coords[numCoords] *= deg_to_rad;
                numCoords++;
            }
            else
            {
                syntaxError = true;
            }
            checkLocale(LC_NUMERIC, "");
            break;
        case RADIUS:
            checkLocale(LC_NUMERIC, "C");
            if (numRad < 2)
            {
                sscanf(returnString, "%lf", &radius[numRad]);
                if (radius[numRad] < 0) 
                {
                    xpWarn("Radius value must be positive\n",
                           __FILE__, __LINE__);
                    radius[numRad] = -1;
                    syntaxError = true;
                }
                else
                {
                    numRad++;
                }
            }
            checkLocale(LC_NUMERIC, "");
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
            errStr << "Syntax error in arc file\n"
                   << "line is \"" << line << "\"\n";
            xpWarn(errStr.str(), __FILE__, __LINE__);
            return;
        }

        if (val == ENDOFLINE) break;
    }
    
    if (numCoords != 4)
    {
        ostringstream errStr;
        errStr << "Incomplete entry in arc file\n"
               << "line is \"" << line << "\"\n";
        xpWarn(errStr.str(), __FILE__, __LINE__);
        return;
    }

    if (numRad == 0) radius[1] = radius[0];

    for (i = 0; i < 2; i++)
    {
        if (radius[i] < 0)
        {
            if (planet != NULL)
            {
                radius[i] = planet->Radius(coords[2*i]);
            }
            else
            {
                radius[i] = 1;
            }
        }
    }

    if (planet == NULL)
    {
        double X1, Y1, Z1;
        double X2, Y2, Z2;
        sphericalToPixel(coords[0], coords[1], 1.0,
                         X1, Y1, Z1, NULL, view, NULL);
        sphericalToPixel(coords[2], coords[3], 1.0,
                         X2, Y2, Z2, NULL, view, NULL);

        LineSegment *ls = new LineSegment(color, thickness, X2, Y2, X1, Y1);
        double avgZ = 0.5 * (Z1 + Z2);
        if (Z1 > 0 && Z2 > 0)
            annotationMap.insert(pair<const double, Annotation*>(avgZ, ls));
    }
    else
    {
        drawArc(coords[0], coords[1], radius[0], coords[2], coords[3], 
                radius[1], color, thickness, spacing * deg_to_rad,
                magnify, planet, view, 
                projection, annotationMap);
    }
}

// read an arc file to be plotted on a planet
void
addArcs(PlanetProperties *planetProperties, Planet *planet, 
        View *view, ProjectionBase *projection, 
        multimap<double, Annotation *> &annotationMap)
{
    vector<string> arcfiles = planetProperties->ArcFiles();
    vector<string>::iterator ii = arcfiles.begin();

    while (ii != arcfiles.end()) 
    {
        string arcFile(*ii);
        bool foundFile = findFile(arcFile, "arcs");
        if (foundFile)
        {
            ifstream inFile(arcFile.c_str());
            char *line = new char[MAX_LINE_LENGTH];
            while (inFile.getline (line, MAX_LINE_LENGTH, '\n') != NULL)
                readArcFile(line, planet, view, projection,
                            planetProperties, annotationMap);
            
            inFile.close();
            delete [] line;
        }
        else
        {
            ostringstream errStr;
            errStr << "Can't load arc file " << arcFile << endl;
            xpWarn(errStr.str(), __FILE__, __LINE__);
        }
        ii++;
    }
}

// Read an arc file to be plotted against the background stars (like
// constellation lines)
void
addArcs(View *view, multimap<double, Annotation *> &annotationMap)
{
    Options *options = Options::getInstance();
    vector<string> arcfiles = options->ArcFiles();
    vector<string>::iterator ii = arcfiles.begin();

    while (ii != arcfiles.end()) 
    {
        string arcFile(*ii);
        bool foundFile = findFile(arcFile, "arcs");
        if (foundFile)
        {
            ifstream inFile(arcFile.c_str());
            char *line = new char[256];
            while (inFile.getline (line, 256, '\n') != NULL)
                readArcFile(line, NULL, view, NULL, NULL, annotationMap);

            inFile.close();
            delete [] line;
        }
        else
        {
            ostringstream errStr;
            errStr << "Can't load arc file " << arcFile << endl;
            xpWarn(errStr.str(), __FILE__, __LINE__);
        }
        ii++;
    }
}
