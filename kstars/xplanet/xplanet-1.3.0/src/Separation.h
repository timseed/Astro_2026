#ifndef SEPARATION_H
#define SEPARATION_H

class View;

class Separation
{
 public:
    Separation(const double oX, const double oY, const double oZ,
	       const double tX, const double tY, const double tZ, 
	       const double sX, const double sY, const double sZ);

    ~Separation();

    void getOrigin(double &oX, double &oY, double &oZ);
    void setSeparation(double sep);

 private:
    double tX_, tY_, tZ_;
    double sX_, sY_, sZ_;
    double oX_, oY_, oZ_;

    double oR_;      // target 1 - observer distance
    double sR_;      // target 1 - target 2 distance

    View *view_;

    double calcSeparation(const double angle);

};

#endif
