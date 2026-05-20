#ifndef PLANETPROPERTIES_H
#define PLANETPROPERTIES_H

#include <cstring>
#include <string>
#include <vector>

#include "body.h"

class PlanetProperties
{
 public:
    PlanetProperties(const body index);
    ~PlanetProperties();

    PlanetProperties & operator= (const PlanetProperties &p);

    const unsigned char * ArcColor() const { return(arcColor_); };
    void ArcColor(unsigned char color[3]) { memcpy(arcColor_, color, 3); };

    const int ArcThickness() const { return(arcThickness_); };
    void ArcThickness(const int thickness) { arcThickness_ = thickness; };

    const std::string & BumpMap() const           { return(bumpMap_); };
    void BumpMap(const std::string &b) { bumpMap_ = b; };

    double BumpScale() const { return(bumpScale_); };
    void BumpScale(double b) { bumpScale_ = b; };

    double BumpShade() const { return(bumpShade_); };
    void BumpShade(double s) { bumpShade_ = s; };

    double CloudGamma() const { return(cloudGamma_); };
    void CloudGamma(double c) { cloudGamma_ = c; };

    void CloudMap(const std::string &c) { cloudMap_ = c; };
    const std::string & CloudMap() const           { return(cloudMap_); };

    int CloudThreshold() const { return(cloudThreshold_); };
    void CloudThreshold(int c) { cloudThreshold_ = c; };

    const unsigned char * Color() const { return(color_); };
    void Color(unsigned char color[3]) { memcpy(color_, color, 3); };

    void DayMap(const std::string &dayMap) { dayMap_ = dayMap; };
    const std::string & DayMap() const           { return(dayMap_); };

    void Grid(const bool g) { grid_ = g; };
    bool Grid() const { return(grid_); };
    void Grid1(const int g) { grid1_ = g; };
    int Grid1() const { return(grid1_); };
    void Grid2(const int g) { grid2_ = g; };
    int Grid2() const { return(grid2_); };
    const unsigned char * GridColor() const { return(gridColor_); };
    void GridColor(unsigned char color[3]) { memcpy(gridColor_, color, 3); };

    const unsigned char * MarkerColor() const { return(markerColor_); };
    void MarkerColor(unsigned char color[3]) { memcpy(markerColor_, color, 3); };

    const std::string & MarkerFont() const { return(markerFont_); };
    void MarkerFont(const std::string &font) { markerFont_ = font; };

    int MarkerFontSize() const { return(markerFontSize_); };
    void MarkerFontSize(const int fontsize) { markerFontSize_ = fontsize; };

    const unsigned char * OrbitColor() const { return(orbitColor_); };
    void OrbitColor(unsigned char color[3]) { memcpy(orbitColor_, color, 3); };

    const unsigned char * TextColor() const { return(textColor_); };
    void TextColor(unsigned char color[3]) { memcpy(textColor_, color, 3); };

    void AddArcFile(const std::string &arcFile) { arcFiles_.push_back(arcFile); };
    bool DrawArcs() const { return(!arcFiles_.empty()); };
    const std::vector<std::string> & ArcFiles() const { return(arcFiles_); };

    void AddMarkerFile(const std::string &markerFile) { markerFiles_.push_back(markerFile); };
    bool DrawMarkers() const { return(!markerFiles_.empty()); };
    const std::vector<std::string> & MarkerFiles() const { return(markerFiles_); };

    void AddSatelliteFile(const std::string &satelliteFile) { satelliteFiles_.push_back(satelliteFile); };
    bool DrawSatellites() const { return(!satelliteFiles_.empty()); };
    const std::vector<std::string> & SatelliteFiles() const { return(satelliteFiles_); };

    bool MapBounds() const { return(mapBounds_); };
    void MapBounds(double &uly, double &ulx, 
                   double &lry, double &lrx) const
        {
            uly = mapUly_; ulx = mapUlx_; lry = mapLry_; lrx = mapLrx_;
        };
    
    void MapBounds(bool b, double uly, double ulx, 
                   double lry, double lrx)
        {
            mapBounds_ = b; mapUly_ = uly; mapUlx_ = ulx; mapLry_ = lry; mapLrx_ = lrx;
        };
    
