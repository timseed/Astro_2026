#ifndef LIBMOONS_H
#define LIBMOONS_H

extern void 
moon(const double jd, double &Mx, double &My, double &Mz);

extern void
marsat(const double jd, const body b, 
       double &X, double &Y, double &Z);

extern void
jupsat(const double jd, const body b, 
       double &X, double &Y, double &Z);

extern void
jupsat2(const double jd, const body b, 
	double &X, double &Y, double &Z);

extern void
satsat(const double jd, const body b, 
       double &X, double &Y, double &Z);

extern void
urasat(const double jd, const body b,
       double &X, double &Y, double &Z);

extern void
nepsat(const double jd, const body b,
       double &X, double &Y, double &Z);

extern void
plusat(const double jd, double &X, double &Y, double &Z);

#endif
