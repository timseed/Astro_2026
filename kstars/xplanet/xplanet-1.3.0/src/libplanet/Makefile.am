noinst_LIBRARIES = libplanet.a

AM_CPPFLAGS = -I@top_srcdir@/src

if USE_AR
  libplanet_a_AR = $(AR) cru
else
  libplanet_a_AR = $(CXX) @xplanet_ARFLAGS@
endif

libplanet_a_SOURCES = Planet.cpp Planet.h

