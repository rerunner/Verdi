#pragma once

#include <iostream>
#include "boost/lexical_cast.hpp"
#include "Uuid.hpp"

namespace Verdi
{

class EntityBase
{
protected:
  Uuid id_;
  Uuid parentId_;
  
public:
  EntityBase(){}
  
  Uuid GetId() const { return id_; }

  Uuid GetParentId() const { return parentId_; }

  bool operator==(const EntityBase& other) const
  {
    return (id_.Get() == other.GetId().Get());
  }

  bool operator!=(const EntityBase& other) const
  {
    return !(*this == other);
  }
};

}
