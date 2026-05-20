#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
using namespace std;

#include <sys/time.h>

#include "config.h"
#include "xpDefines.h"
#include "xpUtil.h"

#ifndef _GETOPT_H
#include "xpGetopt.h"
#endif
 
#ifdef HAVE_LIBX11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#else
#include "ParseGeom.h"
extern "C" {
    extern int 
    XParseGeometry(const char *string, int *x, int *y, unsigned int *width, 
                   unsigned int *height);
}
#endif

#include "keywords.h"
#include "findBodyXYZ.h"
#include "Options.h"
#include "parseColor.h"
#include "PlanetProperties.h"

#include "libannotate/libannotate.h"
#include "libplanet/Planet.h"
#include "libprojection/libprojection.h"

extern void
printVersion();

Options* Options::instance_ = NULL;

Options*
Options::getInstance()
{
    if (instance_ == NULL) instance_ = new Options;
    return(instance_);
}

Options::~Options()
{
//    delete instance_;
}

Options::Options() :
    arcSpacing_(0.1),
    arcThickness_(1),
    background_(""),
    baseMag_(10.0),
    centerSelected_(false),
    configFile_(defaultConfigFile),
    dateFormat_("%c %Z"),
    displayMode_(ROOT),
    drawLabel_(false),
    drawUTCLabel_(false),
    dynamicOrigin_(""),
    font_(defaultFont),
    fontSize_(12),
    fork_(false), 
    fov_(-1),
    fovMode_(RADIUS),
    geometryMask_(NoValue),
    geometrySelected_(false),
    glare_(28),
    grsLon_(94),
    grsSet_(false),
    hibernate_(0),
    idleWait_(0),
    interpolateOriginFile_(false),
    jplFile_(""),
    labelMask_(XNegative),
    labelX_(-15),
    labelY_(15),
    labelBody_(UNKNOWN_BODY), 
    labelString_(""), 
    latitude_(0),
    lightTime_(false),
    localTime_(-1), 
    logMagStep_(0.4),
    longitude_(0),
    makeCloudMaps_(false),
    markerBounds_(""),
    north_(BODY),
    numTimes_(0),
    oppositeSide_(false),
    origin_(SUN),
    originFile_(""),
    originID_(0),
    originMode_(LBR),
    outputBase_(""),
    outputExt_(defaultMapExt),
    outputMapRect_(""), 
    outputStartIndex_(0),
    oX_(0),
    oY_(0),
    oZ_(0),
    pango_(false),
    pathRelativeTo_(SUN),
    pathRelativeToID_(-1),
    post_command_(""),
    prev_command_(""),
    primary_(SUN),
    printEphemeris_(false),
    projection_(MULTIPLE),
    projectionMode_(MULTIPLE),
    quality_(80), 
    radius_(0.45),
    random_(false),
    rangeSpecified_(false), 
    range_(1000),
    rayleighFile_(""),
    rotate_(0),
    rotate0_(0),
    saveDesktopFile_(false),
    separationDist_(0), 
    separationTarget_(UNKNOWN_BODY), 
    starFreq_(0.001),
    star_map(defaultStarMap),
    sunLat_(0),
    sunLon_(0),
    target_(EARTH),
    targetID_(0),
    targetMode_(BODY),
    timewarp(1),
    tmpDir_(""),
    transparency_(false),
    transpng_(false),
    tX_(0),
    tY_(0),
    tZ_(0),
    universalTime_(true),
    useCurrentTime_(true),
    verbosity_(0),
    virtual_root(false),
    wait(300),
    width(512),
    height(512),
    windowX_(0),
    windowY_(0),
    windowTitle_(""),
    xid_(0),
    XYZFile_("")
{
    memset(color_, 0, 3);
    color_[0] = 255;          // default label color is red

    searchdir.push_back(DATADIR);
#if defined(HAVE_AQUA) || defined(HAVE_LIBX11)
    char *homeDir = getenv("HOME");
    if (homeDir != NULL)
    {
        ostringstream xplanetDir;
#ifdef HAVE_AQUA
        xplanetDir << homeDir << "/Library/Xplanet";
#else
        xplanetDir << homeDir << "/.xplanet";
#endif
        searchdir.push_back(xplanetDir.str());
    }
#endif
    searchdir.push_back("xplanet");

    struct timeval time;
    gettimeofday(&time, NULL);

    time_t t = time.tv_sec;
    julianDay_ = toJulian(gmtime(static_cast<time_t *> (&t))->tm_year + 1900,
                          gmtime(static_cast<time_t *> (&t))->tm_mon + 1,
                          gmtime(static_cast<time_t *> (&t))->tm_mday,
                          gmtime(static_cast<time_t *> (&t))->tm_hour,
                          gmtime(static_cast<time_t *> (&t))->tm_min,
                          gmtime(static_cast<time_t *> (&t))->tm_sec);

    tv_sec = get_tv_sec(julianDay_);
    srandom((unsigned int) tv_sec);
}

