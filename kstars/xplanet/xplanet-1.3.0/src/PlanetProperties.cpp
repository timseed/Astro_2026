#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>
using namespace std;

#include "body.h"
#include "Options.h"
#include "PlanetProperties.h"
#include "xpDefines.h"

PlanetProperties::PlanetProperties(const body index)
    : index_(index), 
      arcThickness_(1),
      bumpMap_(""),
      bumpScale_(1),
      bumpShade_(-1.0),
      cloudGamma_(1),
      cloudMap_(""), 
      cloudThreshold_(90), 
      delOrbit_(2),
      drawOrbit_(false),
      grid_(false), 
      grid1_(6),
      grid2_(15),
      magnify_(1.0), 
      mapBounds_(false), 
      mapUlx_(0), mapUly_(0), mapLrx_(0), mapLry_(0),
      markerFont_(""),
      markerFontSize_(-1),
      minRadiusForLabel_(.01),
      maxRadiusForLabel_(3.0),
      minRadiusForMarkers_(40.0), 
      nightMap_(""), 
      randomOrigin_(true),
      randomTarget_(true),
      rayleighEmissionWeight_(0.),
      rayleighFile_(""),
      rayleighLimbScale_(1.),
      rayleighScale_(0.),
      ssecMap_(false),
      shade_(0.3),
      specularMap_(""),
      startOrbit_(-0.5), stopOrbit_(0.5),
      twilight_(6)
{
    memset(arcColor_, 255, 3);              // default arc color is white
    memset(color_, 255, 3);
    memset(gridColor_, 255, 3);             // default grid color is white
    memset(markerColor_, 0, 3);
    memset(orbitColor_, 255, 3);
    memset(textColor_, 0, 3);

    markerColor_[0] = 255;                  // default marker color is red
    textColor_[0] = 255;                    // default text color is red

    arcFiles_.clear();
    markerFiles_.clear();
    satelliteFiles_.clear();

    if (index < RANDOM_BODY) 
    {
        name_ = body_string[index];
        dayMap_ = name_ + defaultMapExt;
        name_[0] = toupper(name_[0]);
    }

    // Everything besides the name and day map gets set to default
    // values from the config file in readConfig.cpp, so there's no
    // point in setting them here.
}

PlanetProperties::~PlanetProperties()
{
}

PlanetProperties &
PlanetProperties::operator= (const PlanetProperties &p)
{
    memcpy(arcColor_, p.arcColor_, 3);
    memcpy(color_, p.color_, 3);
    memcpy(gridColor_, p.gridColor_, 3);
    memcpy(markerColor_, p.markerColor_, 3);
    memcpy(orbitColor_, p.orbitColor_, 3);
    memcpy(textColor_, p.textColor_, 3);

    for (unsigned int i = 0; i < p.arcFiles_.size(); i++)
        arcFiles_.push_back(p.arcFiles_[i]);

    for (unsigned int i = 0; i < p.markerFiles_.size(); i++)
        markerFiles_.push_back(p.markerFiles_[i]);

    for (unsigned int i = 0; i < p.satelliteFiles_.size(); i++)
        satelliteFiles_.push_back(p.satelliteFiles_[i]);

    arcThickness_ = p.arcThickness_;

    bumpMap_ = p.bumpMap_;
    bumpShade_ = p.bumpShade_;
    bumpScale_ = p.bumpScale_;

    cloudGamma_ = p.cloudGamma_;
    cloudMap_ = p.cloudMap_;
    cloudThreshold_ = p.cloudThreshold_;

    delOrbit_ = p.delOrbit_;
    drawOrbit_ = p.drawOrbit_;

    grid_ = p.grid_;
    grid1_ = p.grid1_;
    grid2_ = p.grid2_;

    magnify_ = p.magnify_;

    mapBounds_ = p.mapBounds_;
    mapUly_ = p.mapUly_;
    mapUlx_ = p.mapUlx_;
    mapLry_ = p.mapLry_;
    mapLrx_ = p.mapLrx_;

    markerFont_ = p.markerFont_;
    markerFontSize_ = p.markerFontSize_;

    minRadiusForLabel_ = p.minRadiusForLabel_;
    maxRadiusForLabel_ = p.maxRadiusForLabel_;

    minRadiusForMarkers_ = p.minRadiusForMarkers_;

    randomOrigin_ = p.randomOrigin_;
    randomTarget_ = p.randomTarget_;
    
    rayleighEmissionWeight_ = p.rayleighEmissionWeight_;
    rayleighFile_ = p.rayleighFile_;
    rayleighLimbScale_ = p.rayleighLimbScale_;
    rayleighScale_ = p.rayleighScale_;

    ssecMap_ = p.ssecMap_;
    shade_ = p.shade_;
    startOrbit_ = p.startOrbit_;
    stopOrbit_ = p.stopOrbit_;

    twilight_ = p.twilight_;
    
    return(*this);
}
