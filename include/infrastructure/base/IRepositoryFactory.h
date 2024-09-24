#pragma once

#include <any>
#include <raven_cpp_client.h> // Problems with clang, gcc ok
#include "IRepositoryBase.h"

namespace Verdi
{

enum RepositoryType
{
  HMM,
  ORM,
  ODM,
  FFS
};

class RepositoryTypeBase
{
  public:
  static RepositoryType REPOSITORY_TYPE;//=RepositoryType::HMM;
};

template <typename T>
class IRepositoryFactory : public RepositoryTypeBase
{
public:
  virtual ~IRepositoryFactory(){}
  virtual IRepositoryBase<T>* GetRepository(RepositoryType repository, std::any db) = 0;
};

}
