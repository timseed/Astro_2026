#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#include "buildPlanetMap.h"
#include "findFile.h"
#include "keywords.h"
#include "Options.h"
#include "parse.h"
#include "PlanetProperties.h"
#include "sphericalToPixel.h"
#include "View.h"
#include "xpUtil.h"

#include "libannotate/libannotate.h"
#include "libplanet/Planet.h"
#include "libprojection/ProjectionBase.h"

static void
readMarkerFile(const char *line, Planet *planet, const double pR, 
               const double pX, const double pY, const double pZ,
               View *view, ProjectionBase *projection,
               const int width, const int height, unsigned char *color, 
               string &font, int fontSize, const double magnify,
               map<double, Planet *> &planetsFromSunMap, 
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

    int align = AUTO;
    bool haveLat = false;
    bool haveLon = false;
    double lat, lon;
    string image;

    string lang("");
    string name("");
    double opacity = 1.0;
    bool outlined = true;
    bool pixelCoords = false;
    double radius = -1;
    bool relativeToEdges = true;
    int symbolSize = 2;
    bool syntaxError = false;
    string timezone;
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
                xpWarn("Unrecognized option for align in marker file\n",
                       __FILE__, __LINE__);
                syntaxError = true;
                break;
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
                xpWarn("Need three values for color\n", __FILE__, __LINE__);
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
                xpWarn("fontSize must be positive.\n", 
                       __FILE__, __LINE__);
                syntaxError = true;
            }
            break;
        case IMAGE:
            image.assign(returnString);
            break;
        case LANGUAGE:
            lang.assign(returnString);
            break;
        case LATLON:
            checkLocale(LC_NUMERIC, "C");
            if (haveLon)
            {
                syntaxError = true;
            }
            else if (haveLat)
            {
                sscanf(returnString, "%lf", &lon);
                haveLon = true;
            }
            else
            {
                sscanf(returnString, "%lf", &lat);
                haveLat = true;
            }
            checkLocale(LC_NUMERIC, "");
            break;
        case MAX_RAD_FOR_MARKERS:
        {
            double maxRad;
            sscanf(returnString, "%lf", &maxRad);
            maxRad *= height;
            if (pR > 0 && pR > maxRad) return;
        }
        break;
        case MIN_RAD_FOR_MARKERS:
        {
            double minRad;
            sscanf(returnString, "%lf", &minRad);
            minRad *= height;
            if (pR > 0 && pR < minRad) return;
        }
        break;
        case NAME:
            name.assign(returnString);
            break;
        case OPACITY:
        {
            int s;
            sscanf(returnString, "%d", &s);
            if (s < 0) 
                s = 0;
            else if (s > 100) 
                s = 100;
            opacity = s/100.;
        }
        break;
        case OUTLINED:
            if (strncmp(returnString, "f", 1) == 0
                || strncmp(returnString, "F", 1) == 0)
                outlined = false;
            break;
        case POSITION:
            if (strncmp(returnString, "pixel", 2) == 0)
            {
                pixelCoords = true;
            }
            else if (strncmp(returnString, "absolute", 3) == 0)
            {
                pixelCoords = true;
                relativeToEdges = false;
            }
            else
            {
                if (planet != NULL)
                {
                    body pIndex = Planet::parseBodyName(returnString);
                    
                    if (pIndex != planet->Index())
                    {
                        const Planet *other = findPlanetinMap(planetsFromSunMap, 
                                                              pIndex);
                        double X, Y, Z;
                        other->getPosition(X, Y, Z);
                        planet->XYZToPlanetographic(X, Y, Z, lat, lon);
                        
                        lat /= deg_to_rad;
                        lon /= deg_to_rad;
                    }
                }
            }
            break;
        case RADIUS:
            checkLocale(LC_NUMERIC, "C");
            sscanf(returnString, "%lf", &radius);
            if (radius < 0) 
            {
                xpWarn("Radius value must be positive\n",
                       __FILE__, __LINE__);
                radius = -1;
                syntaxError = true;
            }
            checkLocale(LC_NUMERIC, "");
            break;
        case SYMBOLSIZE:
            sscanf(returnString, "%d", &symbolSize);
            if (symbolSize < 0) symbolSize = 2;
            break;
        case TIMEZONE:
            timezone.assign(returnString);
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
            errStr << "Syntax error in marker file\n"
                   << "line is \"" << line << "\"\n";
            xpWarn(errStr.str(), __FILE__, __LINE__);
            return;
        }

        if (val == ENDOFLINE) break;
    }

    double X, Y, Z = 0;
    bool markerVisible = false;

    if (pixelCoords)
    {   
        X = lon;
        Y = lat;

        if (relativeToEdges)
        {
            if (X < 0) X += width;
            if (Y < 0) Y += height;
        }
    }
    else
    {
        lat *= deg_to_rad;
        lon *= deg_to_rad;

        if (radius < 0)
        {
            if (planet != NULL)
            {
                radius = planet->Radius(lat);
            }
            else
            {
                radius = 1;
            }
        }

        markerVisible = sphericalToPixel(lat, lon, radius * magnify, 
                                         X, Y, Z, planet, view, projection);

        // don't draw markers on the far side of the planet
        if (planet != NULL 
            && view != NULL 
            && Z > pZ
            && sqrt((X-pX)*(X-pX) + (Y-pY)*(Y-pY))/pR < radius * magnify) 
            markerVisible = false;
    }

    if (pixelCoords || markerVisible)
    {
        const int ix = static_cast<int> (floor(X + 0.5));
        const int iy = static_cast<int> (floor(Y + 0.5));

        int iconWidth = 0;
        int iconHeight = 0;
        if (image.empty())
        {
            Symbol *s = new Symbol(color, ix, iy, symbolSize);
            annotationMap.insert(pair<const double, Annotation*>(Z, s));
            iconWidth = symbolSize * 2;
            iconHeight = symbolSize * 2;
        }
        else if (image.compare("none") != 0)
        {
            unsigned char *transparent = (transparency ? transparent_pixel : NULL);
            Icon *i = new Icon(ix, iy, image, transparent);
            annotationMap.insert(pair<const double, Annotation*>(Z, i));
            iconWidth = i->Width();
            iconHeight = i->Height();
        }

        // if the name string has time formatting codes, and the
        // timezone is defined, run the name string through strftime()
        if (name.find("%") != string::npos && !timezone.empty())
        {
            const char *tzEnv = getenv("TZ");
            string tzSave;
            if (tzEnv != NULL)
            {
                tzSave = "TZ=";
                tzSave += tzEnv;
            }

            string tz = "TZ=";
            tz += timezone;
            putenv((char *) tz.c_str());

            tzset();

            if (!lang.empty())
                checkLocale(LC_ALL, lang.c_str());

            // run name string through strftime() and convert to UTF-8
            strftimeUTF8(name);

            if (tzEnv == NULL) 
                removeFromEnvironment("TZ"); 
            else
                putenv((char *) tzSave.c_str()); 

            tzset();

            if (!lang.empty())
                checkLocale(LC_ALL, "");
        }

        if (!name.empty())
        {
            Text *t = new Text(color, ix, iy, 
                               iconWidth, iconHeight, 
                               align, name);

            t->Opacity(opacity);
            t->Outline(outlined);

            if (!font.empty()) t->Font(font);
            if (fontSize > 0) t->FontSize(fontSize);
            
            annotationMap.insert(pair<const double, Annotation*>(Z, t));
        }
    }
}

