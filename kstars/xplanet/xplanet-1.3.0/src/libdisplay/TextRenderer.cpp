#include <bitset>
#include <sstream>
#include <string>
using namespace std;

#include "Options.h"
#include "xpUtil.h"

#include "DisplayBase.h"
#include "TextRenderer.h"

TextRenderer::TextRenderer(DisplayBase *display) : display_(display), 
                                                   opacity_(1.0)
{
    Options *options = Options::getInstance();
    fontSize_ = options->FontSize();
    
    font_.assign(options->Font());
}

TextRenderer::~TextRenderer()
{
}

// x and y are the center left coordinates of the string
void 
TextRenderer::DrawText(const int x, int y, const string &text, 
                       const unsigned char color[3], const double opacity)
{
    SetText(text);
    SetOpacity(opacity);

    int textWidth, textHeight;
    TextBox(textWidth, textHeight);

    y += textHeight/2;

    DrawText(x, y, color);

    FreeText();
}

// x and y are the center left coordinates of the string
void 
TextRenderer::DrawOutlinedText(const int x, int y, const string &text, 
                               const unsigned char color[3], 
                               const double opacity)
{
    SetText(text);
    SetOpacity(opacity);

    int textWidth, textHeight;
    TextBox(textWidth, textHeight);

    y += textHeight/2;

    unsigned char black[3] = { 0, 0, 0 }; 

    DrawText(x+1, y, black);
    DrawText(x-1, y, black);
    DrawText(x, y+1, black);
    DrawText(x, y-1, black);

    DrawText(x, y, color);

    FreeText();
}

bool
TextRenderer::CheckUnicode(const unsigned long unicode, 
                           const std::vector<unsigned char> &text)
{
    int curPos;
    int numWords;
    if (unicode < 0x00000080)
    {
        curPos = 6;
        numWords = 1;
    }
    else if (unicode < 0x00000800)
    {
        curPos = 10;
        numWords = 2;
    }
    else if (unicode < 0x00010000)
    {
        curPos = 15;
        numWords = 3;
    }
    else if (unicode < 0x00200000)
    {
        curPos = 20;
        numWords = 4;
    }
    else if (unicode < 0x04000000)
    {
        curPos = 25;
        numWords = 5;
    }
    else if (unicode < 0x80000000)
    {
        curPos = 30;
        numWords = 6;
    }
    else
    {
        xpWarn("Bad unicode value\n", __FILE__, __LINE__);
        return(false);
    }
    
    // Construct the smallest UTF-8 encoding for this unicode value.
    bitset<32> uBitset(unicode);
    string utf8Val;
    if (numWords == 1) 
    {
        utf8Val += "0";
        for (int i = 0; i < 7; i++)
            utf8Val += (uBitset.test(curPos--) ? "1" : "0");
    }
    else
    {
        for (int i = 0; i < numWords; i++)
            utf8Val += "1";
        utf8Val += "0";
        for (int i = 0; i < 7 - numWords; i++)
            utf8Val += (uBitset.test(curPos--) ? "1" : "0");
        for (int j = 0; j < numWords - 1; j++)
        {
            utf8Val += "10";
            for (int i = 0; i < 6; i++)
                utf8Val += (uBitset.test(curPos--) ? "1" : "0");
        }
    }

    // Check that the input array is the "correct" array for
    // generating the derived unicode value.
    vector<unsigned char> utf8Vec;
    for (unsigned int i = 0; i < utf8Val.size(); i += 8)
    {
        string thisByte(utf8Val.substr(i, i+8));
        utf8Vec.push_back(bitset<8>(thisByte).to_ulong() & 0xff);
    }

    bool goodValue = (text.size() == utf8Vec.size());
    if (goodValue)
    {
        for (unsigned int i = 0; i < utf8Vec.size(); i++)
        {
            if (text[i] != utf8Vec[i])
            {
                goodValue = false;
                break;
            }
        }
    }

    return(goodValue);
}

