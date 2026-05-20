#ifndef DisplayBase_h
#define DisplayBase_h

#include <string>
#include <vector>

#include "config.h"

#define TEXTRENDERER

#ifdef TEXTRENDERER
#include "TextRenderer.h"
#else

#endif
class PlanetProperties;

class DisplayBase
{
public:
    DisplayBase(const int tr);
    virtual ~DisplayBase();

    int Width() const { return(width_); };
    int Height() const { return(height_); };

    void setPixel(const double X, const double Y,
                  const unsigned char color[3]);
    void setPixel(const int x, const int y, const unsigned int value);
    void setPixel(const int x, const int y, const unsigned char pixel[3]);
    void setPixel(const int x, const int y, const unsigned char pixel[3],
                  const double opacity);
    void setPixel(const int x, const int y, const unsigned char pixel[3],
                  const double opacity[3]);
    void getPixel(const int x, const int y, unsigned char pixel[3]) const;

    virtual void renderImage(PlanetProperties *planetProperties[]) = 0;

    const std::string & Font() const { return(textRenderer_->Font()); };
    int FontSize() const { return(textRenderer_->FontSize()); };

    void Font(const std::string &fontname);
    void FontSize(const int size);

    void setText(const std::string &text);
    void DrawText(const int x, int y, const std::string &text, 
                  const unsigned char color[3], const double opacity);
    void DrawOutlinedText(const int x, int y, const std::string &text, 
                          const unsigned char color[3], const double opacity);
    void FreeText();
    void getTextBox(int &textWidth, int &textHeight);

    virtual std::string TmpDir();

protected:
    const int times_run;

    int width_, height_;
    int area_;
    unsigned char *rgb_data;
    unsigned char *alpha;

    int fullWidth_, fullHeight_;       // pixel dimensions of the display

    void allocateRGBData();
    void drawLabel(PlanetProperties *planetProperties[]);
    void drawLabelLine(int &currentX, int &currentY, 
                       const std::string &text);
    void PlaceImageOnRoot();

    void SetBackground(const int width, const int height, 
                       unsigned char *rgb);

private:
    TextRenderer *textRenderer_;
};
#endif
