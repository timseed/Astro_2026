#ifndef IMAGE_H
#define IMAGE_H

class Image
{
 public:
    Image();
    Image(const int w, const int h, const unsigned char *rgb, 
	  const unsigned char *alpha);

    ~Image();

    const unsigned char * getPNGAlpha() const { return(pngAlpha_); };
    const unsigned char * getRGBData() const { return(rgbData_); };

    void getPixel(double px, double py, unsigned char *pixel);
    void getPixel(double px, double py, unsigned char *pixel, 
		  unsigned char *alpha);

    int Width() const  { return(width_); };
    int Height() const { return(height_); };
    void Quality(const int q) { quality_ = q; };

    bool Read(const char *filename);
    bool Write(const char *filename);

    bool Crop(const int x0, const int y0, const int x1, const int y1);
    void Reduce(const int factor);
    void Resize(const int w, const int h);
    void Shift(const int x);

 private:
    int width_, height_, area_;
    unsigned char *rgbData_;
    unsigned char *pngAlpha_;

    int quality_;
};

#endif