void
Options::parseArgs(int argc, char **argv)
{
    static struct option long_options[] =
        {
            {"arc_file",       required_argument, NULL, ARC_FILE},
            {"arc_spacing",    required_argument, NULL, ARC_SPACING},
            {"arc_thickness",  required_argument, NULL, THICKNESS},
            {"background",     required_argument, NULL, BACKGROUND},
            {"base_magnitude", required_argument, NULL, BASEMAG},
            {"body",           required_argument, NULL, TARGET},
            {"center",         required_argument, NULL, CENTER},
            {"color",          required_argument, NULL, COLOR},
            {"config",         required_argument, NULL, CONFIG_FILE},
            {"create_scattering_tables", required_argument, NULL, RAYLEIGH_FILE},
            {"date",           required_argument, NULL, DATE},
            {"date_format",    required_argument, NULL, DATE_FORMAT},
            {"dynamic_origin", required_argument, NULL, DYNAMIC_ORIGIN},
            {"ephemeris_file", required_argument, NULL, JPL_FILE},
            {"font",           required_argument, NULL, FONT},
            {"fontsize",       required_argument, NULL, FONTSIZE},
            {"fork",           no_argument,       NULL, FORK},
            {"fov",            required_argument, NULL, FOV},
            {"geometry",       required_argument, NULL, GEOMETRY},
            {"glare",          required_argument, NULL, GLARE},
            {"gmtlabel",       no_argument,       NULL, UTCLABEL},
            {"grs_longitude",  required_argument, NULL, GRS_LON},
            {"hibernate",      required_argument, NULL, HIBERNATE},
            {"idlewait",       required_argument, NULL, IDLEWAIT},
            {"interpolate_origin_file", no_argument, NULL, INTERPOLATE_ORIGIN_FILE},
            {"jdate",          required_argument, NULL, JDATE},
            {"label",          no_argument,       NULL, LABEL},
            {"labelpos",       required_argument, NULL, LABELPOS},
            {"label_body",     required_argument, NULL, LABEL_BODY},
            {"label_string",   required_argument, NULL, LABEL_STRING},
            {"latitude",       required_argument, NULL, LATITUDE},
            {"light_time",     no_argument,       NULL, LIGHT_TIME},
            {"localtime",      required_argument, NULL, LOCALTIME},
            {"log_magstep",    required_argument, NULL, LOGMAGSTEP},
            {"longitude",      required_argument, NULL, LONGITUDE},
            {"make_cloud_maps",no_argument,       NULL, MAKECLOUDMAPS},
            {"marker_file",    required_argument, NULL, MARKER_FILE},
            {"markerbounds",   required_argument, NULL, MARKER_BOUNDS},
            {"north",          required_argument, NULL, NORTH},
            {"num_times",      required_argument, NULL, NUM_TIMES},
            {"origin",         required_argument, NULL, ORIGIN},
            {"origin_file",    required_argument, NULL, ORIGINFILE},
            {"output",         required_argument, NULL, OUTPUT},
            {"output_map",     required_argument, NULL, OUTPUT_MAP_RECT},
            {"output_start_index",required_argument,NULL,OUTPUT_START_INDEX},
            {"pango",          no_argument,       NULL, PANGO},
            {"path_relative_to",         required_argument, NULL, PATH_RELATIVE_TO},
            {"post_command",   required_argument, NULL, POST_COMMAND},
            {"prev_command",   required_argument, NULL, PREV_COMMAND},
            {"print_ephemeris",no_argument,       NULL, EPHEMERIS},
            {"projection",     required_argument, NULL, PROJECTION},
            {"proj_param",     required_argument, NULL, PROJECTIONPARAMETER},
            {"quality",        required_argument, NULL, QUALITY},
            {"radius",         required_argument, NULL, RADIUS},
            {"random",         no_argument,       NULL, RANDOM},
            {"range",          required_argument, NULL, RANGE},
            {"rotate",         required_argument, NULL, ROTATE},
            {"save_desktop_file", no_argument,    NULL, SAVE_DESKTOP_FILE},
            {"searchdir",      required_argument, NULL, SEARCHDIR},
            {"separation",     required_argument, NULL, SEPARATION},
            {"spice_ephemeris", required_argument, NULL, SPICE_EPHEMERIS}, 
            {"spice_file",     required_argument, NULL, SPICE_FILE}, 
            {"starfreq",       required_argument, NULL, STARFREQ},
            {"starmap",        required_argument, NULL, STARMAP},
            {"target",         required_argument, NULL, TARGET},
            {"tt",             no_argument,       NULL, TERRESTRIAL},
            {"timewarp",       required_argument, NULL, TIMEWARP},
            {"tmpdir",         required_argument, NULL, TMPDIR}, 
            {"transparency",   no_argument,       NULL, TRANSPARENT},
            {"transpng",       required_argument, NULL, TRANSPNG},
            {"utclabel",       no_argument,       NULL, UTCLABEL},
            {"verbosity",      required_argument, NULL, VERBOSITY},
            {"version",        no_argument,       NULL, VERSIONNUMBER},
            {"vroot",          no_argument,       NULL, VROOT},
            {"wait",           required_argument, NULL, WAIT},
            {"window",         no_argument,       NULL, WINDOW},
            {"window-id",      required_argument, NULL, XWINID},
            {"window_title",   required_argument, NULL, WINDOWTITLE},
            {"XID",            required_argument, NULL, XWINID},
            {"xscreensaver",   no_argument,       NULL, VROOT},
            {"XYZ_file",       required_argument, NULL, XYZFILE},
            {NULL,             0,                 NULL, 0}
        };

    int this_option;
    int option_index = 0;

    while((this_option = getopt_long_only(argc, argv, "+", long_options, 
                                          &option_index)) >= 0)
    {
        switch (this_option)
        {
        case ARC_FILE:
            arcFiles_.push_back(optarg);
            break;
        case ARC_SPACING:
            sscanf(optarg, "%lf", &arcSpacing_);
            if (arcSpacing_ < 0)
            {
                ostringstream errMsg;
                errMsg << "Arc spacing must be > 0\n";
                xpWarn(errMsg.str(), __FILE__, __LINE__);
                arcSpacing_ = 0.1;
            }
            break;
        case BACKGROUND:
            background_ = optarg;
            break;
        case BASEMAG:
            sscanf(optarg, "%lf", &baseMag_);
            break;
        case CENTER:
        {
            unsigned int w, h;
            int x, y;
            int mask = XParseGeometry(optarg, &x, &y, &w, &h);
            centerSelected_ = ((mask & XValue) && (mask & YValue));
            centerX_ = x;
            centerY_ = y;
        }
        break;
        case COLOR:
            parseColor(optarg, color_);
            break;
        case CONFIG_FILE:
            configFile_ = optarg;
            break;
        case DATE:
        {
            long int yyyymmdd, hhmmss;
            sscanf(optarg, "%ld.%ld", &yyyymmdd, &hhmmss);
            int yyyymm = yyyymmdd / 100;
            int year = yyyymm/100;
            int month = abs(yyyymm - year * 100);
            int day = abs((int) yyyymmdd - yyyymm * 100);

            int hhmm = hhmmss / 100;
            int hour = hhmm / 100;
            int min = hhmm - hour * 100;
            int sec = hhmmss - hhmm * 100;

            julianDay_ = toJulian(year, month, day, hour, min, sec);
            tv_sec = get_tv_sec(julianDay_);

            useCurrentTime_ = false;
        }
        break;
        case DATE_FORMAT:
            dateFormat_ = optarg;
            break;
        case DYNAMIC_ORIGIN:
            dynamicOrigin_ = optarg;
            originMode_ = LBR;
            break;
        case EPHEMERIS:
            printEphemeris_ = true;
            break;
        case FONT:
#ifdef HAVE_LIBFREETYPE
            font_.assign(optarg);
#else
            {
                ostringstream errMsg;
                errMsg << "Sorry, this binary was built without FreeType "
                       << "support. The -" 
                       << long_options[option_index].name 
                       << " option will be ignored.\n";
                xpWarn(errMsg.str(), __FILE__, __LINE__);
            }
#endif
            break;
        case FONTSIZE:
        {
#ifdef HAVE_LIBFREETYPE
            int val;
            sscanf(optarg, "%d", &val);
            if (val > 0) fontSize_ = val;
#else
            {
                ostringstream errMsg;
                errMsg << "Sorry, this binary was built without FreeType "
                       << "support. The -" << long_options[option_index].name 
                       << " option will be ignored.\n";
                xpWarn(errMsg.str(), __FILE__, __LINE__);
            }
#endif
        }
        break;
        case FORK:
            fork_ = true;
            break;
        case FOV:
            sscanf(optarg, "%lf", &fov_);
            if (fov_ <= 0) 
            {
                xpWarn("FOV must be positive.\n", __FILE__, __LINE__);
            }
            else
            {
                fov_ *= deg_to_rad;
                fovMode_ = FOV;
            }
            break;
        case GEOMETRY:
        {
            geometryMask_ = XParseGeometry(optarg, &windowX_, &windowY_, 
                                           &width, &height);
            geometrySelected_ = ((geometryMask_ & WidthValue) 
                                 && (geometryMask_ & HeightValue));
        }
        break;
        case GLARE:
        { 
            double g;
            sscanf(optarg, "%lf", &g);
            if (g >= 0) glare_ = g;
        }
        break;
        case GRS_LON:
            sscanf(optarg, "%lf", &grsLon_);
            grsLon_ = fmod(grsLon_, 360);
            grsSet_ = true;
            break;
        case HIBERNATE:
            sscanf(optarg, "%lu", &hibernate_);
            hibernate_ *= 1000;
            break;
        case IDLEWAIT:
            sscanf(optarg, "%lu", &idleWait_);
            idleWait_ *= 1000;
            break;
        case INTERPOLATE_ORIGIN_FILE:
            interpolateOriginFile_ = true;
            break;
        case JDATE:
            sscanf(optarg, "%lf", &julianDay_);
            tv_sec = get_tv_sec(julianDay_);
            useCurrentTime_ = false;
            break;
        case JPL_FILE:
            jplFile_ = optarg;
            break;
        case LABEL:
            drawLabel_ = true;
            break;
        case LABELPOS:
        {
            unsigned int temp;
            labelMask_ = XParseGeometry(optarg, &labelX_,
                                        &labelY_, &temp, 
                                        &temp);
            
            if (labelMask_ & (WidthValue | HeightValue))
            {
                xpWarn("width and height supplied in -labelpos will be ignored\n",
                       __FILE__, __LINE__);
            }
            drawLabel_ = true;
        }
        break;
        case LABEL_BODY:
        {
            labelBody_ = Planet::parseBodyName(optarg);
            if (labelBody_ >= UNKNOWN_BODY)
            {
                xpWarn("Unknown body specified for label_body\n", 
                       __FILE__, __LINE__);
                labelBody_ = UNKNOWN_BODY;
            }
        }
        break;
        case LABEL_STRING:
            labelString_ = optarg;
            drawLabel_ = true;
            break;
        case LATITUDE:
            sscanf(optarg, "%lf", &latitude_);
            if (latitude_ < -90) latitude_ = -90;
            if (latitude_ > 90) latitude_ = 90;
            latitude_ *= deg_to_rad;
            originMode_ = LBR;
            break;
        case LIGHT_TIME:
            lightTime_ = true;
            break;
        case LOCALTIME:
            sscanf(optarg, "%lf", &localTime_);
            if (localTime_ < 0 || localTime_ > 24)
            {
                localTime_ = fmod(localTime_, 24.);
                if (localTime_ < 0) localTime_ += 24;

                ostringstream errStr;
                errStr << "localtime set to " << localTime_ << "\n";
                xpWarn(errStr.str(), __FILE__, __LINE__);
            }
            originMode_ = LBR;
            break;
        case LOGMAGSTEP:
            sscanf(optarg, "%lf", &logMagStep_);
            break;
        case LONGITUDE:
            sscanf(optarg, "%lf", &longitude_);
            longitude_ = fmod(longitude_, 360);
            longitude_ *= deg_to_rad;
            originMode_ = LBR;
            break;
        case MAKECLOUDMAPS:
            displayMode_ = OUTPUT;
            makeCloudMaps_ = true;
            break;
        case MARKER_BOUNDS:
            markerBounds_.assign(optarg);
            break;
        case MARKER_FILE:
            markerFiles_.push_back(optarg);
            break;
        case NORTH:
        {
            char *lowercase = optarg;
            char *ptr = optarg;
            while (*ptr) *ptr++ = tolower(*optarg++);
            if (strncmp(lowercase, "galactic", 1) == 0)
                north_ = GALACTIC;
            else if (strncmp(lowercase, "orbit", 1) == 0)
                north_ = ORBIT;
            else if (strncmp(lowercase, "path", 1) == 0)
                north_ = PATH;
            else if (strncmp(lowercase, "separation", 1) == 0)
                north_ = SEPARATION;
            else if (strncmp(lowercase, "terrestrial", 1) == 0)
                north_ = TERRESTRIAL;
            else 
            {
                if (strncmp(lowercase, "body", 1) != 0)
                    xpWarn("Unknown value for -north, using body\n",
                           __FILE__, __LINE__);
                north_ = BODY;
            }
        }
        break;
        case NUM_TIMES:
            sscanf(optarg, "%d", &numTimes_);
            if (numTimes_ < 0) numTimes_ = 0;
            break;
        case ORIGIN:
        {
            char *name = optarg;
            if (name[0] == '-')
            {
                oppositeSide_ = true;
                name++;
            }
            origin_ = Planet::parseBodyName(name);
            switch (origin_)
            {
            case ABOVE_ORBIT:
                originMode_ = ABOVE;
                break;
            case BELOW_ORBIT:
                originMode_ = BELOW;
                break;
            case MAJOR_PLANET:
                originMode_ = MAJOR;
                break;
            case NAIF:
                if (strlen(name) < 5)
                {
                    ostringstream errMsg;
                    errMsg << "NAIF id must be specified "
                           << "(e.g. naif-82 for Cassini)\n";
                    xpWarn(errMsg.str(), __FILE__, __LINE__);
                }
                else
                {
                    sscanf(name+4, "%d", &originID_);
                }
                originMode_ = LBR;
                break;
            case NORAD:
                if (strlen(name) < 6)
                {
                    ostringstream errMsg;
                    errMsg << "NORAD id must be specified "
                           << "(e.g. NORAD20580 for Hubble)\n";
                    xpWarn(errMsg.str(), __FILE__, __LINE__);
                }
                else
                {
                    sscanf(name+5, "%d", &originID_);
                }
                originMode_ = LBR;
                break;
            case RANDOM_BODY:
                originMode_ = RANDOM;
                break;
            case SAME_SYSTEM:
                originMode_ = SYSTEM;
                break;
            case ALONG_PATH:
            case UNKNOWN_BODY:
                xpWarn("Invalid origin specified, using SUN\n",
                       __FILE__, __LINE__);
                origin_ = SUN;
            default:
                originMode_ = BODY;
                break;
            }
        }
        break;
        case ORIGINFILE:
            originFile_ = optarg;
            originMode_ = LBR;
            break;
        case TRANSPNG:
            transpng_ = true;
            // fall through to OUTPUT block
        case OUTPUT:
            outputBase_ = optarg;
            if (outputBase_.find('.') == string::npos)
            {
                outputExt_ = defaultMapExt;
            }
            else
            {
                outputExt_.assign(outputBase_, outputBase_.rfind('.'), 
                                  outputBase_.size());
                outputBase_.assign(outputBase_, 0, outputBase_.rfind('.'));
            }

            displayMode_ = OUTPUT;
            geometrySelected_ = true;
            break;
        case OUTPUT_MAP_RECT:
        {
            string base = optarg;
            string ext = defaultMapExt;
            if (base.find('.') != string::npos)
            {
                ext.assign(base, base.rfind('.'), base.size());
                base.assign(base, 0, base.rfind('.'));
            }
            outputMapRect_ = base + ext;
        }
        break;
        case OUTPUT_START_INDEX:
            sscanf(optarg, "%d", &outputStartIndex_);
            break;
        case PANGO:
#ifdef HAVE_LIBPANGOFT2 
            pango_ = true;
#else
            {
                ostringstream errMsg;
                errMsg << "Sorry, this binary was built without Pango "
                       << "support. The -" << long_options[option_index].name 
                       << " option will be ignored.\n";
                xpWarn(errMsg.str(), __FILE__, __LINE__);
            }
#endif
            break;
        case PATH_RELATIVE_TO:
            pathRelativeTo_ = Planet::parseBodyName(optarg);
            switch (pathRelativeTo_)
            {
            case NAIF:
                if (strlen(optarg) < 5)
                {
                    ostringstream errMsg;
                    errMsg << "NAIF id must be specified "
                           << "(e.g. naif-82 for Cassini)\n";
                    xpWarn(errMsg.str(), __FILE__, __LINE__);
                }
                else
                {
                    sscanf(optarg+4, "%d", &pathRelativeToID_);
                }
                break;
            case NORAD:
                if (strlen(optarg) < 6)
                {
                    ostringstream errMsg;
                    errMsg << "NORAD id must be specified "
                           << "(e.g. NORAD20580 for Hubble)\n";
                    xpWarn(errMsg.str(), __FILE__, __LINE__);
                }
                else
                {
                    sscanf(optarg+5, "%d", &pathRelativeToID_);
                }
                break;
            case RANDOM_BODY:
            case ABOVE_ORBIT:
            case ALONG_PATH:
            case BELOW_ORBIT:
            case DEFAULT:
            case MAJOR_PLANET:
            case SAME_SYSTEM:
            case UNKNOWN_BODY:
                xpWarn("Unknown body specified for path, using SUN\n",
                       __FILE__, __LINE__);
                pathRelativeTo_ = SUN;
            default:
                break;
            }
            break;      
        case POST_COMMAND:
            post_command_.assign(optarg);
            break;
        case PREV_COMMAND:
            prev_command_.assign(optarg);
            break;
        case PROJECTION:
            projectionMode_ = getProjectionType(optarg);
            break;
        case PROJECTIONPARAMETER:
        {
            double d;
            sscanf(optarg, "%lf", &d);
            projectionParameters_.push_back(d * deg_to_rad);
        }
        break;
        case QUALITY:
            sscanf(optarg, "%d", &quality_);
            if (quality_ < 0) quality_ = 0;
            if (quality_ > 100) quality_ = 100;
            break;
        case RADIUS:
            sscanf(optarg, "%lf", &radius_);
            if (radius_ <= 0)
                xpExit("radius must be positive\n", __FILE__, __LINE__);

            radius_ /= 100;
            fov_ = 1;   // just a sneaky way to know that -radius has
                        // been set
            fovMode_ = RADIUS;
            break;
        case RANDOM:
            random_ = true;
            originMode_ = LBR;

            // This block is repeated in setOrigin(), but this way the
            // user can use -random to set a random rotation and then
            // set the sub-observer point another way, like with
            // -origin sun
            longitude_ = random() % 360;
            longitude_ *= deg_to_rad;

            // Weight random latitudes towards the equator
            latitude_ = (random() % 2000)/1000.0 - 1;
            latitude_ = asin(latitude_);

            rotate0_ = random() % 360;
            rotate0_ *= deg_to_rad;
            rotate_ = rotate0_;

            break;
        case RANGE:
            sscanf(optarg, "%lf", &range_);
            rangeSpecified_ = (range_ > 1);
            if (!rangeSpecified_) 
            {
                range_ = 1000;
                xpWarn("range must be greater than 1\n",
                       __FILE__, __LINE__);
            }
            break;
        case RAYLEIGH_FILE:
            rayleighFile_ = optarg;
            break;
        case ROTATE:
            sscanf(optarg, "%lf", &rotate0_);
            rotate0_ = fmod(rotate0_, 360.) * deg_to_rad;
            rotate_ = rotate0_;
            break;
        case SAVE_DESKTOP_FILE:
            saveDesktopFile_ = true;
            break;
        case SEARCHDIR:
            searchdir.push_back(optarg);        
            break;
        case SEPARATION:
        {
            const char *colon = strstr(optarg, ":");
            if (colon != NULL)
            {
                sscanf(colon + 1, "%lf", &separationDist_);
                separationTarget_ = Planet::parseBodyName(optarg);
                if (separationTarget_ >= RANDOM_BODY) 
                    separationTarget_ = RANDOM_BODY;
            }
        }
        break;
        case SPICE_EPHEMERIS:
        {
#ifdef HAVE_CSPICE
            int id;
            sscanf(optarg, "%d", &id);
            spiceEphemeris_.push_back(id);
#else
            ostringstream errMsg;
            errMsg << "Sorry, this binary was built without SPICE "
                   << "support. The -" 
                   << long_options[option_index].name 
                   << " option will be ignored.\n";
            xpWarn(errMsg.str(), __FILE__, __LINE__);
#endif
        }
        break;
        case SPICE_FILE:
        {
#ifdef HAVE_CSPICE
            spiceFiles_.push_back(optarg);
#else
            ostringstream errMsg;
            errMsg << "Sorry, this binary was built without SPICE "
                   << "support. The -" 
                   << long_options[option_index].name 
                   << " option will be ignored.\n";
            xpWarn(errMsg.str(), __FILE__, __LINE__);
#endif
        }
        break;
        case STARFREQ:
            sscanf(optarg, "%lf", &starFreq_);
            if (starFreq_ < 0) starFreq_ = 0;
            else if (starFreq_ > 1) starFreq_ = 1;
            break;
        case STARMAP:
            star_map = optarg;
            break;
        case TARGET:
            target_ = Planet::parseBodyName(optarg);
            switch (target_)
            {
            case ALONG_PATH:
                targetMode_ = XYZ;
                break;
            case MAJOR_PLANET:
                targetMode_ = MAJOR;
                break;
            case NAIF:
                if (strlen(optarg) < 5)
                {
                    ostringstream errMsg;
                    errMsg << "NAIF id must be specified "
                           << "(e.g. naif-82 for Cassini)\n";
                    xpWarn(errMsg.str(), __FILE__, __LINE__);
                }
                else
                {
                    sscanf(optarg+4, "%d", &targetID_);
                    targetMode_ = XYZ;
                }
                break;
            case NORAD:
                if (strlen(optarg) < 6)
                {
                    ostringstream errMsg;
                    errMsg << "NORAD id must be specified "
                           << "(e.g. NORAD20580 for Hubble)\n";
                    xpWarn(errMsg.str(), __FILE__, __LINE__);
                }
                else
                {
                    sscanf(optarg+5, "%d", &targetID_);
                    targetMode_ = XYZ;
                }
                break;
            case RANDOM_BODY:
                targetMode_ = RANDOM;
                break;
            case ABOVE_ORBIT:
            case BELOW_ORBIT:
            case SAME_SYSTEM:
            case UNKNOWN_BODY:
                xpWarn("Unknown target body specified, using EARTH\n",
                       __FILE__, __LINE__);
                target_ = EARTH;
            default:
                targetMode_ = BODY;
                break;
            }
            break;      
        case TERRESTRIAL:
            universalTime_ = false;
            break;
        case THICKNESS:
        {
            int thickness;
            sscanf(optarg, "%d", &thickness);
            if (thickness > 0)
            {
                arcThickness_ = thickness;
            }
            else
            {
                xpWarn("thickness must be positive.\n", 
                       __FILE__, __LINE__);
            }
        }
        break;
        case TIMEWARP:
            sscanf(optarg, "%lf", &timewarp);
            useCurrentTime_ = false;
            break;
        case TMPDIR:
            tmpDir_.assign(optarg);
            tmpDir_ += separator;
            break;
        case TRANSPARENT:
            transparency_ = true;
            break;
        case UTCLABEL:
            drawLabel_ = true;
            drawUTCLabel_ = true;
            break;
        case VERBOSITY:
            sscanf(optarg, "%d", &verbosity_);
            break;
        case VERSIONNUMBER:
            printVersion();
            exit(EXIT_SUCCESS);
            break;
        case VROOT:
            virtual_root = true;
            break;
        case WAIT:
            sscanf(optarg, "%d", &wait);
            break;
        case WINDOWTITLE:
            windowTitle_.assign(optarg);
            // fall through
        case WINDOW:
            displayMode_ = WINDOW;
            geometrySelected_ = true;
            break;
        case XWINID:
        {
            if (optarg[0] == '0'
                && (optarg[1] == 'x' || optarg[1] == 'X'))
            {
                sscanf(optarg, "%lx", &xid_);
            }
            else
            {
                sscanf(optarg, "%lu", &xid_);
            }
            displayMode_ = WINDOW;
        }
        break;
        case XYZFILE:
        {
            XYZFile_ = optarg;
        }
        default:
        case UNKNOWN:
        {
            cout << "Valid options to Xplanet are:\n";
            unsigned int i = 0;
            while (1)
            {
                if (long_options[i].name == NULL) break;
                printf("-%-20s", long_options[i].name);
                if (long_options[i].has_arg) cout << " (needs argument)";
                cout << endl;
                i++;
            }
            exit(EXIT_SUCCESS);
        }
        break;
        }
    }

    if (optind < argc)
    {
        string errMsg("unrecognized options: ");
        while (optind < argc) 
        {
            errMsg += argv[optind++];
            errMsg += " ";
        }
        errMsg += "\n";
        if (long_options[option_index].has_arg)
        {
            errMsg += "Perhaps you didn't supply an argument to -";
            errMsg += long_options[option_index].name;
            errMsg += "?\n";
        }
        xpExit(errMsg, __FILE__, __LINE__);
    }

    // useCurrentTime is false if:
    // 1) -date or -jdate is used
    // 2) -timewarp is used
    // 3) -origin_file is used AND -interpolate_origin_file is not
    // used
    if (useCurrentTime_)
    {
        if (!originFile_.empty() && !interpolateOriginFile_)
        {
            useCurrentTime_ = false;
        }
    }

    if (!originFile_.empty()
        || !dynamicOrigin_.empty()) originMode_ = LBR;

    // A number of options are meaningless if we're not looking at a
    // planetary body.
    if (targetMode_ == XYZ)
    {
        if (projectionMode_ != MULTIPLE)
        {
            ostringstream errStr;
            errStr << "Can't use -projection option without a "
                   << "planetary body\n";
            xpWarn(errStr.str(), __FILE__, __LINE__);
            projectionMode_ = MULTIPLE;
        }

        if (fovMode_ != FOV)
        {
            fov_ = 45 * deg_to_rad;
            fovMode_ = FOV;
        }

        if (north_ == BODY || north_ == ORBIT)
        {
            north_ = TERRESTRIAL;
        }
    }
}

void
Options::getOrigin(double &X, double &Y, double &Z)
{
    X = oX_;
    Y = oY_;
    Z = oZ_;
}

void
Options::setOrigin(const double X, const double Y, const double Z)
{
    oX_ = X;
    oY_ = Y;
    oZ_ = Z;
}

void
Options::getTarget(double &X, double &Y, double &Z)
{
    X = tX_;
    Y = tY_;
    Z = tZ_;
}

void
Options::setTarget(const double X, const double Y, const double Z)
{
    tX_ = X;
    tY_ = Y;
    tZ_ = Z;
}

void
Options::incrementTime(const double sec)
{
    julianDay_ += sec/86400;
    tv_sec = get_tv_sec(julianDay_);
}

void
Options::setTime(const double jd)
{
    julianDay_ = jd;
    tv_sec = get_tv_sec(julianDay_);
}

