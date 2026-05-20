#include <bitset>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

#include "findFile.h"
#include "Options.h"
#include "xpDefines.h"
#include "xpUtil.h"

#include "DisplayBase.h"
#include "TextRendererFT2.h"

TextRendererFT2::TextRendererFT2(DisplayBase *display) : TextRenderer(display)
{
    glyphs_ = NULL;
    pos = NULL;
    numGlyphs_ = 0;

    const int error = FT_Init_FreeType(&library_);
    if (error)
        xpExit("Can't initialize freetype library\n", __FILE__, __LINE__);

    Options *options = Options::getInstance();
    fontSize_ = options->FontSize();

    Font(options->Font());
}

TextRendererFT2::~TextRendererFT2()
{
    FT_Done_Face(face_);
    FT_Done_FreeType(library_);

    delete [] glyphs_;
    delete [] pos;
}

void
TextRendererFT2::Font(const string &font)
{
    font_.assign(font);

    if (!findFile(font_, "fonts"))
    {
        ostringstream errStr;
        errStr << "Can't open font file " << font_ << endl;
        xpWarn(errStr.str(), __FILE__, __LINE__);
        font_ = defaultFont;
        if (!findFile(font_, "fonts"))
        {
            errStr.str("");
            errStr << "Can't open default font file " << font_ << endl;
            xpExit(errStr.str(), __FILE__, __LINE__);
        }
    }

    int error = FT_New_Face(library_, font_.c_str(), 0, &face_);
    if (error)
    {
        ostringstream errStr;
        errStr << "Can't load font " << font_ << endl;
        xpExit(errStr.str(), __FILE__, __LINE__);
    }

    error = FT_Select_Charmap(face_, ft_encoding_unicode);
    if (error)
    {
        ostringstream errStr;
        errStr << "No unicode map in font " << font_ << endl;
        xpExit(errStr.str(), __FILE__, __LINE__);
    }

    FontSize(fontSize_);
}

void
TextRendererFT2::FontSize(const int size)
{
    fontSize_ = size;
    int error = FT_Set_Pixel_Sizes(face_, 0, fontSize_);
    if (error) 
    {
        ostringstream errStr;
        errStr << "Can't set pixel size to " << fontSize_ << endl;
        xpWarn(errStr.str(), __FILE__, __LINE__);
        fontSize_ = 12;
        error = FT_Set_Pixel_Sizes(face_, 0, fontSize_);
    }
    if (error)
    {
        ostringstream errStr;
        errStr << "Can't set pixel size to " << fontSize_ << "\n";
        xpExit(errStr.str(), __FILE__, __LINE__);
    }
}

int
TextRendererFT2::FontHeight() const
{
    return(static_cast<int> (1.25 * face_->size->metrics.y_ppem));
}

void
TextRendererFT2::DrawText(const int x, const int y, 
                          const unsigned char color[3])
{
    for (unsigned int i = 0; i < numGlyphs_; i++)
    {
        FT_Glyph image;
        int error = FT_Glyph_Copy(glyphs_[i], &image);

        FT_Vector pen;
        pen.x = pos[i].x;
        pen.y = pos[i].y;
        
        error = FT_Glyph_To_Bitmap( &image, ft_render_mode_normal,
                                    &pen, 1 );

        if (!error) 
        {
            FT_BitmapGlyph bit = (FT_BitmapGlyph) image;
            FT_Bitmap bitmap = bit->bitmap;
            pen.x += (x + bit->left);
            pen.y += (y - bit->top);
            for (int j = 0; j < bitmap.rows; j++)
            {
                int istart = j * bitmap.width;
                for (int k = 0; k < bitmap.width; k++)
                {
                    if (bitmap.buffer[istart + k])
                    {
                        double opacity = opacity_ 
                            * bitmap.buffer[istart + k]/255.0;
                        display_->setPixel(pen.x + k, 
                                           pen.y + j, 
                                           color, opacity);
                    }
                }
            }
            FT_Done_Glyph(image);
        }
    }
}

