#include <clocale>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <SpiceUsr.h>
using namespace std;

#include "findFile.h"
#include "keywords.h"
#include "Options.h"
#include "parse.h"
#include "sphericalToPixel.h"
#include "View.h"
#include "xpUtil.h"

#include "libannotate/libannotate.h"
#include "libplanet/Planet.h"
#include "libprojection/ProjectionBase.h"

bool
calculateSpicePosition(double jd, const int naifInt,
                       Planet *relative, const int relativeInt,
                       double &X, double &Y, double &Z)
{
    SpiceInt naifId = static_cast<SpiceInt> (naifInt);
    SpiceInt relativeTo = static_cast<SpiceInt> (relativeInt);
    
    // seconds past J2000
    const SpiceDouble et = ((jd - 2451545.0) * 86400);
    SpiceDouble pos[3];
    SpiceDouble lt;

    spkgps_c(naifId, et, "J2000", relativeTo, pos, &lt);
    if (return_c()) // true if spkgps_c fails
    {
        reset_c();  // reset the SPICE error flags
        return(false);
    }

    // convert from km to AU
    for (int i = 0; i < 3; i++) pos[i] /= AU_to_km;

    relative->getPosition(X, Y, Z);
    X += pos[0];
    Y += pos[1];
    Z += pos[2];

    Options *options = Options::getInstance();
    if (options->LightTime())
    {
        // Rectangular coordinates of the observer
        double oX, oY, oZ;
        options->getOrigin(oX, oY, oZ);

        // Now get the position relative to the origin
        double dX = X - oX;
        double dY = Y - oY;
        double dZ = Z - oZ;
        double dist = sqrt(dX*dX + dY*dY + dZ*dZ);

        double light_time = dist * AU_to_km / 299792.458;
        spkgps_c(naifId, et - light_time, "J2000", relativeTo, pos, &lt);
        if (return_c()) 
        {
            reset_c();
            return(false);
        }

        for (int i = 0; i < 3; i++) pos[i] /= AU_to_km;
                
        relative->getPosition(X, Y, Z);
        X += pos[0];
        Y += pos[1];
        Z += pos[2];
    }
    return(true);
}

