noinst_LIBRARIES = libimage.a

read = ReadImage.cpp 
write = WriteImage.cpp
bmp = bmp.c

if HAVE_LIBGIF
  gif = gif.c
endif

if HAVE_LIBJPEG
  jpeg = jpeg.c
endif

if HAVE_LIBPNG
  png = png.c
endif

if HAVE_LIBPNM
  pnm = pnm.c
endif

if HAVE_LIBTIFF
  tiff = tiff.c
endif

EXTRA_libimage_a_SOURCES = bmp.c gif.c jpeg.c png.c pnm.c tiff.c ReadImage.cpp WriteImage.cpp

AM_CPPFLAGS = -I@top_srcdir@/src

if USE_AR
  libdisplay_a_AR = $(AR) cru
  libtimer_a_AR = $(AR) cru
else
  libdisplay_a_AR = $(CXX) @xplanet_ARFLAGS@
  libtimer_a_AR = $(CXX) @xplanet_ARFLAGS@
endif

libimage_a_SOURCES = Image.cpp Image.h $(read) $(write) $(bmp) $(gif) $(jpeg) $(png) $(pnm) $(tiff)
