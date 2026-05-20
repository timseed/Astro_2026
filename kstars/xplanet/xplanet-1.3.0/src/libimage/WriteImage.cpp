#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;

#include "config.h"

extern "C"
{
    int
    write_bmp(const char *filename, int width, int height, 
              unsigned char *rgb);
    
#ifdef HAVE_LIBGIF
    int
    write_gif(const char *filename, int width, int height, 
              unsigned char *rgb);
#endif
    
#ifdef HAVE_LIBJPEG
    int
    write_jpeg(FILE *outfile, int width, int height, 
               unsigned char *rgb, int quality);
#endif
    
#ifdef HAVE_LIBPNG
    int
    write_png(FILE *outfile, int width, int height, unsigned char *rgb, 
              unsigned char *alpha);
#endif
    
#ifdef HAVE_LIBPNM
#include <pnm.h>
    int
    write_pnm(FILE *outfile, int width, int height, unsigned char *rgb,
              int maxv, int format, int forceplain);
#endif
    
#ifdef HAVE_LIBTIFF
    int
    write_tiff(const char *filename, int width, int height, 
               unsigned char *rgb);
#endif
}

bool 
WriteImage(const char *filename, const int width, const int height, 
           unsigned char * const rgb_data, 
           unsigned char * const png_alpha, 
           const int quality)
{
    FILE *outfile;
    const char *extension = strrchr(filename, '.');
    char *lowercase;
    char *ptr;
    int success = 0;
  
    lowercase = (char *) malloc(strlen(extension) + 1);
    strcpy(lowercase, extension);
    ptr = lowercase;

    while (*ptr != '\0') *ptr++ = (char) tolower(*extension++);

    outfile = fopen(filename, "wb");
    if (outfile == NULL) return(false);
  
    if (strcmp(lowercase, ".bmp" ) == 0)
    {
        success = write_bmp(filename, width, height, rgb_data); 
    }
    else if (strcmp(lowercase, ".gif" ) == 0)
    {
#ifdef HAVE_LIBGIF
        success = write_gif(filename, width, height, rgb_data); 
#else
        fprintf(stderr, 
                "Sorry, this program was not compiled with GIF support\n");
        success = 0;
#endif /* HAVE_LIBGIF */
    }
    else if ((   strcmp(lowercase, ".jpg" ) == 0)
             || (strcmp(lowercase, ".jpeg") == 0))
    {
#ifdef HAVE_LIBJPEG
        success = write_jpeg(outfile, width, height, rgb_data, quality); 
#else
        fprintf(stderr, 
                "Sorry, this program was not compiled with JPEG support\n");
        success = 0;
#endif /* HAVE_LIBJPEG */
    }

    else if (strcmp(lowercase, ".png" ) == 0)
    {
#ifdef HAVE_LIBPNG
        success = write_png(outfile, width, height, rgb_data, png_alpha); 
#else
        fprintf(stderr, 
                "Sorry, this program was not compiled with PNG support\n");
        success = 0;
#endif /* HAVE_LIBPNG */
    }

    else if ((   strcmp(lowercase, ".pbm") == 0)
             || (strcmp(lowercase, ".pgm") == 0)
             || (strcmp(lowercase, ".ppm") == 0))
    {
#ifdef HAVE_LIBPNM
        if (strcmp(lowercase, ".pbm") == 0)
            success = write_pnm(outfile, width, height, rgb_data, 1, 
                                PBM_TYPE, 0);
        else if (strcmp(lowercase, ".pgm") == 0)
            success = write_pnm(outfile, width, height, rgb_data, 255, 
                                PGM_TYPE, 0);
        else if (strcmp(lowercase, ".ppm") == 0)
            success = write_pnm(outfile, width, height, rgb_data, 255, 
                                PPM_TYPE, 0);
#else
        fprintf(stderr, 
                "Sorry, this program was not compiled with PNM support\n");
        success = 0;
#endif /* HAVE_LIBPNM */
    }

    else if ((strcmp(lowercase, ".tif" ) == 0)
             || (strcmp(lowercase, ".tiff" ) == 0))
    {
#ifdef HAVE_LIBTIFF
        success = write_tiff(filename, width, height, rgb_data); 
#else
        fprintf(stderr, 
                "Sorry, this program was not compiled with TIFF support\n");
        success = 0;
#endif /* HAVE_LIBTIFF */
    }

    else
    {
        fprintf(stderr, "Unknown image format\n");
        success = 0;
    }

    free(lowercase);
    fclose(outfile);

    return(success == 1);
}
