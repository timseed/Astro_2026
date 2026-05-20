#ifndef OPTIONS_H
#define OPTIONS_H

#include <ctime>
#include <string>
#include <vector>

#include "body.h"

class PlanetProperties;

class Options
{
 public:
    static Options* getInstance();

    Options();
    ~Options();

    void parseArgs(int argc, char **argv);

    const std::vector<std::string> & ArcFiles() const { return(arcFiles_); };
    double ArcSpacing() const       { return(arcSpacing_); };
    int ArcThickness() const       { return(arcThickness_); };

    const std::string & Background() const { return(background_); };
    double BaseMagnitude() const { return(baseMag_); };

    bool CenterSelected() const     { return(centerSelected_); };
    void CenterX(const double x)    { centerX_ = x; };
    void CenterY(const double y)    { centerY_ = y; };
    double CenterX() const          { return(centerX_); };
    double CenterY() const          { return(centerY_); };
    const std::string & ConfigFile() const { return(configFile_); };
    const unsigned char * Color() const { return(color_); };

    const std::string & DateFormat() const { return(dateFormat_); };
    int DisplayMode() const { return(displayMode_); };
    bool DrawLabel() const { return(drawLabel_); };
    bool DrawUTCLabel() const { return(drawUTCLabel_); };
    const std::string & DynamicOrigin() const { return(dynamicOrigin_); };
    
    double FieldOfView() const           { return(fov_); };
    void FieldOfView(const double f)     { fov_ = f; };
    const std::string & Font() const { return(font_); };
    int FontSize() const { return(fontSize_); };
    bool Fork() const { return(fork_); };
    int FOVMode() const          { return(fovMode_); };
    void FOVMode(const int f)    { fovMode_ = f; };

    double Glare() const { return(glare_); };
    double GRSLon() const { return(grsLon_); };
    bool GRSSet() const { return(grsSet_); };

    unsigned long Hibernate() const { return(hibernate_); };

    unsigned long IdleWait() const { return(idleWait_); };
    bool InterpolateOriginFile() const { return(interpolateOriginFile_); };

    const std::string & JPLFile() const { return(jplFile_); };
    int JPEGQuality() const { return(quality_); };
    double JulianDay() const { return(julianDay_); };

    body LabelBody() const          { return(labelBody_); };
    int LabelMask() const { return(labelMask_); };
    int LabelX() const { return(labelX_); };
    int LabelY() const { return(labelY_); };
    const std::string & LabelString() const { return(labelString_); };
    double Latitude() const      { return(latitude_); };
    void Latitude(const double b) { latitude_ = b; };
    bool LightTime() const       { return(lightTime_); };
    double LocalTime() const { return(localTime_); };
    void LocalTime(const double l) { localTime_ = l; };
    double LogMagnitudeStep() const { return(logMagStep_); };
    double Longitude() const     { return(longitude_); };
    void Longitude(const double l) { longitude_ = l; };

    bool MakeCloudMaps() const { return(makeCloudMaps_); };

    const std::vector<std::string> & MarkerFiles() const { return(markerFiles_); };
    const std::string & MarkerBounds() const { return(markerBounds_); };

    int North() const            { return(north_); };
    int NumTimes() const         { return(numTimes_); };
    void NumTimes(const int n) { numTimes_ = n; };

    bool OppositeSide() const { return(oppositeSide_); };
    int OriginMode() const { return(originMode_); };
    body Origin() const          { return(origin_); };
    void Origin(const body b)    { origin_ = b; };
    const std::string & OriginFile() const { return(originFile_); };
    int OriginID() const { return(originID_); };
    const std::string & OutputBase() const      { return(outputBase_); };
    const std::string & OutputExtension() const { return(outputExt_); };
    const std::string & OutputMapRect() const { return(outputMapRect_); };
    int OutputStartIndex() const { return(outputStartIndex_); };

    bool Pango() const { return(pango_); };
    body PathRelativeTo() const { return(pathRelativeTo_); };
    int PathRelativeToID() const { return(pathRelativeToID_); };
    const std::string & PostCommand() const { return(post_command_); };
    const std::string & PrevCommand() const { return(prev_command_); };
    body Primary() const { return(primary_); };
    void Primary(const body p) { primary_ = p; };
    bool PrintEphemeris() const { return(printEphemeris_); };
    int Projection() const       { return(projection_); };
    void Projection(const int p)     { projection_ = p; };
    int ProjectionMode() const       { return(projectionMode_); };
    const std::vector<double> & ProjectionParameters() const { return(projectionParameters_); };
    void AddProjectionParameter(double p) { projectionParameters_.push_back(p); };

    double Radius() const        { return(radius_); };
    void Radius(const double r)  { radius_ = r; };
    bool RandomLatLonRot() const { return(random_); };
    double Range() const         { return(range_); };
    void Range(const double r) { range_ = r; };
    bool RangeSpecified() const { return(rangeSpecified_); };
    const std::string & RayleighFile() const { return(rayleighFile_); };
    void Rotate0(const double r) { rotate0_ = r; };
    double Rotate() const        { return(rotate_); };
    void Rotate(const double r) { rotate_ = rotate0_ + r; };

    const std::vector<std::string> & getSearchDir() const { return(searchdir); };
    bool SaveDesktopFile() const { return(saveDesktopFile_); };
    const std::vector<int> & SpiceEphemeris() const { return(spiceEphemeris_); };
    const std::vector<std::string> & SpiceFiles() const { return(spiceFiles_); };
    double StarFreq() const        { return(starFreq_); };
    const std::string & getStarMap() const { return(star_map); };

    double SeparationDist() const { return(separationDist_); };
    body SeparationTarget() const { return(separationTarget_); };
    double SunLat() const      { return(sunLat_); };
    void SunLat(const double b) { sunLat_ = b; };
    double SunLon() const     { return(sunLon_); };
    void SunLon(const double l) { sunLon_ = l; };

