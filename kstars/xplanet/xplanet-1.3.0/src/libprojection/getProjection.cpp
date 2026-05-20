#include <cctype>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <vector>
using namespace std;

#include "keywords.h"
#include "xpUtil.h"

#include "ProjectionBase.h"
#include "ProjectionAncient.h"
#include "ProjectionAzimuthal.h"
#include "ProjectionAzimutEqualArea.h"
#include "ProjectionBonne.h"
#include "ProjectionGnomonic.h"
#include "ProjectionHemisphere.h"
#include "ProjectionIcosagnomonic.h"
#include "ProjectionLambert.h"
#include "ProjectionMercator.h"
#include "ProjectionMollweide.h"
#include "ProjectionOrthographic.h"
#include "ProjectionPeters.h"
#include "ProjectionPolyconic.h"
#include "ProjectionRectangular.h"
#include "ProjectionTSC.h"

static vector<int> projectionTypes;

static void
setProjectionTypes()
{
    if (projectionTypes.empty())
    {
        projectionTypes.push_back(ANCIENT);
        projectionTypes.push_back(AZIMUTHAL);
        projectionTypes.push_back(BONNE);
        projectionTypes.push_back(EQUAL_AREA);
        projectionTypes.push_back(GNOMONIC);
        projectionTypes.push_back(HEMISPHERE);
        projectionTypes.push_back(ICOSAGNOMONIC);
        projectionTypes.push_back(LAMBERT);
        projectionTypes.push_back(MERCATOR);
        projectionTypes.push_back(MOLLWEIDE);
        projectionTypes.push_back(ORTHOGRAPHIC);
        projectionTypes.push_back(PETERS);
        projectionTypes.push_back(POLYCONIC);
        projectionTypes.push_back(RECTANGULAR);
        projectionTypes.push_back(TSC);
    }
}

int
getRandomProjection()
{
    setProjectionTypes();
    int index = static_cast<int> (random() % projectionTypes.size());
    return(projectionTypes[index]);
}

int 
getProjectionType(char *proj_string)
{
    int projection;

    char *lowercase = proj_string;
    char *ptr = proj_string;
    while (*ptr != '\0') *ptr++ = tolower(*proj_string++);
    if (strncmp(lowercase, "ancient", 2) == 0)
        projection = ANCIENT;
    else if (strncmp(lowercase, "azimuthal", 2) == 0)
        projection = AZIMUTHAL;
    else if (strncmp(lowercase, "bonne", 1) == 0)
        projection = BONNE;
    else if (strncmp(lowercase, "equal_area", 1) == 0)
        projection = EQUAL_AREA;
    else if (strncmp(lowercase, "gnomonic", 1) == 0)
        projection = GNOMONIC;
    else if (strncmp(lowercase, "hemisphere", 1) == 0)
        projection = HEMISPHERE;
    else if (strncmp(lowercase, "icosagnomonic", 1) == 0)
        projection = ICOSAGNOMONIC;
    else if (strncmp(lowercase, "lambert", 1) == 0)
        projection = LAMBERT;
    else if (strncmp(lowercase, "mercator", 2) == 0)
        projection = MERCATOR;
    else if (strncmp(lowercase, "mollweide", 2) == 0)
        projection = MOLLWEIDE;
    else if (strncmp(lowercase, "orthographic", 1) == 0)
        projection = ORTHOGRAPHIC;
    else if (strncmp(lowercase, "peters", 2) == 0)
        projection = PETERS;
    else if (strncmp(lowercase, "polyconic", 2) == 0)
        projection = POLYCONIC;
    else if (strncmp(lowercase, "random", 2) == 0)
        projection = RANDOM;
    else if (strncmp(lowercase, "rectangular", 2) == 0)
        projection = RECTANGULAR;
    else if (strncmp(lowercase, "tsc", 1) == 0)
        projection = TSC;
    else 
    {
        setProjectionTypes();

        ostringstream errMsg;
        errMsg << "Unknown projection \"" << lowercase <<  "\"."
               << "  Valid projections are:\n";

        for (unsigned int i = 0; i < projectionTypes.size(); i++)
            errMsg << "\t" << keyWordString[projectionTypes[i]-'?'] << "\n";
        xpExit(errMsg.str(), __FILE__, __LINE__);
    }
    return(projection);
}

ProjectionBase *
getProjection(const int projection, const int flipped,
              const int width, const int height)
{
    ProjectionBase *thisProjection = NULL;
    switch (projection)
    {
    case ANCIENT:
        thisProjection = new ProjectionAncient(flipped, width, height);
        break;
    case AZIMUTHAL:
        thisProjection = new ProjectionAzimuthal(flipped, width, height);
        break;
    case BONNE:
        thisProjection = new ProjectionBonne(flipped, width, height);
        break;
    case EQUAL_AREA:
        thisProjection = new ProjectionAzimutEqualArea(flipped, width, 
						       height);
        break;
    case GNOMONIC:
        thisProjection = new ProjectionGnomonic(flipped, width, height);
        break;
    case HEMISPHERE:
        thisProjection = new ProjectionHemisphere(flipped, width, height);
        break;
    case ICOSAGNOMONIC:
        thisProjection = new ProjectionIcosagnomonic(flipped, width, height);
        break;
    case LAMBERT:
        thisProjection = new ProjectionLambert(flipped, width, height);
        break;
    case MERCATOR:
        thisProjection = new ProjectionMercator(flipped, width, height);
        break;
    case MOLLWEIDE:
        thisProjection = new ProjectionMollweide(flipped, width, height);
        break;
    case ORTHOGRAPHIC:
        thisProjection = new ProjectionOrthographic(flipped, width, height);
        break;
    case PETERS:
        thisProjection = new ProjectionPeters(flipped, width, height);
        break;
    case POLYCONIC:
        thisProjection = new ProjectionPolyconic(flipped, width, height);
        break;
    case RECTANGULAR:
        thisProjection = new ProjectionRectangular(flipped, width, height);
        break;
    case TSC:
        thisProjection = new ProjectionTSC(flipped, width, height);
        break;
    default:
        xpWarn("getProjection: Unknown projection type specified\n",
               __FILE__, __LINE__);
        thisProjection = new ProjectionRectangular(flipped, width, height);
        break;
    }
    return(thisProjection);
}
