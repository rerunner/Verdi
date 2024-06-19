#pragma once

#include "hiberlite.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "boost/lexical_cast.hpp"
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>
#include "domain/base/ValueObjectBase.hpp"

class Uuid : public ValueObjectBase
{
private:
  std::string uuid_;
  static boost::uuids::uuid generateUuid()
  {
    static boost::uuids::random_generator idGenerator;
    return idGenerator();
  }
  //Boilerplate start
  friend class hiberlite::access;
  template<class Archive>
  void hibernate(Archive & ar)
  {
    ar & HIBERLITE_NVP(uuid_);
  }
  //Boilerplate end
public:
  Uuid ()
  {
    uuid_ = boost::lexical_cast<std::string>(generateUuid());
  }
  Uuid (std::string withUuid_)
  {
    uuid_ = withUuid_;
  }

  virtual bool operator==(const ValueObjectBase& other) const override
  {
    if (const Uuid* otherUuid = dynamic_cast<const Uuid*>(&other))
    {
        return (uuid_ == otherUuid->Get());
    }
    return false;
  }
  const std::string Get() const
  {
    return uuid_;
  }
  //JSON boilerplate
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Uuid, uuid_)
};

// Boilerplate
HIBERLITE_EXPORT_CLASS(Uuid)
