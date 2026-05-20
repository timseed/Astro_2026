#ifndef TEXTRENDERERPANGO_H
#define TEXTRENDERERPANGO_H

#include <pango/pango.h>
#include <pango/pangoft2.h>

#include "TextRenderer.h"

class DisplayBase;

class TextRendererPangoFT2 : public TextRenderer
{
 public:
    TextRendererPangoFT2(DisplayBase *display);
    virtual ~TextRendererPangoFT2();

    virtual void Font(const std::string &font);
    const std::string & Font() const { return(font_); };

    virtual int FontHeight() const;

    virtual void FontSize(const int size);
    int FontSize() const { return(fontSize_); };

    virtual void DrawText(const int x, const int y, 
                          const unsigned char color[3]);

    virtual void SetText(const std::string &text);
    virtual void FreeText();

    virtual void TextBox(int &textWidth, int &textHeight);

 private:
    PangoContext *context_;
    PangoDirection direction_;
    PangoFontDescription *fontDescription_;
    static PangoFontMap *fontMap_;
    PangoLayout *layout_;
};

#endif
