#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <sstream>
using namespace std;

#include "findFile.h"
#include "parse.h"
#include "xpUtil.h"

#include "libimage/Image.h"
#include "libmultiple/RayleighScattering.h"

// Calculate the airmass using the expressions in Smith and Smith, JGR
// 77(19), 3592--3597, 1972
// x = distance from planet center, units of atmospheric scale height
// chi = zenith angle
static double
chapman(double x, double chi)
{
    double chapman;
    double y = sqrt(x/2) * fabs(cos(chi));

    if (chi < M_PI_2)
        chapman = sqrt(x * M_PI_2) * exp(y*y) * erfc(y);
    else
        chapman = sqrt(2 * M_PI * x) 
            * (sqrt(sin(chi))*exp(x*(1-sin(chi)))
               -0.5*exp(y*y)*erfc(y));

    return chapman;
}

// Find the altitude (units of planetary radius) where the sun sets
// for a given zenith angle
static double
shadowHeight(double sza)
{
    double shadowHeight;
    if (cos(sza) > 0)
        shadowHeight = 0;
    else
        shadowHeight = 1/sin(sza) - 1;

    return shadowHeight;
}

// find the height above the ground given the zenith angle at the ground
// and the slant path length to the ground
static double 
slantHeight(double chi, double slant)
{
    double a = 1;
    double b = 2;
    double c = -slant * (slant + 2 * cos(chi));

    return (-b + sqrt(b*b-4*a*c))/2*a; 
}

// find the slant path length to the ground given the zenith angle at the ground
// and the height above the ground
static double
slantLength(double chi, double height)
{
    double a = 1;
    double b = 2 * cos(chi);
    double c = -height * (height + 2);

    return (-b + sqrt(b*b-4*a*c))/2*a; 
}

// find the zenith angle along the slant path given the
// zenith angle at the ground and the height above the ground
static double
zenithAlongSlant(double chi, double height)
{
    return (asin(sin(chi) / (1+height)));
}

// find the zenith angle at a point along the tangent given the zenith
// angle at the tangent point.  Set isNear to true if the point is
// closer than the tangent, false if it is on the far side.  Tangent
// point is assumed to be at the ground.
static double
zenithAlongTangent(double chi, double height, bool isNear)
{
    double angle1 = asin(1/(1+height));
    double angle2 = M_PI_2 - chi;

    if (isNear)
    {
        return angle1 - angle2;
    }
    else
    {
        return M_PI - angle1 - angle2;
    }
}

