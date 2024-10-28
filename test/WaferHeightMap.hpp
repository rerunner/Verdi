#pragma once
#include <list>
#include "hiberlite.h"
#include <nlohmann/json.hpp>
#include "domain/base/AggregateRootBase.hpp"
#include "Measurement.hpp" //Value Object

using json = nlohmann::json;

class WaferHeightMap : public Verdi::AggregateRootBase
{
private:
  std::list<Measurement> measurements_;

  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(id_); // From Entity Base class
    ar & HIBERLITE_NVP(parentId_); // From Entity Base class
    ar & HIBERLITE_NVP(measurements_);
  }
  //Boilerplate end
public:
  WaferHeightMap() : AggregateRootBase(){};
  WaferHeightMap(Measurement m) : AggregateRootBase()
  {
    measurements_.push_back(m); //First entry
  }

  std::list<Measurement> GetHeightMap(void){return measurements_;}
  void AddMeasurement(Measurement m) { measurements_.push_back(m); }

  //JSON boilerplate
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WaferHeightMap, id_, parentId_, measurements_)
};

// Boilerplate
HIBERLITE_EXPORT_CLASS(WaferHeightMap)
