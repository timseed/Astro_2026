#ifndef ANNOTATION_H
#define ANNOTATION_H

class DisplayBase;

class Annotation
{
 public:
    Annotation();

    Annotation(const unsigned char color[3]);

    virtual ~Annotation();

    int Width() const { return(width_); };
    int Height() const { return(height_); };

    virtual void Shift(const int x) = 0;
    virtual void Draw(DisplayBase *display) = 0;

 protected:

    unsigned char color_[3];
    int width_, height_;
};

#endif
