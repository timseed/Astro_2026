# Process this file with autoconf to produce a configure script.
AC_INIT(xplanet, 1.3.0)
AC_CONFIG_SRCDIR(src/xplanet.cpp)
AM_INIT_AUTOMAKE

AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

AM_CONDITIONAL([am__fastdepOBJC], false)

AC_PATH_PROG(PERL, perl, no)
AC_SUBST(PERL)

AC_ARG_WITH(cygwin,AC_HELP_STRING([--with-cygwin],
	[check for Cygwin environment (YES)]))

separator="/"
use_ar='yes'
have_aqua='no'
have_cygwin='no'
AC_CANONICAL_HOST
case "$host" in
     sparc-sun-*)
       if test "x$GXX" != "xyes"; then
# This is for Sun Workshop compiler
         use_ar='no'
         xplanet_ARFLAGS="-xar -o"
         xplanet_LDFLAGS="-xildoff"
       fi
     ;;
     mips-sgi-irix*)
# this is for the MIPSpro compilers
       if test "x$GXX" != "xyes"; then
         use_ar='no'
         xplanet_ARFLAGS="-ar -WR,-s,-u,-v -o"
       fi
     ;;
     *-cygwin*)
     if test "$with_cygwin" != 'no'; then
       AC_DEFINE(HAVE_CYGWIN,,Define if compiling under cygwin)
       have_cygwin='yes'
       xplanet_LDFLAGS="-static"
       separator="\\\\\\\\"
     fi
     ;;
     *-apple-darwin*)
     have_aqua='yes'
     ;;
     i686-pc-linux-gnu)
     ;;
esac

AC_FIND_X11
AC_FIND_XSS
AC_FIND_FREETYPE
AC_FIND_PANGO

AC_SUBST(xplanet_ARFLAGS)
AC_SUBST(xplanet_LDFLAGS)
AC_SUBST(separator)

AC_USE_MACAQUA

AC_FIND_GRAPHICS_LIBS

AC_FIND_CSPICE

if test "$have_aqua" = no; then
AM_ICONV
if test "$am_cv_func_iconv" = yes; then
  proto_iconv="$am_cv_proto_iconv_arg1"
else
  proto_iconv=""
fi
AC_FIND_LOCALE_CHARSET
fi

dnl AM_CONDITIONAL(HAVE_LIBCHARSET, test "$have_locale_charset" = 'yes')
dnl AM_CONDITIONAL(HAVE_LANGINFO_H, test "$have_langinfo_h" = 'yes')
AM_CONDITIONAL(HAVE_CSPICE, test "$have_cspice" = 'yes')
AM_CONDITIONAL(HAVE_LIBFREETYPE, test "$have_freetype" = 'yes')
AM_CONDITIONAL(HAVE_LIBPANGOFT2, test "$have_pangoft2" = 'yes')
AM_CONDITIONAL(HAVE_LIBX11, test "$have_libx11" = 'yes')
AM_CONDITIONAL(HAVE_AQUA, test "$with_aqua" = 'yes')

AM_CONDITIONAL(HAVE_LIBGIF, test "$have_gif" = 'yes')
AM_CONDITIONAL(HAVE_LIBJPEG, test "$have_jpeg" = 'yes')
AM_CONDITIONAL(HAVE_LIBPNG, test "$have_png" = 'yes')
AM_CONDITIONAL(HAVE_LIBPNM, test "$have_pnm" = 'yes')
AM_CONDITIONAL(HAVE_LIBTIFF, test "$have_tiff" = 'yes')

AM_CONDITIONAL(HAVE_CYGWIN, test "$have_cygwin" = "yes")
AM_CONDITIONAL(USE_AR, test $use_ar = 'yes')

AC_DEFINE_UNQUOTED(ICONV_CONST, $proto_iconv,
  [Define as const if the declaration of iconv() needs const.])

AC_ARG_WITH(map-extension, [  --with-map-extension=EXTENSION
                          use EXTENSION as default map extension [jpg]], 
			  mapext="$withval", mapext="jpg")
AC_SUBST(mapext)

AC_CHECK_FUNCS(unsetenv)
AC_CHECK_FUNCS(timegm)

AC_CONFIG_FILES([xpDefines.h				\
		Makefile				\
		src/Makefile				\
		src/libannotate/Makefile		\
		src/libdisplay/Makefile			\
		src/libephemeris/Makefile		\
		src/libephemeris/libmoons/Makefile	\
		src/libimage/Makefile			\
		src/libmultiple/Makefile		\
		src/libplanet/Makefile			\
		src/libprojection/Makefile		\
		src/libsgp4sdp4/Makefile])
		
AC_OUTPUT