static void
readSpiceFile(const char *line, 
              map<double, Planet *> &planetsFromSunMap, 
              View *view,  ProjectionBase *projection,
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
    string name("");
    
    unsigned char color[3] = { 255, 0, 0 };
    string font("");
    int fontSize = -1;
    string image("");
    int symbolSize = 2;
    bool syntaxError = false;

    int thickness = 1;

    double trailStart = 0;
    double trailEnd = 0;
    double trailInterval = 1;

    bool transparency = false;
    unsigned char transparent_pixel[3];

    bool haveId = false;
    int naifInt = 0; // Solar system barycenter
    int relativeInt = 10; // SUN
    SpiceChar relativeName[128];
    
    SpiceBoolean found;
    bodc2n_c(static_cast<SpiceInt> (relativeInt), 
             128, relativeName, &found);

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
                xpWarn("Unrecognized option for align in spice file\n",
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
                color[0] = r & 0xff;
                color[1] = g & 0xff;
                color[2] = b & 0xff;
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
            sscanf(returnString, "%d", &naifInt);

            SpiceChar bodyName[128];
            bodc2n_c(static_cast<SpiceInt> (naifInt), 128, bodyName, &found);

            if (found != SPICETRUE)
            {
                ostringstream errStr;
                errStr << "Invalid NAIF id code: " << naifInt << ".\n";
                xpWarn(errStr.str(), __FILE__, __LINE__);
                syntaxError = true;
            }
            else
            {
                if (name.empty()) name.assign(bodyName);
                haveId = true;
            }
        }
        break;
        case NAME:
            name.assign(returnString);
            break;
        case ORIGIN: // relative_to keyword
        {
            sscanf(returnString, "%d", &relativeInt);
            bodc2n_c(static_cast<SpiceInt> (relativeInt), 128, 
                     relativeName, &found);
            if (found != SPICETRUE)
            {
                ostringstream errStr;
                errStr << "Invalid NAIF id code: " << relativeInt << ".\n";
                xpWarn(errStr.str(), __FILE__, __LINE__);
                syntaxError = true;
            }
        }
        break;
        case SYMBOLSIZE:
            sscanf(returnString, "%d", &symbolSize);
            if (symbolSize < 0) symbolSize = 2;
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
            checkLocale(LC_NUMERIC, "C");
            if (!sscanf(returnString, "%lf,%lf,%lf", &trailStart, &trailEnd,
                        &trailInterval) == 3)
            {
                xpWarn("Need three values for trail{}!\n", 
                       __FILE__, __LINE__);
                syntaxError = true;
            }
            else
            {
                if (trailInterval < 1e-4) trailInterval = 1e-4;
            }
            checkLocale(LC_NUMERIC, "");
        }
        break;
        case TRANSPARENT:
        {
            int r, g, b;
            if (sscanf(returnString, "%d,%d,%d", &r, &g, &b) == 3)
            {
                transparent_pixel[0] = r & 0xff;
                transparent_pixel[1] = g & 0xff;
                transparent_pixel[2] = b & 0xff;
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
            errStr << "Syntax error in spice file\n";
            errStr << "line is \"" << line << "\"" << endl;
            xpWarn(errStr.str(), __FILE__, __LINE__);
            return;
        }

        if (val == ENDOFLINE) break;
    }

    if (!haveId)
    {
        ostringstream errStr;
        errStr << "Can't compute position of " << name 
               << " relative to " << relativeName
               << ", try a major planet or satellite.\n";
        xpWarn(errStr.str(), __FILE__, __LINE__);
        return;
    }

    Planet *relative = NULL;
    for (map<double, Planet *>::iterator it0 = planetsFromSunMap.begin(); 
         it0 != planetsFromSunMap.end(); it0++)
    {
        relative = it0->second;
        if (relativeInt == naif_id[relative->Index()]) break;
        relative = NULL;
    }

    if (relative == NULL) return;
    
    const double jd = options->JulianDay();
    double X, Y, Z;
    if (calculateSpicePosition(jd, naifInt, relative, relativeInt, X, Y, Z))
    {
        if (options->Verbosity() > 1)
        {
            ostringstream xpStr;
            xpStr << "Calculating position of " << name 
                  << " relative to " << relativeName << endl;
            xpMsg(xpStr.str(), __FILE__, __LINE__);
        }
    }
    else
    {
        ostringstream errStr;
        errStr << "Can't compute position of " << name 
               << " relative to " << relativeName << endl;
        xpWarn(errStr.str(), __FILE__, __LINE__);
        return;
    }
    
    bool plotThis = false;
    if (view != NULL)
    {
        double pX, pY, pZ;
        view->XYZToPixel(X, Y, Z, pX, pY, pZ);
        pX += options->CenterX();
        pY += options->CenterY();

        plotThis = (pZ > 0);

        // Rectangular coordinates of the observer
        double oX, oY, oZ;
        options->getOrigin(oX, oY, oZ);
        
        // Now get the position relative to the origin
        double dX = X - oX;
        double dY = Y - oY;
        double dZ = Z - oZ;
        double dist = sqrt(dX*dX + dY*dY + dZ*dZ);
            
        // don't plot this point if it's too close to a planet
        for (map<double, Planet *>::iterator it0 = planetsFromSunMap.begin();
             it0 != planetsFromSunMap.end(); it0++)
        {
          Planet *planet = it0->second;
          double rX, rY, rZ;
          planet->getPosition(rX, rY, rZ);
          view->XYZToPixel(rX, rY, rZ, rX, rY, rZ);
          rX += options->CenterX();
          rY += options->CenterY();
          
            double pixelDist = sqrt((rX - pX)*(rX - pX) 
                                    + (rY - pY)*(rY - pY));
            if (pixelDist < 1)
            {
                double planetRadius = planet->Radius() / dist;
                plotThis = (planetRadius / options->FieldOfView() > 0.01);
                break;
            }
        }

        if (options->Verbosity() > 0 && plotThis)
        {
            ostringstream msg;
            char buffer[256];
            char shortName[10];
            memcpy(shortName, name.c_str(), 9);
            shortName[9] = '\0';
            snprintf(buffer, 256, "%10s%10.4f%8.1f%8.1f\n",
                     shortName, dist, pX, pY);
            msg << buffer;
            xpMsg(msg.str(), __FILE__, __LINE__);
        }

        X = pX;
        Y = pY;
        Z = pZ;
    }
    else
    {
        double lat, lon;
        relative->XYZToPlanetographic(X, Y, Z, lat, lon);
        plotThis = projection->sphericalToPixel(lon * relative->Flipped(), 
                                                lat, X, Y);
        Z = 0;
    }
    
    if (plotThis)
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
            Text *t = new Text(color, ix, iy, xOffset, yOffset, 
                               align, name);
            annotationMap.insert(pair<const double, Annotation*>(Z, t));
        }
    }

    if (trailInterval > 0)
    {
        double X0, Y0, Z0;
        double X1, Y1, Z1;
        calculateSpicePosition(jd + trailStart, naifInt, 
                               relative, relativeInt, 
                               X0, Y0, Z0);
        for (double et = trailStart + trailInterval; 
             et <= trailEnd; et += trailInterval)
        {
            calculateSpicePosition(jd + et, naifInt, 
                                   relative, relativeInt, 
                                   X1, Y1, Z1);
            double newX0 = X1;
            double newY0 = Y1;
            double newZ0 = Z1;
            if (view != NULL)
            {
                view->XYZToPixel(X0, Y0, Z0, X0, Y0, Z0);
                X0 += options->CenterX();
                Y0 += options->CenterY();
                
                view->XYZToPixel(X1, Y1, Z1, X1, Y1, Z1);
                X1 += options->CenterX();
                Y1 += options->CenterY();
            }
            else
            {
                double lat, lon;
                relative->XYZToPlanetographic(X0, Y0, Z0, lat, lon);
                if (projection->sphericalToPixel(lon * relative->Flipped(), 
                                                 lat, X0, Y0))
                    Z0 = 0;
                else
                    Z0 = -1;

                relative->XYZToPlanetographic(X1, Y1, Z1, lat, lon);
                if (projection->sphericalToPixel(lon * relative->Flipped(),
                                                 lat, X1, Y1))
                    Z1 = 0;
                else
                    Z1 = -1;
            }

            if (Z0 >= 0 && Z1 >= 0)
            {
                double Z = 0.5 * (Z0 + Z1);
                LineSegment *ls = new LineSegment(color, thickness, 
                                                  X1, Y1, X0, Y0);
                annotationMap.insert(pair<const double, Annotation*>(Z, ls));
            }
            X0 = newX0;
            Y0 = newY0;
            Z0 = newZ0;
        }
    }
}
    
