#ifndef AGGREGATE_ROOT_BASE_H
#define AGGREGATE_ROOT_BASE_H

namespace Verdi
{

class AggregateRootBase
{
protected:
  Uuid id_;
  
public:
  AggregateRootBase(){}
  
  Uuid GetId() const { return id_; }

  Uuid GetParentId() const { return id_; }

  bool operator==(const AggregateRootBase& other) const
  {
    return (id_.Get() == other.GetId().Get());
  }

  bool operator!=(const AggregateRootBase& other) const
  {
    return !(*this == other);
  }
};

}

#endif // AGGREGATE_ROOT_BASE_H
