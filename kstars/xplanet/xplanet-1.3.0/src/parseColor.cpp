#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
using namespace std;

#include "findFile.h"
#include "xpUtil.h"

static map<string, unsigned int> RGBColors;

static void
buildColorMap()
{
    string RGBFile("rgb.txt");
    bool foundFile = findFile(RGBFile, "");
    if (!foundFile)
    {
        ostringstream errStr;
        errStr << "Can't load RGB file " << RGBFile << "\n";
        xpWarn(errStr.str(), __FILE__, __LINE__);
        return;
    }

    char line[128], name[128];
    int r, g, b;

    ifstream infile(RGBFile.c_str());
    while (infile.getline(line, 128))
    {
        if (sscanf(line, "%d %d %d %[^\n]\n", &r, &g, &b, name) != 4) 
            continue;

        if ((r < 0 || r > 255) || (g < 0 || g > 255) || (b < 0 || g > 255))
            continue;

        // strip off DOS end-of-line
        char *ptr = strchr(name, 13);
        if (ptr != NULL) *ptr = '\0';

        // convert to lower case
        for (ptr = name; *ptr != '\0'; ptr++) *ptr = tolower(*ptr);

        unsigned int c = (r << 16) + (g << 8) + b;
        
        RGBColors.insert(make_pair(string(name), c));
    }
    infile.close();
}

void
parseColor(string color, unsigned char RGB[3], string &failed)
{
    if (RGBColors.empty()) buildColorMap();

    string defaultcolor = "red";
    if (color.empty()) color = defaultcolor;

    unsigned int value = 0xff0000;

    if (color[0] == '0' && color[1] == 'x')
    {
        value = strtoul(color.c_str(), NULL, 16);
    }
    else
    {
        // Convert to lower case
        for (unsigned int i = 0; i < color.size(); i++)
            color[i] = tolower(color[i]);
        
        // There's a DarkSlateGray but no DarkSlateGrey
        string::size_type grey = color.find("grey");
        if (grey != string::npos && grey < color.size()) 
            color[grey+2] = 'a';
        
        memset(RGB, 0, 3);
        
        map<string, unsigned int>::iterator p;
        p = RGBColors.find(color);
        if (p != RGBColors.end())
        {
            value = p->second;
        }
        else
        {
            if (color.compare(defaultcolor) != 0)
            {
                ostringstream errStr;
                errStr << "Can't find color " << color << ", using "
                       << defaultcolor << "\n";
                failed.assign(errStr.str());
            }
        }
    }

    RGB[0] = (unsigned char) ((value & 0xff0000) >> 16);
    RGB[1] = (unsigned char) ((value & 0x00ff00) >> 8);
    RGB[2] = (unsigned char)  (value & 0x0000ff);
}

void
parseColor(string color, unsigned char RGB[3])
{
    string failed;
    parseColor(color, RGB, failed);

    if (!failed.empty())
        xpWarn(failed, __FILE__, __LINE__);
}