void 
RayleighScattering::createTables()
{
    double deg_to_rad = M_PI/180;

    double planck1 = 1.19104e-6;
    double planck2 = 0.0143883;
    double temp = 5700;

    double peakLambda = 510e-9;
    double peakRadiance = planck1/(pow(peakLambda,5) 
                                  * (exp(planck2/(peakLambda*temp))-1));

    double htop = tanHeight_[tanHeight_.size()-1];
    double min_dh = scaleHeight_ / 5;
    
    // assume tables for each degree between 0 and 180
    vector<double> phase_d;
    vector<double> phase;
    for (int i = 0; i < 181; i++)
    {
        phase_d.push_back(i);
        phase.push_back(i * deg_to_rad);
    }

    int area_th = incidence_.size() * tanHeight_.size();
    int area_em = incidence_.size() * emission_.size();
        
    double n2m1 = indexOfRefraction_*indexOfRefraction_-1;

    // build the limb tables
    for (unsigned int ip = 0; ip < phase.size(); ip++)
    {
        double th_array[lambda_.size() * area_th];
        for (unsigned int i = 0; i < lambda_.size() * area_th; i++) 
            th_array[i] = 0;

        for (unsigned int il = 0; il < lambda_.size(); il++)
        {
            double lambda4 = pow(lambda_[il], 4);
            double rayleighScale = 1/peakRadiance;
            rayleighScale *= planck1/(pow(lambda_[il],5) 
                                      * (exp(planck2/(lambda_[il]*temp))-1));
            rayleighScale *= (2 * pow(M_PI*n2m1, 2)) / (3*numberDensity_);
            double crossRayleigh = 4*M_PI*rayleighScale / (numberDensity_*lambda4);

            // fudge factor - scale the red down at low phase angles
            if (il == 0) rayleighScale *= 0.5 * (1 + (1-cos(phase[ip]/2)));

            for (unsigned int ii = 0; ii < incidence_.size(); ii++)
            {
                if (phase[ip] > (M_PI_2+incidence_[ii])+2*deg_to_rad) continue;
                if (phase[ip] < fabs(M_PI_2-incidence_[ii])-2*deg_to_rad) continue;

                double hmin = shadowHeight(incidence_[ii]) * radius_;
                if (htop < hmin) continue;

                double cos2phase = cos(phase[ip]) * cos(phase[ip]);

                for (unsigned int it = 0; it < tanHeight_.size(); it++)
                {
                    unsigned int ipos = il*area_th + it*incidence_.size() + ii;

                    double rayleigh = 0;
                    // slant distance from the tangent point to htop
                    double xfar = slantLength(M_PI_2,
                                              (htop-tanHeight_[it])
                                              /(radius_+tanHeight_[it]));
                    xfar *= (radius_+tanHeight_[it]);
                    int n_h = (int) ceil(xfar/min_dh);
                    double dx = xfar / (n_h-1);

                    // the far side of the tangent point
                    for (int ix = n_h-2; ix >= 0; ix--)
                    {
                        double x = (ix + 0.5)* dx;
                        double h = slantHeight(M_PI_2, 
                                               x/(radius_+tanHeight_[it]));
                        double thisIncidence = zenithAlongTangent(incidence_[ii], h, 
                                                                  (phase[ip]>M_PI_2));
                        double thisEmission = zenithAlongTangent(M_PI_2, h, false);
                        h *= (radius_ + tanHeight_[it]);
                        h += tanHeight_[it];

                        hmin = shadowHeight(thisIncidence) * radius_;
                        if (h < hmin) continue;

                        double atten = exp(-h/scaleHeight_);
                        double tauScale = numberDensity_*atten*scaleHeight_*crossRayleigh;
                        double tauSun = chapman((h+radius_)/scaleHeight_, thisIncidence);
                        double tauView = chapman((h+radius_)/scaleHeight_, thisEmission);

                        rayleigh += dx * atten * exp(-(tauSun+tauView)*tauScale);
                    }
                    // the near side of the tangent point
                    for (int ix = 0; ix < n_h-1; ix++)
                    {
                        double x = (ix+0.5) * dx;
                        double h = slantHeight(M_PI_2, x/(radius_+tanHeight_[it]));
                        double thisIncidence = zenithAlongTangent(incidence_[ii], h, 
                                                                  (phase[ip]<M_PI_2));
                        double thisEmission = zenithAlongTangent(M_PI_2, h, true);
                        h *= (radius_ + tanHeight_[it]);
                        h += tanHeight_[it];
                        
                        hmin = shadowHeight(thisIncidence) * radius_;
                        if (h < hmin) continue;

                        double atten = exp(-h/scaleHeight_);
                        double tauScale = numberDensity_*atten*scaleHeight_*crossRayleigh;
                        double tauSun = chapman((h+radius_)/scaleHeight_, thisIncidence);
                        double tauView = chapman((h+radius_)/scaleHeight_, thisEmission);

                        rayleigh += dx * atten * exp(-(tauSun+tauView)*tauScale);
                    }
                    rayleigh *= (0.75*(1+cos2phase)*rayleighScale/lambda4);

                    th_array[ipos] = rayleigh;
                } // tangent height
            } // incidence
        } // lambda
        char buffer[64];
        snprintf(buffer, 64, limbTemplate_.c_str(), (int) (phase_d[ip]));

        writeTable(buffer, th_array, lambda_.size(), incidence_.size(), 
                   tanHeight_.size(), limbPNG_);
    }

    // Now build the tables for the disk
    for (unsigned int ip = 0; ip < phase.size(); ip++)
    {
        double em_array[lambda_.size() * area_em];
        for (unsigned int i = 0; i < lambda_.size() * area_em; i++) 
            em_array[i] = 0;

        double cos2phase = cos(phase[ip]) * cos(phase[ip]);
        for (unsigned int il = 0; il < lambda_.size(); il++)
        {
            double lambda4 = pow(lambda_[il], 4);
            double rayleighScale = 1/peakRadiance;
            rayleighScale *= planck1/(pow(lambda_[il],5) 
                                      * (exp(planck2/(lambda_[il]*temp))-1));
            rayleighScale *= (2 * pow(M_PI*n2m1, 2)) / (3*numberDensity_);
            double crossRayleigh = 4*M_PI*rayleighScale 
                / (numberDensity_*lambda4);

            for (unsigned int ii = 0; ii < incidence_.size(); ii++)
            {
                double hmin = shadowHeight(incidence_[ii]) * radius_;
                if (hmin > htop) continue;

                for (unsigned int ie = 0; ie < emission_.size(); ie++)
                {
                    if (phase[ip] > (emission_[ie]+incidence_[ii])+2*deg_to_rad) continue;
                    if (phase[ip] < fabs(emission_[ie]-incidence_[ii])-2*deg_to_rad) continue;


                    unsigned int ipos = il * area_em + ie * incidence_.size() + ii;

                    double rayleigh = 0;

                    // slant distance from the tangent point to htop
                    double xfar = slantLength(emission_[ie], htop/radius_);
                    xfar *= radius_;
                    int n_h = (int) ceil(xfar/min_dh);
                    double dx = xfar / (n_h-1);

                    // integrate along the slant path
                    for (int ix = 0; ix < n_h-1; ix++)
                    {
                        double x = (ix+0.5) * dx;
                        double h = slantHeight(emission_[ie], x/radius_);
                        double thisIncidence = zenithAlongSlant(incidence_[ii], h);
                        double thisEmission = zenithAlongSlant(emission_[ie], h);
                        h *= radius_;

                        if (h < hmin) continue;

                        double atten = exp(-h/scaleHeight_);
                        double tauSun = chapman((h+radius_)/scaleHeight_, thisIncidence);
                        double tauView = chapman((h+radius_)/scaleHeight_,thisEmission);
                        double tauScale = numberDensity_ * atten * 
                            scaleHeight_ * crossRayleigh;

                        rayleigh += dx * atten * exp(-(tauSun+tauView)*tauScale);
                    }
                    rayleigh *= (0.75*(1+cos2phase)*rayleighScale/lambda4);

                    em_array[ipos] = rayleigh;
                } // emission
            } // incidence
        } // lambda

        char buffer[64];
        snprintf(buffer, 64, diskTemplate_.c_str(), (int) (phase_d[ip]));

        writeTable(buffer, em_array, lambda_.size(), incidence_.size(), 
                   emission_.size(), diskPNG_);
    }
}

