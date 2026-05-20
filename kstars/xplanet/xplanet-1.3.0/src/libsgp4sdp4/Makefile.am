noinst_LIBRARIES = libsgp4sdp4.a

if USE_AR
  libsgp4sdp4_a_AR = $(AR) cru
else
  libsgp4sdp4_a_AR = $(CXX) @xplanet_ARFLAGS@
endif

libsgp4sdp4_a_SOURCES =	\
	sgp4sdp4.h	\
	sgp4sdp4.c 	\
	sgp_in.c 	\
	sgp_math.c 	\
	sgp_obs.c 	\
	sgp_time.c 	\
	solar.c

EXTRA_libsgp4sdp4_a_SOURCES = main.c
