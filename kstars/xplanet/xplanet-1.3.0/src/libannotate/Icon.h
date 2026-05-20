#ifndef ICON_H
#define ICON_H

#include <string>

#include "Annotation.h"

class Image;

class Icon : public Annotation
{
 public:
    Icon(const int x, const int y, const std::string &filename, 
	 const unsigned char *transparent);

    virtual ~Icon();

    virtual void Shift(const int x) { x_ += x; };
    virtual void Draw(DisplayBase *display);

 private:

    int x_;
    const int y_;
    std::string filename_;
    Image *image_;
    unsigned char *transparent_;
};

#endif
