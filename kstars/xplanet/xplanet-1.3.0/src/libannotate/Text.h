#ifndef TEXT_H
#define TEXT_H

#include <string>

#include "Annotation.h"

class Text : public Annotation
{
public:
    Text(const unsigned char color[3], 
         const int x, const int y, 
         const int iconWidth, const int iconHeight,
         const int align, 
         const std::string &text);

    virtual ~Text();

    int Align() const { return(align_); };
    void Align(const int align);
    void ComputeBoundingBox(DisplayBase *display);
    bool FixedAlign() const { return(fixedAlign_); };
    void Font(const std::string &font) { font_.assign(font); };
    void FontSize(const int fontSize) { fontSize_ = fontSize; };

    void Opacity(const double d) { opacity_ = d; };

    void Outline(const bool b) { outlined_ = b; };

    int Overhang(const int width, const int height);
    int Overlap(const Text *const t);

    void X(const int x) { x_ = x; } ;
    int X() const { return(x_); };

    virtual void Shift(const int x) { x_ += x; };
    virtual void Draw(DisplayBase *display);

private:

    int align_;
    bool fixedAlign_;
    
    std::string font_;
    int fontSize_;

    const int iconHeight_;
    const int iconWidth_;

    bool needAlign_;
    bool needBoundingBox_;

    double opacity_;

    // (xOffset, yOffset) is the offset of the text from (x_, y_)
    int xOffset_;
    int yOffset_;

    bool outlined_;

    std::string text_;
    int textHeight_;
    int textWidth_;

    // (x_, y_) is the center of the icon
    int x_;
    const int y_;

    int ulx_, uly_;
    int lrx_, lry_;

    int Overlap(const int ulx, const int uly, const int lrx, const int lry);
};

#endif
