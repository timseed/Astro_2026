#ifndef BODY_H
#define BODY_H

enum body
{
    SUN, 
    MERCURY, 
    VENUS, 
    EARTH, MOON, 
    MARS, PHOBOS, DEIMOS, 
    JUPITER, IO, EUROPA, GANYMEDE, CALLISTO,
    SATURN, MIMAS, ENCELADUS, TETHYS, DIONE, RHEA, TITAN, HYPERION, IAPETUS, PHOEBE, 
    URANUS, MIRANDA, ARIEL, UMBRIEL, TITANIA, OBERON, 
    NEPTUNE, TRITON, NEREID, 
    PLUTO, CHARON, 
    RANDOM_BODY,    // RANDOM_BODY needs to be after the last "real" body
    ABOVE_ORBIT, ALONG_PATH, BELOW_ORBIT, DEFAULT, MAJOR_PLANET, NAIF, NORAD, SAME_SYSTEM, UNKNOWN_BODY
};

const char* const body_string[RANDOM_BODY] =
{"sun", 
 "mercury", 
 "venus", 
 "earth", "moon", 
 "mars", "phobos", "deimos", 
 "jupiter", "io", "europa", "ganymede", "callisto", 
 "saturn", "mimas", "enceladus", "tethys", "dione", "rhea", "titan", "hyperion", "iapetus", "phoebe", 
 "uranus", "miranda", "ariel", "umbriel", "titania", "oberon", 
 "neptune", "triton", "nereid", 
 "pluto", "charon"};

const int naif_id[RANDOM_BODY] =
{ 10,
  199,
  299,
  399, 301,
  499, 401, 402,
  599, 501, 502, 503, 504,
  699, 601, 602, 603, 604, 605, 606, 607, 608, 609,
  799, 705, 701, 702, 703, 704,
  899, 801, 802,
  999, 901 };

#endif