RayleighScattering::RayleighScattering(string configFile) :
    diskPNG_(false),
    diskTemplate_(""),
    indexOfRefraction_(0.),
    limbPNG_(false),
    limbTemplate_(""),
    numberDensity_(0.),
    radius_(0.),
    scaleHeight_(0.)
{
    bool foundFile = findFile(configFile, "scattering");
    if (!foundFile)
    {
        ostringstream errStr;
        errStr << "Can't load scattering file " << configFile << "\n";
        xpExit(errStr.str(), __FILE__, __LINE__);       
    }

    phaseDeg_.clear();
    for (int i = 0; i < 181; i++)
    {
        phaseDeg_.push_back((double) i);
        PNGTable_.insert(make_pair(i, (Image *) NULL));
        BINTable_.insert(make_pair(i, (double *) NULL));
    }

    for (int i = 0; i < 3; i++) scattering_[i] = 0;

    readConfigFile(configFile);
}

void 
RayleighScattering::readConfigFile(string configFile)
{
    ifstream inFile(configFile.c_str());
    char line[MAX_LINE_LENGTH];

    vector<double> values;
    if (!readBlock(inFile, "INCIDENCE %d", values))
    {
        ostringstream errStr;
        errStr << "INCIDENCE block not found in " << configFile << "\n";
        xpExit(errStr.str(), __FILE__, __LINE__);       
    }
    incidence_.clear();
    for (unsigned int ii = 0; ii < values.size(); ii++)
        incidence_.push_back(values[ii] * deg_to_rad);

    if (!readBlock(inFile, "EMISSION %d", values))
    {
        ostringstream errStr;
        errStr << "EMISSION block not found in " << configFile << "\n";
        xpExit(errStr.str(), __FILE__, __LINE__);       
    }
    emission_.clear();
    for (unsigned int ii = 0; ii < values.size(); ii++)
        emission_.push_back(values[ii] * deg_to_rad);

    if (!readBlock(inFile, "TANGENT_HEIGHT %d", values))
    {
        ostringstream errStr;
        errStr << "TANGENT_HEIGHT block not found in " << configFile << "\n";
        xpExit(errStr.str(), __FILE__, __LINE__);       
    }
    tanHeight_.clear();
    for (unsigned int ii = 0; ii < values.size(); ii++)
        tanHeight_.push_back(values[ii] * 1e3);

    diskTemplate_.clear();
    limbTemplate_.clear();
    while (inFile.getline(line, MAX_LINE_LENGTH, '\n') != NULL)
    {
        int i = 0;
        while (isDelimiter(line[i]))
        {
            i++;
            if (static_cast<unsigned int> (i) > strlen(line)) break;
        }
        if (static_cast<unsigned int> (i) > strlen(line)) continue;
        if (isEndOfLine(line[i])) continue;

        char templ1[MAX_LINE_LENGTH];
        char templ2[MAX_LINE_LENGTH];
        sscanf(line, "TEMPLATES %s %s", templ1, templ2);

        diskTemplate_.assign(templ1);
        limbTemplate_.assign(templ2);

        size_t found = diskTemplate_.find(".png");
        if (found == string::npos) found = diskTemplate_.find(".PNG");
        diskPNG_ = (found != string::npos);
            
        found = limbTemplate_.find(".png");
        if (found == string::npos) found = limbTemplate_.find(".PNG");
        limbPNG_ = (found != string::npos);
           
        break;
    }
    if (diskTemplate_.length() == 0) 
    {
        ostringstream errStr;
        errStr << "TEMPLATE disk file not found in " << configFile << "\n";
        xpExit(errStr.str(), __FILE__, __LINE__);       
    }
    if (limbTemplate_.length() == 0) 
    {
        ostringstream errStr;
        errStr << "TEMPLATE limb file not found in " << configFile << "\n";
        xpExit(errStr.str(), __FILE__, __LINE__);       
    }

    // remaining parameters are only required for table generation
    readBlock(inFile, "WAVELENGTHS %d", values);
    lambda_.clear();
    for (unsigned int ii = 0; ii < values.size(); ii++)
        lambda_.push_back(values[ii] * 1e-9);

    readValue(inFile, "RADIUS %lf", radius_);
    radius_ *= 1e3;

    readValue(inFile, "SCALE_HEIGHT %lf", scaleHeight_);

    readValue(inFile, "INDEX_OF_REFRACTION %lf", indexOfRefraction_);

    readValue(inFile, "DENSITY %lf", numberDensity_);
}

