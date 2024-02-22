#ifndef MYENTITY_H
#define MYENTITY_H

#include "hiberlite.h"
#include "AggregateRootBase.hpp"
#include "Point.hpp" //Value Object


class MyAggregateRoot : public AggregateRootBase
{
private:
  double x_;
  double y_;
  double z_;
  Point p_;
  
  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(id_); // From Base class
    ar & HIBERLITE_NVP(x_);
    ar & HIBERLITE_NVP(y_);
    ar & HIBERLITE_NVP(z_);
    ar & HIBERLITE_NVP(p_);
  }
  //Boilerplate end
public:
  MyAggregateRoot() : AggregateRootBase(){};
  MyAggregateRoot(double x, double y, double z, Point p) : AggregateRootBase()
  {
    x_ = x; y_ = y; z_ = z; p_ = p;
  }

  double GetX(void) {return x_;}
  double GetY(void) {return y_;}
  double GetZ(void) {return z_;}
  Point GetPoint(void){return p_;}

  void SetX(double x) { x_ = x; }
  void SetY(double y) { y_ = y; }
  void SetZ(double z) { z_ = z; }

};

// Boilerplate
HIBERLITE_EXPORT_CLASS(MyAggregateRoot)

#endif // MYENTITY_H