void
processSpiceKernels(const bool load)
{
    // Set the SPICELIB error response action to "RETURN":
    erract_c (  "SET", 200, "RETURN"  );
    
    // Output ALL CSPICE error messages on error:
    errprt_c (  "SET", 200, "NONE, ALL" );

    Options *options = Options::getInstance();
    vector<string> spiceFiles = options->SpiceFiles();
    for (unsigned int i = 0; i < spiceFiles.size(); i++)
    {
        string kernelFile(spiceFiles[i]);
        kernelFile += ".krn";
        if (findFile(kernelFile, "spice"))
        {
            ifstream inFile(kernelFile.c_str());
            char *line = new char[MAX_LINE_LENGTH];
            while (inFile.getline(line, MAX_LINE_LENGTH, '\n') != NULL)
            {
                int ii = 0;
                while (isDelimiter(line[ii]))
                {
                    ii++;
                    if (static_cast<unsigned int> (ii) > strlen(line))
                        continue;
                }
                if (isEndOfLine(line[ii])) continue;
                
                char *ptr = &line[ii];
                while (!(isDelimiter(*ptr) || isEndOfLine(*ptr))) 
                    ptr++;
                *ptr = '\0';

                string spiceFile(&line[ii]);
                if (findFile(spiceFile, "spice"))
                {
                    if (load)
                        furnsh_c(spiceFile.c_str());
                    else
                        unload_c(spiceFile.c_str());
                }
            }

            inFile.close();
            delete [] line;
        }
        else
        {
            ostringstream errStr;
            errStr << "Can't find spice kernel file " << kernelFile << endl;
            xpWarn(errStr.str(), __FILE__, __LINE__);
        }
    }
}

void
addSpiceObjects(map<double, Planet *> &planetsFromSunMap,
                View *view, ProjectionBase *projection,
                multimap<double, Annotation *> &annotationMap)
{
    Options *options = Options::getInstance();
    vector<string> spiceFiles = options->SpiceFiles();
    for (unsigned int i = 0; i < spiceFiles.size(); i++)
    {
        string spiceFile(spiceFiles[i]);
        if (findFile(spiceFile, "spice"))
        {
            ifstream inFile(spiceFile.c_str());
            char *line = new char[MAX_LINE_LENGTH];
            while (inFile.getline(line, MAX_LINE_LENGTH, '\n') != NULL)
                readSpiceFile(line, planetsFromSunMap, view, projection,
                              annotationMap);
            inFile.close();
            delete [] line;
        }
        else
        {
            ostringstream errStr;
            errStr << "Can't load spice file " << spiceFile << endl;
            xpWarn(errStr.str(), __FILE__, __LINE__);
        }
    }
}
