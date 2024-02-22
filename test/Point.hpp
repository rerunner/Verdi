#ifndef POINT_H
#define POINT_H

#include "ValueObjectBase.hpp"


class Point : public ValueObjectBase
{
private:
  double x_;
  double y_;
  double z_;

  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(x_);
    ar & HIBERLITE_NVP(y_);
    ar & HIBERLITE_NVP(z_);
  }
  //Boilerplate end
  
public:
  Point(double x = 0.0, double y = 0.0, double z = 0.0) : x_(x), y_(y), z_(z) {}

  bool operator==(const ValueObjectBase& other) const override
  {
    if (const Point* otherPoint = dynamic_cast<const Point*>(&other))
      {
	return (x_ == otherPoint->x_) && (y_ == otherPoint->y_);
      }
    return false;
  }

  double GetX() const {return x_;}
  double GetY() const {return y_;}
  double GetZ() const {return x_;}
};

// Boilerplate
HIBERLITE_EXPORT_CLASS(Point)

#endif // POINT_H
