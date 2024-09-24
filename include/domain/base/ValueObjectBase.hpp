#pragma once

#include <iostream>

namespace Verdi
{

class ValueObjectBase
{
public:
  virtual bool operator==(const ValueObjectBase& other) const = 0;

  virtual bool operator!=(const ValueObjectBase& other) const
  {
    return !(*this == other);
  }

  virtual ~ValueObjectBase() = default;
};

}