RayleighScattering::~RayleighScattering()
{
    clearTables();
}

bool
RayleighScattering::readBlock(ifstream &inFile, 
                              const char *format, 
                              vector<double> &values)
{
    values.clear();

    char line[MAX_LINE_LENGTH];
    while (inFile.getline(line, MAX_LINE_LENGTH, '\n') != NULL)
    {
        int i = 0;
        while (isDelimiter(line[i]))
        {
            i++;
            if (static_cast<unsigned int> (i) > strlen(line)) break;
        }
        if (static_cast<unsigned int> (i) > strlen(line)) continue;
        if (isEndOfLine(line[i])) continue;

        int size;
        if (sscanf(line, format, &size) == 0) break;

        for (int ii = 0; ii < size; ii++) 
        {
            double thisValue;
            inFile >> thisValue;
            values.push_back(thisValue);
        }
        return true;
    }
    return false;
}

bool
RayleighScattering::readValue(ifstream &inFile, 
                              const char *format, 
                              double &value)
{
    char line[MAX_LINE_LENGTH];
    while (inFile.getline(line, MAX_LINE_LENGTH, '\n') != NULL)
    {
        int i = 0;
        while (isDelimiter(line[i]))
        {
            i++;
            if (static_cast<unsigned int> (i) > strlen(line)) break;
        }
        if (static_cast<unsigned int> (i) > strlen(line)) continue;
        if (isEndOfLine(line[i])) continue;

        if (sscanf(line, format, &value) == 0) break;

        return true;
    }

    return false;
}

