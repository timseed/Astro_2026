#ifndef PROJECTIONBASE_H
#define PROJECTIONBASE_H

class Options;

class ProjectionBase
{
 public:
    ProjectionBase(const int flipped, const int w, const int h);
    ProjectionBase(const int flipped, const int w, const int h,
		   const Options* o);

    virtual ~ProjectionBase();

    virtual bool pixelToSpherical(const double x, const double y, 
				  double &lon, double &lat) = 0;

    virtual bool sphericalToPixel(const double lon, const double lat,
				  double &x, double &y) const = 0;

    bool IsWrapAround() const { return(isWrapAround_); };

    double Radius() const { return(radius_); };

    void RotateXYZ(double &lat, double &lon) const;
    void RotateZYX(double &lat, double &lon) const;

    virtual double getDarkening() const { return(darkening_); };
    virtual void setRange(const double range);

    void SetXYZRotationMatrix(const double angle_x, 
			      const double angle_y, 
			      const double angle_z);
    void SetZYXRotationMatrix(const double angle_x, 
			      const double angle_y, 
			      const double angle_z);
 protected:
    const int flipped_;
    const int width_, height_;
    double radius_;
    double centerX_, centerY_;

    bool isWrapAround_;

    double centerLat_, centerLon_;
    bool rotate_;
    double rotXYZ_[3][3];
    double rotZYX_[3][3];

    // for the photometric function
    int tableSize_;
    double darkening_;
    double *cosAngle_;
    double *photoFunction_;

    void buildPhotoTable();
    void destroyPhotoTable();
    double getPhotoFunction(const double x) const;

 private:
    void init(const int flipped, const int w, const int h, const Options* options);
	
};

#endif