unsigned long 
TextRenderer::UTF8ToUnicode(const std::vector<unsigned char> &text)
{
    unsigned long returnVal = 0xfffd;  // Unknown character 

    if (text.size() == 1) 
    {
        if (text[0] < 0x80)
        {
            returnVal = static_cast<unsigned long> (text[0] & 0x7f);
        }
        else
        {
            ostringstream errStr;
            errStr << "Multibyte UTF-8 code in single byte encoding:\n";
            for (unsigned int i = 0; i < text.size(); i++)
                errStr << hex << static_cast<int> (text[i]) << dec;
            errStr << endl;
            xpWarn(errStr.str(), __FILE__, __LINE__);
        }
        return(returnVal);
    }
    else if (text.size() > 6)
    {
        ostringstream errStr;
        errStr << "Too many bytes in UTF-8 sequence:\n";
        for (unsigned int i = 0; i < text.size(); i++)
            errStr << "(" << hex << static_cast<int> (text[i]) << dec << ")";
        errStr << endl;
        xpWarn(errStr.str(), __FILE__, __LINE__);
        return(returnVal);
    }

    bool goodChar = (text[0] >= 0xc0 && text[0] <= 0xfd);
    if (!goodChar)
    {
        ostringstream errStr;
        errStr << "Invalid leading byte in UTF-8 sequence:\n";
        for (unsigned int i = 0; i < text.size(); i++)
            errStr << "(" << hex << static_cast<int> (text[i]) << dec << ")";
        errStr << endl;
        xpWarn(errStr.str(), __FILE__, __LINE__);
        return(returnVal);
    }

    for (unsigned int i = 1; i < text.size(); i++)
    {
        goodChar = (text[i] >= 0x80 && text[i] <= 0xbf);
        if (!goodChar)
        {
            ostringstream errStr;
            errStr << "Invalid continuation byte in UTF-8 sequence:\n";
            for (unsigned int i = 0; i < text.size(); i++)
                errStr << hex << "(" << hex
                       << static_cast<int> (text[i]) << dec << ")";
            errStr << endl;
            xpWarn(errStr.str(), __FILE__, __LINE__);
            return(returnVal);
        }
    }

    bitset<8> firstByte(static_cast<unsigned char>(text[0]));

    int numBytes = 0;
    while(firstByte.test(7 - numBytes)) numBytes++;

    string binValue;
    for (int i = 6 - numBytes; i >= 0; i--)
        binValue += (firstByte.test(i) ? "1" : "0");
            
    for (int j = 1; j < numBytes; j++)
    {
        bitset<8> thisByte(static_cast<unsigned char>(text[j]));
        for (int i = 5; i >= 0; i--)
            binValue += (thisByte.test(i) ? "1" : "0");
    }

    returnVal = bitset<32>(binValue).to_ulong();

    // Check for illegal values:
    // U+D800 to U+DFFF (UTF-16 surrogates)
    // U+FFFE and U+FFFF
    if ((returnVal >= 0xd800 && returnVal <= 0xdfff)
        || (returnVal == 0xfffe || returnVal == 0xffff)
        || (!CheckUnicode(returnVal, text)))
    { 
        ostringstream errStr;
        errStr << "Malformed UTF-8 sequence:\n";
        for (unsigned int i = 0; i < text.size(); i++)
            errStr << "(" << hex << static_cast<int> (text[i]) << dec << ")";
        errStr << endl;
        xpWarn(errStr.str(), __FILE__, __LINE__);
        returnVal = 0xfffd;
    }
    return(returnVal);
}

void
TextRenderer::Font(const string &font)
{
}

void
TextRenderer::FontSize(const int size)
{
}

int
TextRenderer::FontHeight() const
{
    return(0);
}

void
TextRenderer::DrawText(const int x, const int y, 
                       const unsigned char color[3])
{
}

void
TextRenderer::SetText(const std::string &text)
{
    ostringstream errMsg;
    errMsg << "Xplanet was compiled without FreeType support. ";
    errMsg << "Ignoring text: " << text << endl;
    xpWarn(errMsg.str(), __FILE__, __LINE__);
}

void
TextRenderer::FreeText()
{
}

void
TextRenderer::TextBox(int &textWidth, int &textHeight)
{
}
