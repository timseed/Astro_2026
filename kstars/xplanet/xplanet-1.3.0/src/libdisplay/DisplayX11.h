#ifndef DisplayX11_h
#define DisplayX11_h

#include <X11/Xlib.h>

#include "DisplayBase.h"

class DisplayX11 : public DisplayBase
{
 public:
    DisplayX11(const int tr);
    virtual ~DisplayX11();

    void renderImage(PlanetProperties *planetProperties[]);

    static Window WindowID() { return(window); };

 private:
    Display *display;
    static Window window;
    Window root;

    void computeShift(unsigned long mask, unsigned char &left_shift, 
                      unsigned char &right_shift);

    Pixmap createPixmap(const unsigned char *rgb_data, 
			const int pixmap_width,
                        const int pixmap_height);

    void decomposePixmap(const Pixmap p, unsigned char *rgb);
};

#endif
