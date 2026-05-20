#include <string>
using namespace std;

#include "Options.h"

#include "DisplayBase.h"
#include "TextRendererPangoFT2.h"

PangoFontMap* TextRendererPangoFT2::fontMap_ = NULL;

TextRendererPangoFT2::TextRendererPangoFT2(DisplayBase *display) 
    : TextRenderer(display), direction_(PANGO_DIRECTION_LTR)
{
    g_type_init();
    
    // There is a memory leak in the pango library.  This can be
    // minimized by making the fontMap_ member static.
    // See http://bugzilla.gnome.org/show_bug.cgi?id=143542
    if (fontMap_ == NULL) fontMap_ = pango_ft2_font_map_new();
    int dpiX = 100;
    int dpiY = 100;
    pango_ft2_font_map_set_resolution(PANGO_FT2_FONT_MAP(fontMap_), 
                                      dpiX, dpiY);
    
    context_ = pango_ft2_font_map_create_context(PANGO_FT2_FONT_MAP(fontMap_));

    pango_context_set_language(context_,
                               pango_language_from_string ("en_US"));
    pango_context_set_base_dir(context_, direction_);

    fontDescription_ = pango_font_description_new();

    Options *options = Options::getInstance();
    Font(options->Font());
    FontSize(options->FontSize());

    layout_ = pango_layout_new(context_);
    pango_layout_set_width(layout_, -1); // Don't wrap lines
}

TextRendererPangoFT2::~TextRendererPangoFT2()
{
    g_object_unref(layout_);
    pango_font_description_free(fontDescription_);
    g_object_unref(context_);
//    g_object_unref(fontMap_);
    pango_ft2_shutdown_display();
}

void
TextRendererPangoFT2::Font(const string &font)
{
    pango_font_description_set_family(fontDescription_, font.c_str());
    pango_font_description_set_style(fontDescription_, PANGO_STYLE_NORMAL);
    pango_font_description_set_variant(fontDescription_,
                                       PANGO_VARIANT_NORMAL);
    pango_font_description_set_weight(fontDescription_, PANGO_WEIGHT_NORMAL);
    pango_font_description_set_stretch(fontDescription_, 
                                       PANGO_STRETCH_NORMAL);
}

void
TextRendererPangoFT2::FontSize(const int size)
{
    pango_font_description_set_size(fontDescription_, size * PANGO_SCALE);
}

int
TextRendererPangoFT2::FontHeight() const
{
    int returnVal = 0;

    PangoRectangle rect;
    pango_layout_get_extents(layout_, NULL, &rect);

    returnVal = static_cast<int> (1.5 * PANGO_PIXELS(pango_font_description_get_size(fontDescription_)));

    return(returnVal);
}

void
TextRendererPangoFT2::DrawText(const int x, const int y, 
                               const unsigned char color[3])
{
    FT_Bitmap bitmap;

    int textWidth, textHeight;
    TextBox(textWidth, textHeight);

    unsigned char *buffer = new unsigned char[textWidth * textHeight];
    memset(buffer, 0, textWidth * textHeight);
    bitmap.rows = textHeight;
    bitmap.width = textWidth;
    bitmap.pitch = bitmap.width;
    bitmap.buffer = buffer;
    bitmap.num_grays = 256;
    bitmap.pixel_mode = ft_pixel_mode_grays;
    
    pango_ft2_render_layout(&bitmap, layout_, 0, 0);

    for (int j = 0; j < bitmap.rows; j++)
    {
        int istart = j * bitmap.width;
        for (int k = 0; k < bitmap.width; k++)
        {
            if (bitmap.buffer[istart + k])
            {
                double opacity = opacity_ * bitmap.buffer[istart + k]/255.0;
                display_->setPixel(x + k, 
                                   y + j - textHeight, 
                                   color, opacity);
            }
        }
    }

    delete [] buffer;
}

void
TextRendererPangoFT2::SetText(const std::string &text)
{
    pango_layout_set_text(layout_, text.c_str(), text.size());
    pango_layout_set_alignment(layout_, PANGO_ALIGN_LEFT);
    pango_layout_set_font_description(layout_, fontDescription_);
}

void
TextRendererPangoFT2::FreeText()
{
}

void
TextRendererPangoFT2::TextBox(int &textWidth, int &textHeight)
{
    PangoRectangle rect;
    pango_layout_get_extents(layout_, NULL, &rect);
    textWidth = PANGO_PIXELS(rect.width);
    textHeight = PANGO_PIXELS(rect.height);
}
