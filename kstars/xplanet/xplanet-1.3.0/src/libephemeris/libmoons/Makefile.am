noinst_LIBRARIES = libmoons.a

AM_CPPFLAGS = -I@top_srcdir@/src

if USE_AR
  libmoons_a_AR = $(AR) cru
else
  libmoons_a_AR = $(CXX) @xplanet_ARFLAGS@
endif

libmoons_a_SOURCES = elp82b.h earth.cpp mars.cpp jupiter.cpp libmoons.h  tass17.h saturn.cpp  uranus.cpp neptune.cpp pluto.cpp


