#ifndef RAYLEIGHSCATTERING_H
#define RAYLEIGHSCATTERING_H

#include <fstream>
#include <map>
#include <vector>

class Image;

class RayleighScattering
{
public:
    RayleighScattering(std::string configFile);
    virtual ~RayleighScattering();

    void clear();
    void clearTables();

    void createTables();
    
    double getColor(int index);
    double getRed() { return getColor(0); };
    double getGreen() { return getColor(1); };
    double getBlue() { return getColor(2); };

    double getScaleHeightKm() { return scaleHeight_ / 1e3; };

    void calcScatteringDisk(double inc, double ems, double phase);
    void calcScatteringLimb(double inc, double tanht, double phase);

private:
    std::vector<double> incidence_;
    std::vector<double> emission_;
    std::vector<double> tanHeight_;
    std::vector<double> phaseDeg_;
    std::vector<double> lambda_;

    std::map<int, Image *> PNGTable_;
    std::map<int, double *> BINTable_;

    bool diskPNG_;
    std::string diskTemplate_;
    double indexOfRefraction_;
    bool limbPNG_;
    std::string limbTemplate_;
    double numberDensity_;
    double radius_;
    double scaleHeight_;

    double scattering_[3];

    void calcScattering(double inc, double y, double phase, 
                        int color, std::vector<double> yaxis, 
                        std::string thisTemplate, bool usePNG);

    double * readBinaryTable(const char *filename);

    void readConfigFile(std::string configFile);

    void readTableValue(std::string thisTemplate, bool usePNG,
                        int color, int x, int y, 
                        std::vector<double> yaxis, 
                        int phase, double values[4]);

    bool readBlock(std::ifstream &inFile, 
                   const char *format, 
                   std::vector<double> &values);

    bool readValue(std::ifstream &inFile, 
                   const char *format, 
                   double &value);

    void writeTable(const char *buffer, double *array, 
                    size_t dim0, size_t dim1, size_t dim2, 
                    bool usePNG);

    void doubleToARGB(double x, unsigned char argb[4]);
    
    double ARGBToDouble(unsigned char argb[4]);

};

#endif
