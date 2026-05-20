#ifndef DISPLAYMSWIN_H
#define DISPLAYMSWIN_H

#include "DisplayBase.h"

class DisplayMSWin : public DisplayBase
{
 public:
    DisplayMSWin(const int tr);
    virtual ~DisplayMSWin();

    void renderImage(PlanetProperties *planetProperties[]);

    std::string TmpDir();
};

#endif
