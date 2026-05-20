SUBDIRS = libmoons

noinst_LIBRARIES = libephemeris.a

AM_CPPFLAGS = -I@top_srcdir@/src

if USE_AR
  libephemeris_a_AR = $(AR) cru
else
  libephemeris_a_AR = $(CXX) @xplanet_ARFLAGS@
endif

if HAVE_CSPICE
  spiceFiles = EphemerisSpice.cpp EphemerisSpice.h
endif

EXTRA_libephemeris_a_SOURCES = EphemerisSpice.cpp EphemerisSpice.h

libephemeris_a_SOURCES = \
			Ephemeris.cpp		\
			Ephemeris.h		\
			EphemerisHigh.cpp	\
			EphemerisHigh.h		\
			EphemerisLow.cpp	\
			EphemerisLow.h		\
			ephemerisWrapper.h	\
			ephemerisWrapper.cpp	\
			jpl_int.h		\
			jpleph.cpp		\
			jpleph.h		\
			pluto.cpp		\
			$(spiceFiles)
