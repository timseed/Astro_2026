#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

#include "keywords.h"
#include "Options.h"
#include "xpUtil.h"

#include "Text.h"

#include "libdisplay/libdisplay.h"

Text::Text(const unsigned char color[3], 
           const int x, const int y, 
           const int iconWidth, const int iconHeight,
           const int align, 
           const std::string &text)
    : Annotation(color), align_(align), font_(""), fontSize_(-1), 
      iconHeight_(iconHeight), iconWidth_(iconWidth), needAlign_(true),
      needBoundingBox_(true), opacity_(1.0), outlined_(true), 
      text_(text), x_(x), y_(y)
{
    if (align_ == AUTO)
    {
        fixedAlign_ = false;
        align_ = RIGHT;
    }
    else
    {
        fixedAlign_ = true;
    }
}

Text::~Text()
{
}

void
Text::ComputeBoundingBox(DisplayBase *display)
{
    string saveFont = display->Font();
    int saveFontSize = display->FontSize();
    
    if (!font_.empty())
        display->Font(font_);
    
    if (fontSize_ < 0)
        display->FontSize(saveFontSize);
    else
        display->FontSize(fontSize_);
    
    display->setText(text_);
    display->getTextBox(textWidth_, textHeight_);
    display->FreeText();
    
    if (!font_.empty())
        display->Font(saveFont);
    
    if (fontSize_ > 0)
        display->FontSize(saveFontSize);

    needBoundingBox_ = false;

    if (needAlign_) Align(align_);
}

int
Text::Overlap(const int ulx, const int uly, const int lrx, const int lry)
{
    if (ulx_ > lrx || lrx_ < ulx) return(0);
    if (uly_ > lry || lry_ < uly) return(0);

    int width, height;
    if (ulx_ > ulx)
        width = min(lrx, lrx_) - ulx_;
    else
        width = min(lrx, lrx_) - max(ulx, ulx_);

    if (uly_ > uly)
        height = min(lry, lry_) - uly_;
    else
        height = min(lry, lry_) - max(uly, uly_);

    return(width * height);
}

int 
Text::Overlap(const Text *const t)
{
    // compute the overlap between the label and the other label's icon
    int ulx = t->x_ - t->iconWidth_/2;
    int uly = t->y_ - t->iconHeight_/2;
    int lrx = ulx + t->iconWidth_;
    int lry = uly + t->iconHeight_;

    int overlap = Overlap(ulx, uly, lrx, lry);

    // compute the overlap between the label and the other label's text
    overlap += Overlap(t->ulx_, t->uly_, t->lrx_, t->lry_);

    return(overlap);
}

// compute how much of the text falls off of the screen
int
Text::Overhang(const int width, const int height)
{
    const int topOverhang = max(0, -uly_);
    const int bottomOverhang = max(0, lry_ - height);
    const int rightOverhang = max(0, lrx_ - width);
    const int leftOverhang = max(0, -ulx_);

    int returnVal = max(topOverhang, bottomOverhang) * textWidth_;
    returnVal += max(rightOverhang, leftOverhang) * textHeight_;

    return(returnVal);
}

void
Text::Align(const int align)
{
    if (needBoundingBox_)
        align_ = RIGHT;
    else
        align_ = align;

    switch (align_)
    {
    case RIGHT:
        xOffset_ = iconWidth_/2 + 2;
        yOffset_ = 0;
        break;
    case LEFT:
        xOffset_ = -(iconWidth_/2 + textWidth_ + 2);
        yOffset_ = 0;
        break;
    case ABOVE:
        xOffset_ = -textWidth_/2;
        yOffset_ = -(iconHeight_ + textHeight_)/2 - 2;
        break;
    case BELOW:
        xOffset_ = -textWidth_/2;
        yOffset_ = (iconHeight_ + textHeight_)/2 + 2;
        break;
    case CENTER:
        xOffset_ = -textWidth_/2;
        yOffset_ = 0;
        break;
    default:
    {
        ostringstream errStr;
        errStr << "Unknown alignment for marker " << text_
               << ", using RIGHT\n";
        xpWarn(errStr.str(), __FILE__, __LINE__);
        
        xOffset_ = iconWidth_/2 + 2;
        yOffset_ = 0;
    }
    }

    // Compute the corners of the text
    ulx_ = x_ + xOffset_ - 1;
    uly_ = y_ + yOffset_ - textHeight_/2 - 1;
    lrx_ = ulx_ + textWidth_ + 1;
    lry_ = uly_ + textHeight_ + 1;

    needAlign_ = false;
}

void
Text::Draw(DisplayBase *display)
{
    string saveFont = display->Font();
    int saveFontSize = display->FontSize();

    if (!font_.empty())
        display->Font(font_);
    
    if (fontSize_ > 0)
        display->FontSize(fontSize_);

    if (needBoundingBox_) ComputeBoundingBox(display);
    if (needAlign_) Align(align_);

    Options *options = Options::getInstance();
    string markerBounds(options->MarkerBounds());
    if (!markerBounds.empty())
    {
        // gcc 2.95 complains about ios_base::app
        ofstream outfile(markerBounds.c_str(), ios::app);
        if (outfile.is_open())
        {
            if (x_ < display->Width() && x_ > -(lrx_ - ulx_))
                outfile << ulx_ << "," << uly_ << " "
                        << lrx_ << "," << lry_ << "\t" 
                        << text_ << endl;
            outfile.close();
/*
            for (int i = ulx_; i <= lrx_; i++)
            {
                display->setPixel(i, uly_, 255);
                display->setPixel(i, lry_, 255);
            }
            
            for (int i = uly_; i <= lry_; i++)
            {
                display->setPixel(ulx_, i, 255);
                display->setPixel(lrx_, i, 255);
            }
*/
        }
        else
        {
            ostringstream errStr;
            errStr << "Can't open markerbounds file " << markerBounds
                   << " for output\n";
            xpWarn(errStr.str(), __FILE__, __LINE__);
        }
    }

    if (outlined_)
        display->DrawOutlinedText(x_ + xOffset_, y_ + yOffset_, 
                                  text_, color_, opacity_);
    else
        display->DrawText(x_ + xOffset_, y_ + yOffset_, 
                          text_, color_, opacity_);

    if (!font_.empty())
        display->Font(saveFont);

    if (fontSize_ > 0)
        display->FontSize(saveFontSize);
}
