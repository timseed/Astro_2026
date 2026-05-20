#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
using namespace std;

#include "config.h"
#include "keywords.h"
#include "Options.h"
#include "PlanetProperties.h"
#include "xpUtil.h"

#include "DisplayX11.h"
#include "TimerX11.h"

#include "vroot.h"

// The window is static because it needs to stay open between
// renderings.
Window DisplayX11::window;

DisplayX11::DisplayX11(const int tr) : DisplayBase(tr)
{
    Options *options = Options::getInstance();

    if (options->DisplayMode() == WINDOW)
        display = TimerX11::DisplayID();
    else
        display = XOpenDisplay(NULL);

    if (display == NULL)
        xpExit("Can't open X display\n", __FILE__, __LINE__);

    const int screen_num    = DefaultScreen(display);

    if (options->VirtualRoot())
        root = VirtualRootWindowOfScreen(ScreenOfDisplay(display, screen_num));
    else
        root = ScreenOfDisplay(display, screen_num)->root;
    
    XWindowAttributes xgwa;
    XGetWindowAttributes(display, root, &xgwa);
    fullWidth_  = xgwa.width;
    fullHeight_ = xgwa.height;

    switch (options->DisplayMode())
    {
    case WINDOW:
    {
        if (options->XID())
        {
            window = static_cast<Window> (options->XID());
        }
        else
        {
            width_ = options->getWidth();
            height_ = options->getHeight();
            
            if (times_run == 0)
            {
                int x = options->getWindowX();
                int y = options->getWindowY();
                if (options->GeometryMask() & XNegative) 
                    x += (fullWidth_ - width_);
                if (options->GeometryMask() & YNegative) 
                    y += (fullHeight_ - height_);
            
                window = XCreateSimpleWindow(display, root, x, y, 
                                             width_, height_, 4,
                                             WhitePixel(display, screen_num),
                                             BlackPixel(display, screen_num));
            
                if (options->GeometryMask() != NoValue)
                {
                    XSizeHints *hints = XAllocSizeHints();
                    hints->flags = USPosition;
                    XSetWMNormalHints(display, window, hints);
                }

                string title;
                if (options->WindowTitle().empty()) 
                {
                    title.assign("Xplanet ");
                    title += VERSION;
                }
                else
                {
                    title.assign(options->WindowTitle());
                }

                XTextProperty windowName;
                char *titlec = (char *) title.c_str();
                XStringListToTextProperty(&titlec, 1, &windowName);
                XSetWMName(display, window, &windowName);       

		// Add X Class Hint
		// contributed by Dragan Stanojevic - Nevidljivi <invisible@hidden-city.net>
		XClassHint classHint;

		classHint.res_name  = "xplanet";
		classHint.res_class = "XPlanet";

		XSetClassHint(display, window, &classHint);
            }
        }
        XGetWindowAttributes(display, window, &xgwa);
        width_  = xgwa.width;
        height_ = xgwa.height;
    }
    break;
    case ROOT:
        if (options->GeometrySelected())
        {
            width_ = options->getWidth();
            height_ = options->getHeight();
        }
        else
        {
            width_ = fullWidth_;
            height_ = fullHeight_;
        }
        
        window = root;
        break;
    default:
        xpExit("DisplayX11: Unknown display mode?\n", __FILE__, __LINE__);
    }

    if (!options->CenterSelected())
    {
        if (width_ % 2 == 0)
            options->CenterX(width_/2 - 0.5);
        else
            options->CenterX(width_/2);

        if (height_ % 2 == 0)
            options->CenterY(height_/2 - 0.5);
        else
            options->CenterY(height_/2);
    }

    allocateRGBData();
}

DisplayX11::~DisplayX11()
{
}

