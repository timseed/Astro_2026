#include <cstdio>
#include <cstring>
using namespace std;

#include "config.h"

extern "C"
{
    int
    read_bmp(const char *filename, int *width, int *height, 
             unsigned char **rgb);
#ifdef HAVE_LIBGIF
    int
    read_gif(const char *filename, int *width, int *height, 
             unsigned char **rgb);
#endif
    
#ifdef HAVE_LIBJPEG
    int read_jpeg(const char *filename, int *width, int *height, 
                  unsigned char **rgb);
#endif
    
#ifdef HAVE_LIBPNG
    int
    read_png(const char *filename, int *width, int *height, 
             unsigned char **rgb, unsigned char **alpha);
#endif
    
#ifdef HAVE_LIBPNM
    int
    read_pnm(const char *filename, int *width, int *height, 
             unsigned char **rgb);
#endif
    
#ifdef HAVE_LIBTIFF
    int
    read_tiff(const char *filename, int *width, int *height, 
              unsigned char **rgb);
#endif
}

bool 
ReadImage(const char *filename, int &width, int &height, 
          unsigned char *&rgb_data, unsigned char *&png_alpha)
{
    char buf[4];
    unsigned char *ubuf = (unsigned char *) buf;
    int success = 0;

    FILE *file;
    file = fopen(filename, "rb");
    if (file == NULL) return(false);
  
    /* see what kind of file we have */

    fread(buf, 1, 4, file);
    fclose(file);

    if (!strncmp("BM", buf, 2))
    {
        success = read_bmp(filename, &width, &height, &rgb_data);
    }
    else if (!strncmp("GIF8", buf, 4))
    {
#ifdef HAVE_LIBGIF
        success = read_gif(filename, &width, &height, &rgb_data);
#else
        fprintf(stderr, 
                "Sorry, this program was not compiled with GIF support\n");
        success = 0;
#endif /* HAVE_LIBGIF */
    }
    else if ((ubuf[0] == 0xff) && (ubuf[1] == 0xd8))
    {
#ifdef HAVE_LIBJPEG
        success = read_jpeg(filename, &width, &height, &rgb_data);
#else
        fprintf(stderr, 
                "Sorry, this program was not compiled with JPEG support\n");
        success = 0;
#endif /* HAVE_LIBJPEG */
    }
    else if ((ubuf[0] == 0x89) && !strncmp("PNG", buf+1, 3))
    {
#ifdef HAVE_LIBPNG
        success = read_png(filename, &width, &height, &rgb_data, &png_alpha);
#else
        fprintf(stderr, 
                "Sorry, this program was not compiled with PNG support\n");
        success = 0;
#endif /* HAVE_LIBPNG */
    }
    else if ((   !strncmp("P6\n", buf, 3))
             || (!strncmp("P5\n", buf, 3))
             || (!strncmp("P4\n", buf, 3))
             || (!strncmp("P3\n", buf, 3))
             || (!strncmp("P2\n", buf, 3))
             || (!strncmp("P1\n", buf, 3)))
    {
#ifdef HAVE_LIBPNM
        success = read_pnm(filename, &width, &height, &rgb_data);
#else
        fprintf(stderr, 
                "Sorry, this program was not compiled with PNM support\n");
        success = 0;
#endif /* HAVE_LIBPNM */
    }
    else if (((!strncmp ("MM", buf, 2)) && (ubuf[2] == 0x00) 
              && (ubuf[3] == 0x2a))
             || ((!strncmp ("II", buf, 2)) && (ubuf[2] == 0x2a) 
                 && (ubuf[3] == 0x00)))
    {
#ifdef HAVE_LIBTIFF
        success = read_tiff(filename, &width, &height, &rgb_data);
#else
        fprintf(stderr, 
                "Sorry, this program was not compiled with TIFF support\n");
        success = 0;
#endif
    }
    else
    {
        fprintf(stderr, "Unknown image format\n");
        success = 0;
    }

    return(success == 1);
}
