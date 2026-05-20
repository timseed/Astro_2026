#ifndef TEXTRENDERERFT2_H
#define TEXTRENDERERFT2_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "TextRenderer.h"

class DisplayBase;

class TextRendererFT2 : public TextRenderer
{
 public:
    TextRendererFT2(DisplayBase *display);
    virtual ~TextRendererFT2();

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
    FT_Library library_;
    FT_Face face_;

    FT_Glyph *glyphs_;
    FT_Vector *pos;
    FT_UInt numGlyphs_;
};

#endif
