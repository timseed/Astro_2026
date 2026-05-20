#include <clocale>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

#include "findFile.h"
#include "Options.h"
#include "View.h"
#include "xpUtil.h"

#include "libdisplay/libdisplay.h"

void
drawStars(DisplayBase *display, View *view)
{
    Options *options = Options::getInstance();

    string starMap = options->getStarMap();
    if (!findFile(starMap, "stars"))
    {
        ostringstream errMsg;
        errMsg << "Can't open star map " << starMap << endl;
        xpWarn(errMsg.str(), __FILE__, __LINE__);
        return;
    }

    const int width = display->Width();
    const int height = display->Height();
    const int area = width * height;

    bool *starPresent = new bool [area];
    double *magnitude = new double [area];
    for (int i = 0; i < area; i++) 
    {
        starPresent[i] = false;
        magnitude[i] = 0;
    }

    ifstream inFile(starMap.c_str());

    char line[MAX_LINE_LENGTH];
    while (inFile.getline(line, MAX_LINE_LENGTH, '\n') != NULL)
    {
        if (line[0] == '#') continue;

        double Vmag, RA, Dec;
        checkLocale(LC_NUMERIC, "C");
        if (sscanf(line, "%lf %lf %lf", &Dec, &RA, &Vmag) < 3) continue;
        checkLocale(LC_NUMERIC, "");

        RA *= deg_to_rad;
        Dec *= deg_to_rad;

        double sX, sY, sZ;
        RADecToXYZ(RA, Dec, sX, sY, sZ);

        double X, Y, Z;
        view->XYZToPixel(sX, sY, sZ, X, Y, Z);
        X += options->CenterX();
        Y += options->CenterY();

        if (Z < 0 
            || X < 0 || X >= width
            || Y < 0 || Y >= height) continue;

        int ipos[4];
        ipos[0] = ((int) floor(Y)) * width + ((int) floor(X));
        ipos[1] = ipos[0] + 1;
        ipos[2] = ipos[0] + width;
        ipos[3] = ipos[2] + 1;

        const double t = X - floor(X);
        const double u = 1 - (Y - floor(Y));

        double weight[4];
        getWeights(t, u, weight);

        for (int i = 0; i < 4; i++)
        {
            if (ipos[i] >= area) ipos[i] = ipos[0];
            magnitude[ipos[i]] += weight[i] * pow(10, -0.4 * Vmag);
            starPresent[ipos[i]] = true;
        }
    }
    inFile.close();

    for (int i = 0; i < area; i++)
    {
        if (starPresent[i])
            magnitude[i] = -2.5 * log10(magnitude[i]);
    }

    // a magnitude 10 star will have a pixel brightness of 1
    const double baseMag = options->BaseMagnitude();
    const double logMagStep = options->LogMagnitudeStep();

    for (int j = 0; j < height; j++)
    {
        int istart = j * width;
        for (int i = 0; i < width; i++)
        {
            const double mag = magnitude[istart+i];
            if (starPresent[istart+i])
            {
                double brightness = pow(10, -logMagStep * (mag - baseMag));

                if (brightness > 255)
                    brightness = 255;

                display->setPixel(i, j, (unsigned int) brightness);
            }
        }
    }

    delete [] magnitude;
    delete [] starPresent;
}