void
RayleighScattering::clear()
{ 
    for (int i = 0; i < 3; i ++) scattering_[i] = 0; 
}

void
RayleighScattering::clearTables()
{ 
    for (map<int, Image *>::iterator it = PNGTable_.begin(); 
         it != PNGTable_.end(); it++)
    {
        delete it->second;
        it->second = NULL;
    }
    for (map<int, double *>::iterator it = BINTable_.begin(); 
         it != BINTable_.end(); it++)
    {
        delete [] it->second;
        it->second = NULL;
    }
}

// store a double (0 < x < 1) as a four byte array of unsigned char
// big endian order: 
// four bytes ABCD -> alpha = A, red = B, green = C, blue = D
void 
RayleighScattering::doubleToARGB(double x, unsigned char argb[4])
{
    static const unsigned long maxValue = 0xffffffff;
    unsigned long ul = (unsigned long) (x * maxValue);
    
    argb[0] = (ul >> 24) & 0xff;
    argb[1] = (ul >> 16) & 0xff;
    argb[2] = (ul >> 8) & 0xff;
    argb[3] =  ul & 0xff;
}

// turn a four byte unsigned char array to a double (0 < x < 1)
// Note: four bytes is only good enough for a float
double
RayleighScattering::ARGBToDouble(unsigned char argb[4])
{
    static const unsigned long maxValue = 0xffffffff;
    unsigned long ul = argb[0] << 24 | argb[1] << 16 | argb[2] << 8 | argb[3];

    double x = ul;
    x /= maxValue;

    return x;
}
void
RayleighScattering::writeTable(const char *buffer, double *array, 
                               size_t dim0, size_t dim1, size_t dim2, 
                               bool usePNG)
{
    if (usePNG)
    {
        unsigned int area = dim1 * dim2;
            
        unsigned char rgb[dim0 * 3 * area];
        unsigned char alpha[dim0 * area];
        memset(rgb, 0, dim0 * 3 * area);
        memset(alpha, 0, dim0 * area);
            
        unsigned char argb[4];

        for (unsigned int il = 0; il < dim0; il++)
        {
            for (unsigned int it = 0; it < dim2; it++)
            {
                for (unsigned int ii = 0; ii < dim1; ii++)
                {
                    unsigned int ipos = il * area + it * dim1 + ii;
                    doubleToARGB(array[ipos], argb);
                    alpha[il * area + it * dim1 + ii] = argb[0];
                    memcpy(rgb+3*(il * area + it*dim1 + ii), 
                           argb + 1, 3);
                }
            }
        }
        Image pngImage(dim1, dim0*dim2, rgb, alpha);
        if (!pngImage.Write(buffer))
        {
            ostringstream errStr;
            errStr << "Can't write scattering file " << buffer << "\n";
            xpWarn(errStr.str(), __FILE__, __LINE__);       
        }
        else
        {
            string message("Wrote ");
            message.append(buffer);
            message.append("\n");
            xpMsg(message, __FILE__, __LINE__);
        }  
    }
    else
    {
        FILE *outfile = fopen(buffer, "wb");
        if (outfile == NULL)
        {
            ostringstream errStr;
            errStr << "Can't write scattering file " << buffer << "\n";
            xpWarn(errStr.str(), __FILE__, __LINE__);       
        }
        else
        {
            fwrite(&dim0, sizeof(size_t), 1, outfile);
            fwrite(&dim1, sizeof(size_t), 1, outfile);
            fwrite(&dim2, sizeof(size_t), 1, outfile);
            fwrite(array, sizeof(double), dim0*dim1*dim2, outfile);
            fclose(outfile);
            string message("Wrote ");
            message.append(buffer);
            message.append("\n");
            xpMsg(message, __FILE__, __LINE__);
        }
    }
}

