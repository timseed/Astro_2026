#include <clocale>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

#include "body.h"
#include "findFile.h"
#include "keywords.h"
#include "Options.h"
#include "parse.h"
#include "PlanetProperties.h"
#include "xpDefines.h"
#include "xpUtil.h"

#include "libplanet/Planet.h"

static PlanetProperties *defaultProperties;
static PlanetProperties *currentProperties;

static void
readConfig(const char *line, PlanetProperties *planetProperties[])
{
    int i = 0;
    while (isDelimiter(line[i]))
    {
        i++;
        if (static_cast<unsigned int> (i) > strlen(line)) return;
    }
    if (isEndOfLine(line[i])) return;

    Options *options = Options::getInstance();

    while (static_cast<unsigned int> (i) < strlen(line))
    {
        char *returnString = NULL;
        int val = parse(i, line, returnString);

        if (val != BODY && currentProperties == NULL)
            xpExit("No Planet defined in config file!\n", 
                   __FILE__, __LINE__);

        switch (val)
        {
        case ARC_COLOR:
        {
            int r, g, b;
            if (sscanf(returnString, "%d,%d,%d", &r, &g, &b) == 3)
            {
                unsigned char color[3] = { r & 0xff, g & 0xff, b & 0xff };
                currentProperties->ArcColor(color);
            }
            else
            {
                xpWarn("Need three values for arc_color\n", 
                       __FILE__, __LINE__);
            }
        }
        break;
        case ARC_FILE:
            currentProperties->AddArcFile(returnString);
            break;
        case BODY:
        {
            body index = Planet::parseBodyName(returnString);
            if (index < RANDOM_BODY)
            {
                // [default] should come first in the config file.
                // The first time we get to a body after [default],
                // set all of the planet properties to the default
                // value.  If [default] isn't first, all of the bodies
                // specified before it get wiped out.
                if (currentProperties == defaultProperties)
                {
                    for (int ibody = SUN; ibody < RANDOM_BODY; ibody++)
                    {
                        *planetProperties[ibody] = *defaultProperties;
                    }
                }

                currentProperties = planetProperties[index];
            }
            else if (index == DEFAULT)
            {
                // We really shouldn't have to do this, since
                // currentProperties is set to defaultProperties
                // before the file is read in, and [default] should
                // come at the top.
                currentProperties = defaultProperties;
            }
            else
            {
                xpExit("Unknown body in config file\n", __FILE__, __LINE__);
            }
        }
        break;
        case BUMP_MAP:
        {
            // This is one that doesn't belong in [default]
            if (currentProperties == defaultProperties)
            {
                ostringstream errStr;
                errStr << "bump_map specified in [default] section.  "
                       << "You probably want to put it in a specific "
                       << "planet's section (like [earth]).\n";
                xpWarn(errStr.str(), __FILE__, __LINE__);
            }
            string bumpMap(returnString);
            if (bumpMap.find('.') == string::npos)
                bumpMap += defaultMapExt;
            currentProperties->BumpMap(bumpMap);
        }
        break;
        case BUMP_SCALE:
        {
            checkLocale(LC_NUMERIC, "C");
            double bumpScale;
            sscanf(returnString, "%lf", &bumpScale);
            currentProperties->BumpScale(bumpScale);
            checkLocale(LC_NUMERIC, "");
        }
        break;
        case BUMP_SHADE:
        {
            int s;
            sscanf(returnString, "%d", &s);
            if (s < 0) 
                s = 0;
            else if (s > 100) 
                s = 100;
            currentProperties->BumpShade(s/100.);
        }
        break;
        case CLOUD_GAMMA:
        {
            checkLocale(LC_NUMERIC, "C");
            double cloudGamma;
            sscanf(returnString, "%lf", &cloudGamma);
            currentProperties->CloudGamma(cloudGamma);
            checkLocale(LC_NUMERIC, "");
        }
        break;
        case CLOUD_MAP:
        {
            // This is one that doesn't belong in [default]
            if (currentProperties == defaultProperties)
            {
                ostringstream errStr;
                errStr << "cloud_map specified in [default] section.  "
                       << "You probably want to put it in a specific "
                       << "planet's section (like [earth]).\n";
                xpWarn(errStr.str(), __FILE__, __LINE__);
            }
            string cloudMap(returnString);
            if (cloudMap.find('.') == string::npos)
                cloudMap += defaultMapExt;
            currentProperties->CloudMap(cloudMap);
        }
        break;
        case CLOUD_SSEC:
          currentProperties->SSECMap(returnString[0] == 't' 
                                     || returnString[0] == 'T');
          break;
        case CLOUD_THRESHOLD:
        {
            int t;
            sscanf(returnString, "%d", &t);
            if (t < 0) 
                t = 0;
            else if (t > 255) 
                t = 255;
            currentProperties->CloudThreshold(t);
        }
        break;
        case COLOR:
        {
            int r, g, b;
            if (sscanf(returnString, "%d,%d,%d", &r, &g, &b) == 3)
            {
                unsigned char color[3] = { r & 0xff, g & 0xff, b & 0xff };
                currentProperties->Color(color);
            }
            else
            {
                xpWarn("Need three values for color\n", 
                       __FILE__, __LINE__);
            }
        }
        break;
        case DAY_MAP:
        case IMAGE:
            // This is one that doesn't belong in [default]
            if (currentProperties == defaultProperties)
            {
                ostringstream errStr;
                errStr << "image or map specified in [default] section.  "
                       << "You probably want to put it in a specific "
                       << "planet's section (like [earth]).\n";
                xpWarn(errStr.str(), __FILE__, __LINE__);
            }
            currentProperties->DayMap(returnString);
            break;
        case DELIMITER:
            break;
        case DRAW_ORBIT:
        {
            bool drawOrbit = (returnString[0] == 't' 
                              || returnString[0] == 'T');
            currentProperties->DrawOrbit(drawOrbit);
        }
        break;
        case ENDOFLINE:
            break;
        case GRID:
        {
            bool grid = (returnString[0] == 't' 
                         || returnString[0] == 'T');
            currentProperties->Grid(grid);
        }
        break;
        case GRID1:
        {
            int grid1;
            sscanf(returnString, "%d", &grid1);
            if (grid1 < 0 || grid1 > 90) 
                xpExit("grid1 must be between 0 and 90\n", 
                       __FILE__, __LINE__);
            currentProperties->Grid1(grid1);
        }
        break;
        case GRID2:
        {
            int grid2;
            sscanf(returnString, "%d", &grid2);
            if (grid2 < 0)
                xpExit("grid2 must be positive\n", __FILE__, __LINE__);
            currentProperties->Grid2(grid2);
        }
        break;
        case GRID_COLOR:
        {
            int r, g, b;
            if (sscanf(returnString, "%d,%d,%d", &r, &g, &b) == 3)
            {
                unsigned char color[3] = { r & 0xff, g & 0xff, b & 0xff };
                currentProperties->GridColor(color);
            }
            else
            {
                xpWarn("Need three values for grid_color\n", 
                       __FILE__, __LINE__);
            }
        }
        break;
        case MAGNIFY:
        {
            checkLocale(LC_NUMERIC, "C");
            double value;
            sscanf(returnString, "%lf", &value);
            if (value < 0) value = 1;
            currentProperties->Magnify(value);
            checkLocale(LC_NUMERIC, "");
        }
        break;
        case MAP_BOUNDS:
        {
            // This is one that doesn't belong in [default]
            if (currentProperties == defaultProperties)
            {
                ostringstream errStr;
                errStr << "map_bounds specified in [default] section.  "
                       << "You probably want to put it in a specific "
                       << "planet's section (like [earth]).\n";
                xpWarn(errStr.str(), __FILE__, __LINE__);
            }
            checkLocale(LC_NUMERIC, "C");
            double uly, ulx, lry, lrx;
            int numRead = sscanf(returnString, "%lf,%lf,%lf,%lf", 
                                 &uly, &ulx, &lry, &lrx);
            if (numRead == 4)
            {
                currentProperties->MapBounds(true, uly, ulx, lry, lrx);
            }
            else
            {
                xpWarn("Need four values for mapbounds\n",
                       __FILE__, __LINE__);
            }
            checkLocale(LC_NUMERIC, "");
        }
        break;
        case MARKER_COLOR:
        {
            int r, g, b;
            if (sscanf(returnString, "%d,%d,%d", &r, &g, &b) == 3)
            {
                unsigned char color[3] = { r & 0xff, g & 0xff, b & 0xff };
                currentProperties->MarkerColor(color);
            }
            else
            {
                xpWarn("Need three values for marker_color\n", 
                       __FILE__, __LINE__);
            }
        }
        break;
        case MARKER_FILE:
            currentProperties->AddMarkerFile(returnString);
            break;
        case MARKER_FONT:
            currentProperties->MarkerFont(returnString);
            break;
        case MARKER_FONTSIZE:
        { 
            int fontSize;
            sscanf(returnString, "%d", &fontSize);
            if (fontSize > 0)
            {
                currentProperties->MarkerFontSize(fontSize);
            }
            else
            {
                xpWarn("fontSize must be positive.\n", 
                       __FILE__, __LINE__);
            }
        }
        break;
        case MAX_RAD_FOR_LABEL:
        {
            checkLocale(LC_NUMERIC, "C");
            double value;
            sscanf(returnString, "%lf", &value);
            currentProperties->MaxRadiusForLabel(value);
            checkLocale(LC_NUMERIC, "");
        }
        break;
        case MIN_RAD_FOR_LABEL:
        {
            checkLocale(LC_NUMERIC, "C");
            double value;
            sscanf(returnString, "%lf", &value);
            currentProperties->MinRadiusForLabel(value);
            checkLocale(LC_NUMERIC, "");
        }
        break;
        case MIN_RAD_FOR_MARKERS:
        {
            checkLocale(LC_NUMERIC, "C");
            double value;
            sscanf(returnString, "%lf", &value);
            currentProperties->MinRadiusForMarkers(value);
            checkLocale(LC_NUMERIC, "");
        }
        break;
        case NAME:
            // This is one that doesn't belong in [default]
            if (currentProperties == defaultProperties)
            {
                ostringstream errStr;
                errStr << "name specified in [default] section.  "
                       << "You probably want to put it in a specific "
                       << "planet's section (like [earth]).\n";
                xpWarn(errStr.str(), __FILE__, __LINE__);
            }
            currentProperties->Name(returnString);
            break;
        case NIGHT_MAP:
            // This is one that doesn't belong in [default]
            if (currentProperties == defaultProperties)
            {
                ostringstream errStr;
                errStr << "night_map specified in [default] section.  "
                       << "You probably want to put it in a specific "
                       << "planet's section (like [earth]).\n";
                xpWarn(errStr.str(), __FILE__, __LINE__);
            }
            currentProperties->NightMap(returnString);
            break;
        case ORBIT:
        {
            checkLocale(LC_NUMERIC, "C");
            double start, stop, delta;
            int numRead = sscanf(returnString, "%lf,%lf,%lf", 
                                 &start, &stop, &delta);
            if (numRead == 3)
            {
                currentProperties->StartOrbit(start);
                currentProperties->StopOrbit(stop);
                currentProperties->DelOrbit(delta);
            }
            else
            {
                xpWarn("Need three values for orbit\n", 
                       __FILE__, __LINE__);
            }
            checkLocale(LC_NUMERIC, "");
        }
        break;
        case ORBIT_COLOR:
        {
            int r, g, b;
            if (sscanf(returnString, "%d,%d,%d", &r, &g, &b) == 3)
            {
                unsigned char color[3] = { r & 0xff, g & 0xff, b & 0xff };
                currentProperties->OrbitColor(color);
            }
            else
            {
                xpWarn("Need three values for orbit_color\n", 
                       __FILE__, __LINE__);
            }
        }
        break;
        case RANDOM_ORIGIN:
            currentProperties->RandomOrigin(returnString[0] == 't' 
                                            || returnString[0] == 'T');
            break;
        case RANDOM_TARGET:
            currentProperties->RandomTarget(returnString[0] == 't' 
                                            || returnString[0] == 'T');
            break;
        case RAYLEIGH_EMISSION_WEIGHT:
        {
            checkLocale(LC_NUMERIC, "C");
            double scale;
            sscanf(returnString, "%lf", &scale);
            currentProperties->RayleighEmissionWeight(scale);
            checkLocale(LC_NUMERIC, "");
        }
        break;
        case RAYLEIGH_FILE:
            currentProperties->RayleighFile(returnString);
            break;
        case RAYLEIGH_LIMB_SCALE:
        {
            checkLocale(LC_NUMERIC, "C");
            double scale;
            sscanf(returnString, "%lf", &scale);
            currentProperties->RayleighLimbScale(scale);
            checkLocale(LC_NUMERIC, "");
        }
        break;
        case RAYLEIGH_SCALE:
        {
            checkLocale(LC_NUMERIC, "C");
            double scale;
            sscanf(returnString, "%lf", &scale);
            currentProperties->RayleighScale(scale);
            checkLocale(LC_NUMERIC, "");
        }
        break;
        case SATELLITE_FILE:
            currentProperties->AddSatelliteFile(returnString);
            break;
        case SHADE:
        {
            int s;
            sscanf(returnString, "%d", &s);
            if (s < 0) 
                s = 0;
            else if (s > 100) 
                s = 100;
            currentProperties->Shade(s/100.);
        }
        break;
        case SPECULAR_MAP:
            currentProperties->SpecularMap(returnString);
            break;
        case TEXT_COLOR:
        {
            int r, g, b;
            if (sscanf(returnString, "%d,%d,%d", &r, &g, &b) == 3)
            {
                unsigned char color[3] = { r & 0xff, g & 0xff, b & 0xff };
                currentProperties->TextColor(color);
            }
            else
            {
                xpWarn("Need three values for text_color\n", 
                       __FILE__, __LINE__);
            }
        }
        break;
        case THICKNESS:
        {
            int thickness;
            sscanf(returnString, "%d", &thickness);
            if (thickness > 0)
            {
                currentProperties->ArcThickness(thickness);
            }
            else
            {
                xpWarn("thickness must be positive.\n", 
                       __FILE__, __LINE__);
            }
        }
        break;
        case TWILIGHT:
        {
            int value;
            sscanf(returnString, "%d", &value);
            if (value >= 0 && value <= 90)
            {
                currentProperties->Twilight(value);
            }
            else
            {
                xpWarn("Twilight value should be between 0 and 90 degrees\n",
                       __FILE__, __LINE__);
            }
        }
        break;
        default:
        {
            ostringstream errStr;
            errStr << "Unknown keyword in configuration file:\n\t" 
                   << line << endl;
            xpWarn(errStr.str(), __FILE__, __LINE__);
        }
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

        if (val == ENDOFLINE) break;
    }
}

void
readConfigFile(string configFile, PlanetProperties *planetProperties[])
{
    bool foundFile = findFile(configFile, "config");
    if (foundFile)
    {
        defaultProperties = new PlanetProperties(UNKNOWN_BODY);
        currentProperties = defaultProperties;

        ifstream inFile(configFile.c_str());
        char *line = new char[256];
        while (inFile.getline(line, 256, '\n') != NULL)
            readConfig(line, planetProperties);
        
        // This condition will only be true if [default] is the only
        // section in the config file.  In this case, set all planet
        // properties to the default values.
        if (currentProperties == defaultProperties)
        {
            for (int ibody = SUN; ibody < RANDOM_BODY; ibody++)
            {
                *planetProperties[ibody] = *defaultProperties;
            }
        }

        inFile.close();
        delete [] line;

        delete defaultProperties;
    }
    else
    {
        ostringstream errStr;
        errStr << "Can't load configuration file " << configFile << endl;
        xpExit(errStr.str(), __FILE__, __LINE__);
    }
}