void
DisplayX11::renderImage(PlanetProperties *planetProperties[])
{
    drawLabel(planetProperties);

    Options *options = Options::getInstance();

    Pixmap pixmap;
        
    switch (options->DisplayMode())
    {
    case WINDOW:
        pixmap = createPixmap(rgb_data, width_, height_);
        XMapWindow(display, window);
        break;
    case ROOT:
        if (options->GeometrySelected()) PlaceImageOnRoot();

        pixmap = createPixmap(rgb_data, fullWidth_, fullHeight_);

        if (options->Transparency())
        {
            // Set the background pixmap for Eterms and aterms.  This
            // code is taken from the Esetroot source.
            Atom prop_root, prop_esetroot, type;
            int format;
            unsigned long length, after;
            unsigned char *data_root, *data_esetroot;
            
            prop_root = XInternAtom(display, "_XROOTPMAP_ID", True);
            prop_esetroot = XInternAtom(display, "ESETROOT_PMAP_ID", True);
            
            if (prop_root != None && prop_esetroot != None) 
            {
                XGetWindowProperty(display, root, prop_root, 
                                   0L, 1L, False, AnyPropertyType,
                                   &type, &format, &length, 
                                   &after, &data_root);
                if (type == XA_PIXMAP) 
                {
                    XGetWindowProperty(display, root, prop_esetroot, 
                                       0L, 1L, False, AnyPropertyType,
                                       &type, &format, &length, 
                                       &after, &data_esetroot);
                    if (data_root && data_esetroot) 
                    {
                        if (type == XA_PIXMAP 
                            && *((Pixmap *) data_root) == *((Pixmap *) data_esetroot)) 
                        {
                            XKillClient(display, *((Pixmap *) data_root));
                        }
                    }
                }
            }
            /* This will locate the property, creating it if it
             * doesn't exist */
            prop_root = XInternAtom(display, "_XROOTPMAP_ID", False);
            prop_esetroot = XInternAtom(display, "ESETROOT_PMAP_ID", False);
            
            /* The call above should have created it.  If that failed,
             * we can't continue. */
            if (prop_root == None || prop_esetroot == None) 
            {
                xpWarn("Can't set pixmap for transparency\n", 
                       __FILE__, __LINE__);
            }
            else
            {
                XChangeProperty(display, root, prop_root, XA_PIXMAP, 
                                32, PropModeReplace,
                                (unsigned char *) &pixmap, 1);
                XChangeProperty(display, root, prop_esetroot, XA_PIXMAP,
                                32, PropModeReplace,
                                (unsigned char *) &pixmap, 1);
                XSetCloseDownMode(display, RetainPermanent);
                XFlush(display);
            }
        }
        break;
    default:
        xpExit("Unknown X11 display mode?\n", __FILE__, __LINE__);
    }

    XSetWindowBackgroundPixmap(display, window, pixmap);
    if (!options->Transparency()) XFreePixmap(display, pixmap);
    XClearWindow(display, window);
    XFlush(display);

    if (options->DisplayMode() == ROOT) XCloseDisplay(display);
}

void
DisplayX11::computeShift(unsigned long mask, 
                         unsigned char &left_shift, 
                         unsigned char &right_shift)
{
    left_shift = 0;
    right_shift = 8;
    if (mask != 0)
    {
        while ((mask & 0x01) == 0)
        {
            left_shift++;
            mask >>= 1;
        }
        while ((mask & 0x01) == 1)
        {
            right_shift--;
            mask >>= 1;
        }
    }
}

Pixmap
DisplayX11::createPixmap(const unsigned char *rgb, 
                         const int pixmap_width, const int pixmap_height)
{
    int i, j;   // loop variables 

    const int screen_num = DefaultScreen(display);

    const int depth = DefaultDepth(display, screen_num);
    Visual *visual = DefaultVisual(display, screen_num);
    Colormap colormap = DefaultColormap(display, screen_num);

    Pixmap tmp = XCreatePixmap(display, window, pixmap_width, pixmap_height, 
                               depth);

    char *pixmap_data = NULL;
    switch (depth)
    {
    case 32:
    case 24:
        pixmap_data = new char[4 * pixmap_width * pixmap_height];
        break;
    case 16:
    case 15:
        pixmap_data = new char[2 * pixmap_width * pixmap_height];
        break;
    case 8:
        pixmap_data = new char[pixmap_width * pixmap_height];
        break;
    default:
        break;
    }

    XImage *ximage = XCreateImage(display, visual, depth, ZPixmap, 0,
                                  pixmap_data, pixmap_width, pixmap_height, 
                                  8, 0);

    int entries;
    XVisualInfo v_template;
    v_template.visualid = XVisualIDFromVisual(visual);
    XVisualInfo *visual_info = XGetVisualInfo(display, VisualIDMask, 
                                              &v_template, &entries);

    unsigned long ipos = 0;
    switch (visual_info->c_class)
    {
    case PseudoColor:  
    {
        XColor xc;
        xc.flags = DoRed | DoGreen | DoBlue;

        int num_colors = 256;
        XColor *colors = new XColor[num_colors];
        for (i = 0; i < num_colors; i++) colors[i].pixel = (unsigned long) i;
        XQueryColors(display, colormap, colors, num_colors);
        
        int *closest_color = new int[num_colors];

        for (i = 0; i < num_colors; i++)
        {
            xc.red = (i & 0xe0) << 8;           // highest 3 bits
            xc.green = (i & 0x1c) << 11;        // middle 3 bits
            xc.blue = (i & 0x03) << 14;         // lowest 2 bits

            // find the closest color in the colormap
            double distance, distance_squared, min_distance = 0;
            for (int ii = 0; ii < num_colors; ii++)
            {
                distance = colors[ii].red - xc.red;
                distance_squared = distance * distance;
                distance = colors[ii].green - xc.green;
                distance_squared += distance * distance;
                distance = colors[ii].blue - xc.blue;
                distance_squared += distance * distance;
                
                if ((ii == 0) || (distance_squared <= min_distance))
                {
                    min_distance = distance_squared;
                    closest_color[i] = ii;
                }
            }
        }

        for (j = 0; j < pixmap_height; j++)
        {
            for (i = 0; i < pixmap_width; i++)
            {
                xc.red = (unsigned short) (rgb[ipos++] & 0xe0);
                xc.green = (unsigned short) (rgb[ipos++] & 0xe0);
                xc.blue = (unsigned short) (rgb[ipos++] & 0xc0);

                xc.pixel = xc.red | (xc.green >> 3) | (xc.blue >> 6);
                XPutPixel(ximage, i, j, 
                          colors[closest_color[xc.pixel]].pixel);
            }
        }
        delete [] colors;
        delete [] closest_color;
    }
    break;
    case TrueColor:
    {
        unsigned char red_left_shift;
        unsigned char red_right_shift;
        unsigned char green_left_shift;
        unsigned char green_right_shift;
        unsigned char blue_left_shift;
        unsigned char blue_right_shift;

        computeShift(visual_info->red_mask, red_left_shift, 
                     red_right_shift);
        computeShift(visual_info->green_mask, green_left_shift, 
                     green_right_shift);
        computeShift(visual_info->blue_mask, blue_left_shift, 
                     blue_right_shift);

        unsigned long pixel;
        unsigned long red, green, blue;
        for (j = 0; j < pixmap_height; j++)
        {
            for (i = 0; i < pixmap_width; i++)
            {
                red = (unsigned long) 
                    rgb[ipos++] >> red_right_shift;
                green = (unsigned long) 
                    rgb[ipos++] >> green_right_shift;
                blue = (unsigned long) 
                    rgb[ipos++] >> blue_right_shift;
                
                pixel = (((red << red_left_shift) & visual_info->red_mask)
                         | ((green << green_left_shift) 
                            & visual_info->green_mask)
                         | ((blue << blue_left_shift) 
                            & visual_info->blue_mask));

                XPutPixel(ximage, i, j, pixel);
            }
        }
    }
    break;
    default:
    {
        ostringstream errStr;
        errStr << "createPixmap: visual = " << visual_info->c_class << endl
               << "Visual should be either PseudoColor or TrueColor\n";
        xpWarn(errStr.str(), __FILE__, __LINE__);
        return(tmp);
    }
    }
    
    GC gc = XCreateGC(display, window, 0, NULL);
    XPutImage(display, tmp, gc, ximage, 0, 0, 0, 0, pixmap_width, 
              pixmap_height);

    XFreeGC(display, gc);

    XFree(visual_info);

    delete [] pixmap_data;

    // Set ximage data to NULL since pixmap data was deallocated above
    ximage->data = NULL;
    XDestroyImage(ximage);

    return(tmp);
}

