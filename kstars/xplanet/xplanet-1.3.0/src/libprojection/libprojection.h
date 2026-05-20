#ifndef LIBPROJECTION_H
#define LIBPROJECTION_H

#include "ProjectionBase.h"

extern int getRandomProjection();
extern int getProjectionType(char *proj_string);
extern ProjectionBase *getProjection(const int projection,
                                     const int flipped, 
                                     const int width, const int height);

#endif