    double MinRadiusForLabel() const { return(minRadiusForLabel_); };
    void MinRadiusForLabel(double m) { minRadiusForLabel_ = m; };

    double MaxRadiusForLabel() const { return(maxRadiusForLabel_); };
    void MaxRadiusForLabel(double m) { maxRadiusForLabel_ = m; };
    
    double MinRadiusForMarkers() const { return(minRadiusForMarkers_); };
    void MinRadiusForMarkers(double m) { minRadiusForMarkers_ = m; };

    void Name(const std::string &name) { name_ = name; };
    const std::string & Name() { return(name_); };

    void NightMap(const std::string &nightMap) { nightMap_ = nightMap; };
    const std::string & NightMap() const         { return(nightMap_); };

    bool SSECMap() const { return(ssecMap_); };
    void SSECMap(bool b) { ssecMap_ = b; };

    void SpecularMap(const std::string &specularMap) { specularMap_ = specularMap; };
    const std::string & SpecularMap() const      { return(specularMap_); };
    
    void Magnify(const double m) { magnify_ = m; };
    double Magnify() const { return(magnify_); };

    bool RandomOrigin() const { return(randomOrigin_); };
    void RandomOrigin(bool b) { randomOrigin_ = b; };

    bool RandomTarget() const { return(randomTarget_); };
    void RandomTarget(bool b) { randomTarget_ = b; };

    void RayleighEmissionWeight(double r) { rayleighEmissionWeight_ = r; };
    double RayleighEmissionWeight() { return rayleighEmissionWeight_; };

    void RayleighFile(const std::string &rayleighFile) { rayleighFile_ = rayleighFile; };
    const std::string & RayleighFile() const { return rayleighFile_; };

    void RayleighLimbScale(double r) { rayleighLimbScale_ = r; };
    double RayleighLimbScale() { return rayleighLimbScale_; };

    void RayleighScale(double r) { rayleighScale_ = r; };
    double RayleighScale() { return rayleighScale_; };

    double Shade() const { return(shade_); };
    void Shade(double s) { shade_ = s; };

    double Twilight() const { return(twilight_); };
    void Twilight(double t) { twilight_ = t; };

    void DrawOrbit(const bool d) { drawOrbit_ = d; };
    bool DrawOrbit() const { return(drawOrbit_); };
    void StartOrbit(const double s) { startOrbit_ = s; };
    double StartOrbit() const { return(startOrbit_); };
    void StopOrbit(const double s) { stopOrbit_ = s; };
    double StopOrbit() const { return(stopOrbit_); };
    void DelOrbit(const double d) { delOrbit_ = d; };
    double DelOrbit() const { return(delOrbit_); };

 private:

    body index_;

    unsigned char arcColor_[3];

    std::vector<std::string> arcFiles_;
    int arcThickness_;

    std::string bumpMap_;
    double bumpScale_;
    double bumpShade_;

    double cloudGamma_;
    std::string cloudMap_;
    int cloudThreshold_;

    unsigned char color_[3];

    std::string dayMap_;

    double delOrbit_;
    bool drawOrbit_;

    bool grid_;
    int grid1_, grid2_;    
    unsigned char gridColor_[3];

    double magnify_;

    bool mapBounds_;
    double mapUlx_, mapUly_, mapLrx_, mapLry_;

    unsigned char markerColor_[3];
    std::string markerFont_;
    int markerFontSize_;
    std::vector<std::string> markerFiles_;

    double minRadiusForLabel_, maxRadiusForLabel_;
    double minRadiusForMarkers_;

    std::string name_;

    std::string nightMap_;

    unsigned char orbitColor_[3];

    bool randomOrigin_;
    bool randomTarget_;

    double rayleighEmissionWeight_;
    std::string rayleighFile_;
    double rayleighLimbScale_;
    double rayleighScale_;

    std::vector<std::string> satelliteFiles_;

    bool ssecMap_;

    double shade_;

    std::string specularMap_;

    double startOrbit_, stopOrbit_;

    unsigned char textColor_[3];

    double twilight_;    // if the sun is within twilight degrees of
                         // the horizon, blend the day and night
                         // images together
};

#endif