// Used for labeling planets/moons
void
addMarkers(PlanetProperties *planetProperties, Planet *planet, 
           const double pixel_radius,
           const double X, const double Y, const double Z,
           View *view, ProjectionBase *projection, 
           const int width, const int height, 
           map<double, Planet *> &planetsFromSunMap, 
           multimap<double, Annotation *> &annotationMap)
{
    vector<string> markerfiles = planetProperties->MarkerFiles();
    vector<string>::iterator ii = markerfiles.begin();

    while (ii != markerfiles.end()) 
    {
        string markerFile(*ii);
        bool foundFile = findFile(markerFile, "markers");
        if (foundFile)
        {
            ifstream inFile(markerFile.c_str());
            char *line = new char[MAX_LINE_LENGTH];
            while (inFile.getline (line, MAX_LINE_LENGTH, '\n') != NULL)
            {
                unsigned char color[3];
                memcpy(color, planetProperties->MarkerColor(), 3);
                string font(planetProperties->MarkerFont());
                int fontSize(planetProperties->MarkerFontSize());
                
                readMarkerFile(line, planet, pixel_radius, X, Y, Z, 
                               view, projection, width, height, 
                               color, font, fontSize, 
                               planetProperties->Magnify(),
                               planetsFromSunMap, annotationMap);
            }
            
            inFile.close();
            delete [] line;
        }
        else
        {
            ostringstream errStr;
            errStr << "Can't load marker file " << markerFile << endl;
            xpWarn(errStr.str(), __FILE__, __LINE__);
        }
        ii++;
    }
}

// Used for labeling star fields
void
addMarkers(View *view, const int width, const int height, 
           map<double, Planet *> &planetsFromSunMap, 
           multimap<double, Annotation *> &annotationMap)
{
    Options *options = Options::getInstance();

    vector<string> markerfiles = options->MarkerFiles();
    vector<string>::iterator ii = markerfiles.begin();

    while (ii != markerfiles.end()) 
    {
        string markerFile(*ii);
        bool foundFile = findFile(markerFile, "markers");
        if (foundFile)
        {
            ifstream inFile(markerFile.c_str());
            char *line = new char[MAX_LINE_LENGTH];
            while (inFile.getline (line, MAX_LINE_LENGTH, '\n') != NULL)
            {
                unsigned char color[3];
                memcpy(color, options->Color(), 3);
                string font(options->Font());
                int fontSize(options->FontSize());
                
                readMarkerFile(line, NULL, 0, 0, 0, 0,
                               view, NULL, width, height, 
                               color, font, fontSize, 1.0, 
                               planetsFromSunMap, annotationMap);
            }            
            inFile.close();
            delete [] line;
        }
        else
        {
            ostringstream errStr;
            errStr << "Can't load marker file " << markerFile << endl;
            xpWarn(errStr.str(), __FILE__, __LINE__);
        }
        ii++;
    }
}
