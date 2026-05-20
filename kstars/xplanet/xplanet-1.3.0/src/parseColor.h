#ifndef PARSECOLOR_H
#define PARSECOLOR_H

#include <string>

extern void 
parseColor(std::string color, unsigned char RGB[3]);

extern void
parseColor(std::string color, unsigned char RGB[3], std::string &failed);

#endif
