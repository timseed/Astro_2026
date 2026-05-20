noinst_LIBRARIES = libmultiple.a

AM_CPPFLAGS = -I@top_srcdir@/src @FREETYPE_CFLAGS@

if USE_AR
  libmultiple_a_AR = $(AR) cru
else
  libmultiple_a_AR = $(CXX) @xplanet_ARFLAGS@
endif

libmultiple_a_SOURCES = 	\
	libmultiple.h 		\
	addOrbits.cpp 		\
	RayleighScattering.h	\
	RayleighScattering.cpp	\
	drawSphere.cpp		\
	drawEllipsoid.cpp	\
	drawRings.cpp 		\
	drawStars.cpp 		\
	drawSunGlare.cpp