    body Target() const          { return(target_); };
    void Target(const body b)    { target_ = b; };
    int TargetID() const { return(targetID_); };
    int TargetMode() const { return(targetMode_); };

    double getTimeWarp() const      { return(timewarp); };
    time_t TVSec() const { return(tv_sec); };

    bool TransPNG() const { return(transpng_); };
    bool Transparency() const { return(transparency_); };

    std::string TmpDir() const { return(tmpDir_); };

    bool UniversalTime() const { return(universalTime_); };

    int Verbosity() const { return(verbosity_); };
    bool VirtualRoot() const        { return(virtual_root); };
    int getWait() const             { return(wait); };

    int getWindowX() const          { return(windowX_); };
    int getWindowY() const          { return(windowY_); };

    const std::string & WindowTitle() const { return(windowTitle_); };

    unsigned long XID() const { return(xid_); };
    const std::string & XYZFile() const { return(XYZFile_); };

    bool GeometrySelected() const   { return(geometrySelected_); };
    int GeometryMask() const     { return(geometryMask_); };
    int getWidth() const            { return((int) width); };
    int getHeight() const           { return((int) height); };

    void getOrigin(double &X, double &Y, double &Z);
    void setOrigin(const double X, const double Y, const double Z);
    void getTarget(double &X, double &Y, double &Z);
    void setTarget(const double X, const double Y, const double Z);
    bool UseCurrentTime() const { return(useCurrentTime_); };
    void incrementTime(const double sec);
    void setTime(const double jd);

 private:

    static Options *instance_;

    std::vector<std::string> arcFiles_;
    double arcSpacing_;
    int arcThickness_;

    std::string background_;
    double baseMag_;    // a star of this magnitude has a pixel
                        // brightness of 1
    bool centerSelected_;
    double centerX_, centerY_;
    unsigned char color_[3];
    std::string configFile_;

    std::string dateFormat_;
    int displayMode_;
    bool drawLabel_;
    bool drawUTCLabel_;
    std::string dynamicOrigin_;

    std::string font_;
    int fontSize_;
    bool fork_;
    double fov_;          // field of view
    int fovMode_;

    int geometryMask_;
    bool geometrySelected_;
    double glare_;
    double grsLon_;
    bool grsSet_;

    unsigned long hibernate_;

    unsigned long idleWait_;
    bool interpolateOriginFile_;

    std::string jplFile_;
    double julianDay_;

    int labelMask_;
    int labelX_;
    int labelY_;
    body labelBody_;      // print this body's info in the label
    std::string labelString_;
    double latitude_;
    bool lightTime_;
    double localTime_;
    double logMagStep_;   // log (base 10) of brightness increase for
                          // each step in magnitude (default 0.4)
    double longitude_;

    bool makeCloudMaps_;
    std::string markerBounds_;
    std::vector<std::string> markerFiles_;

    int north_;                    // BODY, GALACTIC, ORBIT, or TERRESTRIAL
    int numTimes_;

    bool oppositeSide_;
    body origin_;
    std::string originFile_;
    int originID_;                // for NAIF or NORAD bodies
    int originMode_;                     // BODY, LBR, RANDOM, MAJOR,
                                         // SYSTEM, ABOVE, BELOW
    std::string outputBase_;
    std::string outputExt_;
    std::string outputMapRect_;
    int outputStartIndex_;  // start numbering output files with this
                            // index
    double oX_, oY_, oZ_;   // heliocentric rectangular coordinates of
                            // the observer

    bool pango_;
    body pathRelativeTo_;
    int pathRelativeToID_;
    std::string post_command_;    // command to run after xplanet renders
    std::string prev_command_;    // command to run before xplanet renders
    body primary_;
    bool printEphemeris_;
    int projection_;         // type of map projection
    int projectionMode_;         // type of map projection
    std::vector<double> projectionParameters_; // extra parameters
                                               // used for projection

    int quality_;        // For JPEG images

    double radius_;      // radius of the body, as a fraction of the
                         // height of the display
    bool random_;
    bool rangeSpecified_; // if the -range option is used
    double range_;        // distance from the body, in units of its radius
    std::string rayleighFile_; // used to create Rayleigh scattering lookup tables
    double rotate_;       // rotate0 plus any increments
    double rotate0_;      // rotation angle specified on command line

    bool saveDesktopFile_;
    double separationDist_; // separation from the primary target
    body separationTarget_;     // secondary target
    std::vector<int> spiceEphemeris_;
    std::vector<std::string> spiceFiles_;
    double starFreq_;
    std::string star_map;
    double sunLat_, sunLon_;    // these aren't options and shouldn't
                                // be here, but it's a convenient way
                                // to put them in the label
    body target_;
    int targetID_;          // for NAIF or NORAD bodies
    int targetMode_;        // BODY, RANDOM, MAJOR
    double timewarp;        // multiplication factor for the passage of time
    std::string tmpDir_;
    bool transparency_;
    bool transpng_;

    double tX_, tY_, tZ_;   // heliocentric rectangular coordinates of
                            // the target

    bool universalTime_;
    bool useCurrentTime_;

    int verbosity_;
    bool virtual_root;

    int wait;           // time (in seconds) to wait between updates
    unsigned int width, height;
    int windowX_, windowY_;
    std::string windowTitle_;

    unsigned long xid_;
    std::string XYZFile_; // file containing XYZ coordinates of
                          // target, origin, and/or north

    std::vector<std::string> searchdir; // check these directories for files

    time_t tv_sec;      // UNIX time (seconds from 00:00:00 UTC on
                        // January 1, 1970)

    void showHelp();
};

#endif
