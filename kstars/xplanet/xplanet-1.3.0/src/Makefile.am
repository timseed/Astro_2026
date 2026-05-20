SUBDIRS = libannotate libdisplay libephemeris libimage libmultiple libplanet libprojection libsgp4sdp4

bin_PROGRAMS = xplanet

EXTRA_xplanet_SOURCES = ParseGeom.c ParseGeom.h

if !HAVE_LIBX11
  parsegeom = ParseGeom.c ParseGeom.h
endif

AM_CPPFLAGS = -DDATADIR=\"$(datadir)@separator@xplanet\" @X_CFLAGS@ @FREETYPE_CFLAGS@ 
AM_LDFLAGS = @xplanet_LDFLAGS@

xplanet_SOURCES = \
	Map.cpp			\
	Map.h			\
	Options.cpp		\
	Options.h		\
	PlanetProperties.h	\
	PlanetProperties.cpp	\
	Ring.cpp		\
	Ring.h			\
	Satellite.h		\
	Satellite.cpp		\
	Separation.h		\
	Separation.cpp		\
	View.cpp		\
	View.h			\
	body.h			\
	buildPlanetMap.h	\
	buildPlanetMap.cpp	\
	createMap.h		\
	createMap.cpp		\
	drawMultipleBodies.cpp	\
	drawProjection.cpp	\
	findBodyXYZ.h		\
	findBodyXYZ.cpp		\
	findFile.h		\
	findFile.cpp		\
	getopt.c		\
	getopt.h		\
	getopt1.c		\
	keywords.h		\
	parse.h			\
	parse.cpp  		\
	parseColor.h		\
	parseColor.cpp		\
	printVersion.cpp	\
	readConfig.cpp		\
	readOriginFile.h	\
	readOriginFile.cpp	\
	satrings.h		\
	ssec.cpp		\
	setPositions.h		\
	setPositions.cpp	\
	sphericalToPixel.h	\
	sphericalToPixel.cpp	\
	xpGetopt.h		\
	xpUtil.cpp		\
	xpUtil.h		\
	xplanet.cpp		\
	$(parsegeom)

xplanet_LDADD =	libannotate/libannotate.a		\
		libdisplay/libdisplay.a 		\
		libdisplay/libtimer.a 			\
		libephemeris/libephemeris.a 		\
		libephemeris/libmoons/libmoons.a	\
		libimage/libimage.a 			\
		libmultiple/libmultiple.a		\
		libplanet/libplanet.a 			\
		libprojection/libprojection.a 		\
		libsgp4sdp4/libsgp4sdp4.a		\
		@GRAPHICS_LIBS@ @CSPICE_LIBS@ @X_LIBS@ 	\
		@XSS_LIBS@ @FREETYPE_LIBS@ @AQUA_LIBS@ 	\
		@LIBICONV@ @LIBCHARSET@