void
DisplayX11::decomposePixmap(const Pixmap p, unsigned char *rgb)
{
    int i, j;
    unsigned long ipos = 0;

    const int screen_num = DefaultScreen(display);
    Visual *visual = DefaultVisual(display, screen_num);

    int entries;
    XVisualInfo v_template;
    v_template.visualid = XVisualIDFromVisual(visual);
    XVisualInfo *visual_info = XGetVisualInfo(display, VisualIDMask,
                                              &v_template, &entries);

    Colormap colormap = DefaultColormap(display, screen_num);

    XImage *ximage = XGetImage(display, p, 0, 0, width_, 
                               height_, AllPlanes, ZPixmap);

    switch (visual_info->c_class)
    {
    case PseudoColor:
    {
        XColor *xc = new XColor[width_];

        for (j = 0; j < height_; j++)
        {
            for (i = 0; i < width_; i++)
                xc[i].pixel = XGetPixel(ximage, i, j);

            XQueryColors(display, colormap, xc, width_);

            for (i = 0; i < width_; i++)
            {
                rgb[ipos++] = (unsigned char) (xc[i].red >> 8);
                rgb[ipos++] = (unsigned char) (xc[i].green >> 8);
                rgb[ipos++] = (unsigned char) (xc[i].blue >> 8);
            }
        }
        delete [] xc;
    }
    break;
    case TrueColor:
    {
        unsigned char red_left_shift;
        unsigned char red_right_shift;
        unsigned char green_left_shift;
        unsigned char green_right_shift;
        unsigned char blue_left_shift;
        unsigned char blue_right_shift;
        
        computeShift(visual_info->red_mask, red_left_shift, 
                     red_right_shift);
        computeShift(visual_info->green_mask, green_left_shift, 
                     green_right_shift);
        computeShift(visual_info->blue_mask, blue_left_shift, 
                     blue_right_shift);
        
        unsigned long pixel;
        for (j = 0; j < height_; j++)
        {
            for (i = 0; i < width_; i++)
            {
                pixel = XGetPixel(ximage, i, j);
                rgb[ipos++] = ((unsigned char) 
                               (((pixel & visual_info->red_mask) 
                                 >> red_left_shift)
                                << red_right_shift));
                rgb[ipos++] = ((unsigned char) 
                               (((pixel & visual_info->green_mask) 
                                 >> green_left_shift)
                                << green_right_shift));
                rgb[ipos++] = ((unsigned char) 
                               (((pixel & visual_info->blue_mask) 
                                 >> blue_left_shift)
                                << blue_right_shift));
            }
        }
    }
    break;
    default:
    {
        ostringstream errStr;
        errStr << "decomposePixmap: visual = " << visual_info->c_class << endl
               << "Visual should be either PseudoColor or TrueColor\n";
        xpWarn(errStr.str(), __FILE__, __LINE__);
    }
    }

    XFree(visual_info);
    XDestroyImage(ximage);
}
