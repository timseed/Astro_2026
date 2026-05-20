#include <cmath>
#include <cstdlib>
#include <cstring>
#include <sstream>
using namespace std;

#include "findFile.h"
#include "xpUtil.h"

#include "libimage/Image.h"

/*
  Convert lon, lat coordinates to screen coordinates for the Mollweide
  projection.
*/
static void
sphericalToPixel(const int width, const int height, 
                 const double lon, const double lat, 
                 int &x, int &y)
{
    double theta = lat;
    double del_theta = 1;
    while (fabs(del_theta) > 1e-5)
    {
        del_theta = -((theta + sin(theta) - M_PI * sin(lat)) 
                      / (1 + cos(theta)));
        theta += del_theta;
    }
    theta /= 2;

    double X = lon / M_PI * cos(theta);
    double Y = sin(theta);

    x = (int) ((X + 1) * width/2);
    y = (int) (height/2 * (1 - Y));
}

/*
  Convert x, y coordinates on the screen to lon, lat for a rectangular
  projection.
*/
static void
pixelToSpherical(const int width, const int height, 
                 const int x, const int y, 
                 double &lon, double &lat)
{
    lon = (x + 0.5) * TWO_PI / width - M_PI;
    lat = M_PI_2 - (y + 0.5) * M_PI / height;
}

static void
equalizeHistogram(unsigned char *&rgb, const int width, const int height)
{
    // create a histogram
    int *hist = new int[256];
    for (int i = 0; i < 256; i++) hist[i] = 0;
    for (int i = 0; i < 3 * width * height; i += 3) 
        hist[(int) rgb[i]]++;

    // create an integrated histogram
    int *ihist = new int[256];
    ihist[0] = hist[0];
    for (int i = 1; i < 256; i++) ihist[i] = ihist[i-1] + hist[i];

    // replace the histogram by an intensity map
    double denom = static_cast<double> (ihist[255] - ihist[0]);
    if (denom > 0)
    {
        for (int i = 0; i < 256; i++)
            hist[i] = static_cast<int> (255 * (ihist[i] - ihist[0])/denom);
        
        for (int i = 0; i < 3 * width * height; i++) 
            rgb[i] = static_cast<unsigned char> (hist[static_cast<int>(rgb[i])]);
    }
    
    delete [] hist;
    delete [] ihist;
}

/*
  This routine reads in a global cloud image downloaded from
  http://www.ssec.wisc.edu/data/comp/latest_moll.gif 
  and reprojects and resizes the image, gets rid of the ugly pink
  coastlines, and stretches the contrast.
*/
static bool
convertSsecImage(Image *image, unsigned char *&rgb)
{
    // There's a 20 pixel border on the left & right and a 10 pixel border
    // on the top and bottom
    image->Crop(10, 20, image->Width() - 10, image->Height() - 20);
    const int image_height = image->Height();
    const int image_width = image->Width();

    // This array will hold the final cloud image
    rgb = (unsigned char *) malloc(3 * image_width * image_height);
    if (rgb == NULL) return(false);

    memset(rgb, 0, 3 * image_width * image_height);

    // This converts the mollweide projection to rectangular
    double lon, lat;
    int ipos = 0;
    const unsigned char *moll = image->getRGBData();
    for (int j = 0; j < image_height; j++)
    {
        for (int i = 0; i < image_width; i++)
        {
            int ii, jj;
            pixelToSpherical(image_width, image_height, i, j, lon, lat);
            sphericalToPixel(image_width, image_height, lon, lat, ii, jj);
            memcpy(rgb + ipos, moll + 3 * (jj * image_width + ii), 3);
            ipos += 3;
        }
    }

    int avg;
    int npoints;
    int avgwhole = 0;
    int npointswhole = 0;

    // Replace pink outlines by the average value in a 10x10 pixel square.
    for (int j = 0; j < 31; j++)
    {
        for (int i = 0; i < 62; i++)
        {
            avg = 0;
            npoints = 0;
            for (int jj = 0; jj < 10; jj++)
            {
                for (int ii = 0; ii < 10; ii++)
                {
                    ipos = 3*((10 * j + jj) * 620 + 10 * i + ii);
                    if (!(rgb[ipos] == 0xff 
                          && rgb[ipos+1] == 0
                          && rgb[ipos+2] == 0xff))
                    {
                        npoints++;
                        avg += (int) rgb[ipos];
                        npointswhole++;
                        avgwhole += (int) rgb[ipos];
                    }
                }
            }
            if (npoints != 0) avg = (int) (avg / (double) npoints);

            for (int jj = 0; jj < 10; jj++)
            {
                for (int ii = 0; ii < 10; ii++)
                {
                    ipos = 3*((10 * j + jj) * 620 + 10 * i + ii);
                    if (rgb[ipos] == 0xff 
                        && rgb[ipos+1] == 0
                        && rgb[ipos+2] == 0xff) 
                        memset(rgb + ipos, avg, 3);
                }
            }
        }
    }
    // Fill in the poles
    if (npointswhole != 0) 
        avgwhole = (int) (avgwhole / (double) npointswhole);
    for (int i = 0; i < image_width * image_height; i++)
    {
        ipos = 3 * i;
        if (rgb[ipos] < 0x03) memset(rgb + ipos, avgwhole, 3);
    }

    // Smooth out the seam at 180 degrees Longitude
    double eastVal, westVal;
    int eastIndex, westIndex;
    for (int i = 0; i < image_height - 1; i++)
    {
        eastIndex = 3 * (i * image_width + 1);
        westIndex = 3 * ((i + 1) * image_width - 2);
        eastVal = (double) rgb[eastIndex];
        westVal = (double) rgb[westIndex];
        memset(rgb + eastIndex - 3, 
               (int) (eastVal + (westVal - eastVal)/3), 3);
        memset(rgb + westIndex + 3, 
               (int) (westVal + (eastVal - westVal)/3), 3);
    }

    equalizeHistogram(rgb, image_width, image_height);

    return(true);
}

void
loadSSEC(Image *&image, const unsigned char *&rgb, string &imageFile, 
         const int imageWidth, const int imageHeight)
{
    bool foundFile = findFile(imageFile, "images");
    if (foundFile) 
    {
        image = new Image;
        foundFile = image->Read(imageFile.c_str());
    }
    
    if (foundFile)
    {
        unsigned char *tmpRGB = NULL;
        if (!convertSsecImage(image, tmpRGB))
        {
            ostringstream errStr;
            errStr << "Can't read SSEC map file: " << imageFile << "\n";
            xpWarn(errStr.str(), __FILE__, __LINE__);
            return;
        }

        Image *tmpImage = image;
        image = new Image(image->Width(), image->Height(), tmpRGB, NULL);
        delete tmpImage;
        free(tmpRGB);
        if ((image->Width() != imageWidth)
            || (image->Height() != imageHeight))
        {
            ostringstream errStr;
            errStr << "Resizing SSEC cloud map\n"
                   << "For better performance, all image maps should "
                   << "be the same size as the day map\n";
            xpWarn(errStr.str(), __FILE__, __LINE__);
            image->Resize(imageWidth, imageHeight);
        }
        rgb = image->getRGBData();
    }
    else
    {
        ostringstream errStr;
        errStr << "Can't load map file " << imageFile << "\n";
        xpWarn(errStr.str(), __FILE__, __LINE__);
    }
}