double *
RayleighScattering::readBinaryTable(const char *filename)
{
    double *dblArray = NULL;

    FILE *inFile = fopen(filename, "rb");

    if (inFile != NULL)
    {
        size_t dim0, dim1, dim2;
        fread(&dim0, sizeof(size_t), 1, inFile);
        fread(&dim1, sizeof(size_t), 1, inFile);
        fread(&dim2, sizeof(size_t), 1, inFile);

        size_t size = dim0*dim1*dim2;

        dblArray = new double[size];
        fread(dblArray, sizeof(double), size, inFile);
    }

    fclose(inFile);

    return dblArray;
}

double
RayleighScattering::getColor(int index)
{
    return scattering_[index];
}

void
RayleighScattering::calcScatteringLimb(double inc, double tanht, double phase)
{
    for (int ic = 0; ic < 3; ic++)
        calcScattering(inc, tanht, phase, ic, tanHeight_, limbTemplate_, 
                       limbPNG_);
}

void
RayleighScattering::calcScatteringDisk(double inc, double ems, double phase)
{
    if (phase > M_PI_2) phase = M_PI - phase;
    for (int ic = 0; ic < 3; ic++)
        calcScattering(inc, ems, phase, ic, emission_, diskTemplate_, diskPNG_);
}

void
RayleighScattering::readTableValue(string thisTemplate, bool usePNG, 
                                   int color, int x, int y, 
                                   vector<double> yaxis, 
                                   int phase, double values[4])
{
    if (usePNG)
    {
        map<int, Image *>::iterator it = PNGTable_.find(phase);
        Image *image = NULL;
        if (it->second == NULL)
        {
            char buffer[64];
            snprintf(buffer, 64, thisTemplate.c_str(), phase);
            string thisFile(buffer);
            if (!findFile(thisFile, "scattering")) 
            {
                ostringstream errStr;
                errStr << "Can't load scattering file " << thisFile << "\n";
                xpExit(errStr.str(), __FILE__, __LINE__);
            }
            image = new Image();
            if (!image->Read(thisFile.c_str()))
            {
                scattering_[color] = 0;
                return;
            }
            it->second = image;
        }

        unsigned char argb[4];
        image = it->second;
        image->getPixel(x, y, argb+1, argb);
        values[0] = ARGBToDouble(argb);
        image->getPixel(x+1, y, argb+1, argb);
        values[1] = ARGBToDouble(argb);
        image->getPixel(x, y+1, argb+1, argb);
        values[2] = ARGBToDouble(argb);
        image->getPixel(x+1, y+1, argb+1, argb);
        values[3] = ARGBToDouble(argb);
    }
    else
    {
        map<int, double *>::iterator it = BINTable_.find(phase);
        double *array = NULL;
        if (it->second == NULL)
        {
            char buffer[64];
            snprintf(buffer, 64, thisTemplate.c_str(), phase);
            string thisFile(buffer);
            if (!findFile(thisFile, "scattering"))
            {
                ostringstream errStr;
                errStr << "Can't load scattering file " << thisFile << "\n";
                xpExit(errStr.str(), __FILE__, __LINE__);
            }
            array = readBinaryTable(thisFile.c_str());
            if (array == NULL)
            {
                scattering_[color] = 0;
                return;
            }
            it->second = array;
        }

        array = it->second;
        unsigned int ipos[4];
        ipos[0] = y * incidence_.size() + x;
        ipos[1] = y * incidence_.size() + x+1;
        ipos[2] = (y+1) * incidence_.size() + x;
        ipos[3] = (y+1) * incidence_.size() + x+1;
        unsigned int isize = 3 * yaxis.size() * incidence_.size();
        for (int i = 0; i < 4; i++) 
        {
            if (isize > ipos[i]) values[i] = array[ipos[i]];
            else values[i] = 0;
        }
    }
}