void
TextRendererFT2::SetText(const std::string &text)
{
    unsigned int numChars = 0;
    
    vector<unsigned long> unicodeText;
    vector<unsigned char> utf8Text;
    for (unsigned int i = 0; i < text.size(); i++)
    {
        unsigned char thisByte = (text[i] & 0xff);
        if (thisByte < 0x80 || (thisByte >= 0xc0 && thisByte <= 0xfd))
        {
            // This is either an ASCII character or the first byte of
            // a multibyte sequence
            if (!utf8Text.empty()) 
            {
                numChars++;
                unicodeText.push_back(UTF8ToUnicode(utf8Text));
            }
            utf8Text.clear();
        }
        utf8Text.push_back(thisByte);
    }
    if (!utf8Text.empty()) 
    {   
        numChars++;
        unicodeText.push_back(UTF8ToUnicode(utf8Text));
    }

    int pen_x = 0;   /* start at (0,0) !! */
    int pen_y = 0;
    
    FT_Bool use_kerning = FT_HAS_KERNING(face_);
    FT_UInt previous = 0;
    
    delete [] glyphs_;
    delete [] pos;

    glyphs_ = new FT_Glyph[text.size()];
    pos = new FT_Vector[text.size()];
    numGlyphs_ = 0;

    for (unsigned int n = 0; n < numChars; n++ )
    {
        // convert character code to glyph index
        FT_UInt glyph_index = FT_Get_Char_Index( face_, unicodeText[n] );
        
        // retrieve kerning distance and move pen position
        if ( use_kerning && previous && glyph_index )
        {
            FT_Vector delta;
            FT_Get_Kerning( face_, previous, glyph_index,
                            ft_kerning_default, &delta );
            pen_x += delta.x >> 6;
        }

        pos[numGlyphs_].x = pen_x;
        pos[numGlyphs_].y = pen_y;

        // load glyph image into the slot. DO NOT RENDER IT !!
        int error = FT_Load_Glyph( face_, glyph_index, FT_LOAD_DEFAULT );
        if (error) continue;  // ignore errors, jump to next glyph

        // extract glyph image and store it in our table
        error = FT_Get_Glyph( face_->glyph, &glyphs_[numGlyphs_] );
        if (error) continue;  // ignore errors, jump to next glyph

        // increment pen position
        pen_x += face_->glyph->advance.x >> 6;

        // record current glyph index
        previous = glyph_index;

        numGlyphs_++;
    }
}

void
TextRendererFT2::FreeText()
{
    for (unsigned int i = 0; i < numGlyphs_; i++)
        FT_Done_Glyph(glyphs_[i]);
}

void
TextRendererFT2::TextBox(int &textWidth, int &textHeight)
{
    FT_BBox  bbox;
    
    // initialise string bbox to "empty" values
    bbox.xMin = bbox.yMin =  32000;
    bbox.xMax = bbox.yMax = -32000;
    
    // for each glyph image, compute its bounding box, translate it,
    // and grow the string bbox
    for (unsigned int i = 0; i < numGlyphs_; i++)
    {
        FT_BBox   glyph_bbox;

        FT_Glyph_Get_CBox( glyphs_[i], ft_glyph_bbox_pixels, &glyph_bbox );
        
        glyph_bbox.xMin += pos[i].x;
        glyph_bbox.xMax += pos[i].x;
        glyph_bbox.yMin += pos[i].y;
        glyph_bbox.yMax += pos[i].y;
        
        if (glyph_bbox.xMin < bbox.xMin)
            bbox.xMin = glyph_bbox.xMin;
        
        if (glyph_bbox.yMin < bbox.yMin)
            bbox.yMin = glyph_bbox.yMin;
        
        if (glyph_bbox.xMax > bbox.xMax)
            bbox.xMax = glyph_bbox.xMax;
        
        if (glyph_bbox.yMax > bbox.yMax)
            bbox.yMax = glyph_bbox.yMax;
    }
    
    // check that we really grew the string bbox
    if ( bbox.xMin > bbox.xMax )
    {
        bbox.xMin = 0;
        bbox.yMin = 0;
        bbox.xMax = 0;
        bbox.yMax = 0;    
    }

    textWidth = bbox.xMax - bbox.xMin;
    textHeight = bbox.yMax - bbox.yMin;
}
