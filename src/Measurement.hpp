#pragma once

#include "domain/base/ValueObjectBase.hpp"
#include "Position.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Measurement : public ValueObjectBase
{
private:
  double z_;
  Position position_;

  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(z_);
    ar & HIBERLITE_NVP(position_);
  }
  //Boilerplate end
  
public:
  Measurement() : ValueObjectBase() {};
  Measurement(Position position, double zvalue = 0.0) : ValueObjectBase()
  {
    position_ = position;
    z_ = zvalue;
  }

  bool operator==(const ValueObjectBase& other) const override
  {
    //TODO
    return false;
  }

  Position GetPosition() const {return position_;}
  double GetZ() const {return z_;}

  //JSON boilerplate
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Measurement, z_, position_)
};

// Hiberlite Boilerplate
HIBERLITE_EXPORT_CLASS(Measurement)
