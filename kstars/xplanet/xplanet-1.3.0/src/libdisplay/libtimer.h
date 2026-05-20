#ifndef LIBTIMER_H
#define LIBTIMER_H

#include "Timer.h"

extern Timer *getTimer(const int wait, const unsigned long hibernate, 
		       const unsigned long idlewait);

#endif
