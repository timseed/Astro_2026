noinst_LIBRARIES = libprojection.a

AM_CPPFLAGS = -I@top_srcdir@/src

if USE_AR
  libprojection_a_AR = $(AR) cru
else
  libprojection_a_AR = $(CXX) @xplanet_ARFLAGS@
endif

libprojection_a_SOURCES = 	\
		ProjectionAncient.cpp		\
		ProjectionAncient.h		\
		ProjectionAzimuthal.cpp		\
		ProjectionAzimuthal.h		\
		ProjectionAzimutEqualArea.cpp	\
		ProjectionAzimutEqualArea.h	\
		ProjectionBase.cpp		\
		ProjectionBase.h		\
		ProjectionBonne.h		\
		ProjectionBonne.cpp		\
		ProjectionGnomonic.cpp		\
		ProjectionGnomonic.h		\
		ProjectionHemisphere.cpp	\
		ProjectionHemisphere.h		\
		ProjectionIcosagnomonic.cpp	\
		ProjectionIcosagnomonic.h	\
		ProjectionLambert.cpp		\
		ProjectionLambert.h		\
		ProjectionMercator.cpp		\
		ProjectionMercator.h		\
		ProjectionMollweide.cpp		\
		ProjectionMollweide.h		\
		ProjectionOrthographic.cpp	\
		ProjectionOrthographic.h	\
		ProjectionPeters.cpp		\
		ProjectionPeters.h		\
		ProjectionPolyconic.cpp		\
		ProjectionPolyconic.h		\
		ProjectionRectangular.cpp	\
		ProjectionRectangular.h		\
		ProjectionTSC.cpp		\
		ProjectionTSC.h			\
		getProjection.cpp		\
		libprojection.h
