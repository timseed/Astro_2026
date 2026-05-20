To compile Xplanet with spice support, use the --with-cspice option
with the configure script.  The location of the header files should be
in your CPPFLAGS environment variable and the location of libcspice.a
should be in the LIBS environment variable.

The source code for the SPICE toolkit may be found at
ftp://naif.jpl.nasa.gov/pub/naif/toolkit
Remember to rename or copy cspice.a to libcspice.a so that the
configure script will find it.

SPICE kernels and other files go in this directory, or in a
subdirectory named "spice" of a directory specified by -searchdir.
Files for many missions may be found at
ftp://naif.jpl.nasa.gov/pub/naif

Most people will probably be interested in SPK (Spacecraft Kernel)
files for each mission.  Other files of interest are the leapsecond
file (e.g. naif0007.tls) and the P_constants file (e.g. pck00007.tpc).

Use the -spice_file option to use the SPICE kernels.  For example, to
look at Saturn as seen from the Cassini spacecraft:

xplanet -spice_file cassini -origin naif-82 -target saturn

A file named cassini must exist (it can be empty) as well as a file
named cassini.krn containing a list of kernel files to read, one per
line.  You will have to figure out which kernel files you need.

Kernel files are loaded in the order listed in the .krn file.  If a 
particular body's ephemeris is in more than one kernel file, the
last appropriate kernel file loaded will be used.  In general, put the
most recent kernel files last in the .krn file.

To display the Mars Global Surveyor orbiting around Mars, seen from
the earth:

xplanet -spice_file mgs -origin earth -body mars

Valid keywords are "align", "color", "font", "fontsize", "image",
"relative_to", "thickness", "trail", and "transparent".  In addition,
a string to be plotted with the marker may be enclosed in either
double quotes (""), or braces ({}).  If a string is not supplied, the
marker will take the name of the object from the SPICE kernel.

If the "relative_to" keyword is not supplied, the position of the
object is computed relative to the Sun.  Normally the best thing to do
is to compute the position of the object relative to the planet it
orbits.  The index must be the NAIF id.  When using "relative_to",
the right kernel files must be loaded to properly calculate the
object's position relative to the desired body.  

For example, the kernel file 971103_SCEPH_LP0_SP0.bsp contains values
to compute the position of the Cassini orbiter relative to the Sun.
In order to compute its position relative to Saturn, you must supply
the proper kernel files to compute Saturn's position relative to the
Sun.  Note that some kernel files give the position of the Saturn
barycenter, which is the center of mass of the Saturn system, not
Saturn itself.

A few example lines in the spice file are given below.  In all cases,
the names of the appropriate SPICE kernels must be listed in the .krn
file.

-94

This plots the Mars Global Surveyor orbiter relative to the Sun.
Since Xplanet uses a low precision ephemeris for Mars by default, it
probably won't be in the right place relative to Mars.  You can use
-spice_ephemeris 499 (499 is the NAIF id for Mars) on the command
line, or else use the "relative_to" keyword:

-94 relative_to=499 "MGS" image=mgs.png trail={-.01,0.0001,.001}

This draws the image mgs.png at the location of the MGS orbiter, with
an orbit trail from 0.01 days in the past to 0.0001 days in the
future.  The trail is just a series of line segments connected every
0.001 days.  Orbit trails also use the "relative_to" keyword, so if
this is omitted, the trail will not go around Mars, but show the
orbiter's path around the Sun, which is partly due to Mars moving in
its orbit.  If that description isn't clear, try it and see what I
mean.