void
RayleighScattering::calcScattering(double inc, double yValue, double phase, 
                                   int color, vector<double> yaxis, 
                                   string thisTemplate, bool usePNG)
{
    double deg_to_rad = M_PI / 180;

    if (inc < incidence_.front() || inc > incidence_.back() \
        || yValue < yaxis.front() || yValue > yaxis.back())
    {
        scattering_[color] = 0;
        return;
    }

    double phase_deg = phase / deg_to_rad;

    int phase_lo = floor(phase_deg);
    if (phase_lo < 0) phase_lo = 0;

    int phase_hi = ceil(phase_deg);
    if (phase_hi - phase_deg < 1e-3) phase_hi = phase_lo+1;

    vector<double>::iterator inc_hi, inc_lo;
    // lower_bound returns first element greater than or equal to value
    inc_lo = lower_bound(incidence_.begin(), incidence_.end(), inc);
    if (inc_lo > incidence_.begin()) inc_lo--;
    // upper_bound returns first element greater than value
    inc_hi = upper_bound(incidence_.begin(), incidence_.end(), inc);

    vector<double>::iterator y_hi, y_lo;
    y_lo = lower_bound(yaxis.begin(), yaxis.end(), yValue);
    if (y_lo > yaxis.begin()) y_lo--;
    y_hi = upper_bound(yaxis.begin(), yaxis.end(), yValue);

    int x = inc_lo - incidence_.begin();
    double xFrac = (*inc_hi == *inc_lo ? 0 : (inc - *inc_lo) / (*inc_hi - *inc_lo));
    int y = y_lo - yaxis.begin();
    double yFrac = (*y_hi == *y_lo ? 0 : (yValue - *y_lo) / (*y_hi - *y_lo));

    double weight[4];
    getWeights(xFrac, 1-yFrac, weight);

    y += color * yaxis.size();

    double values[4];

    readTableValue(thisTemplate, usePNG, color, x, y, yaxis, phase_lo, values);
    double loPhase = 0;
    for (int i = 0; i < 4; i++) 
        loPhase += weight[i] * values[i];

    readTableValue(thisTemplate, usePNG, color, x, y, yaxis, phase_hi, values);
    double hiPhase = 0;
    for (int i = 0; i < 4; i++) 
        hiPhase += weight[i] * values[i];

    scattering_[color] = loPhase;
    if (phase_hi - phase_lo != 0)
        scattering_[color] += (phase_deg-phase_lo)/(phase_hi-phase_lo) 
            * (hiPhase - loPhase);

    if (scattering_[color] < 0) scattering_[color] = 0;
}
