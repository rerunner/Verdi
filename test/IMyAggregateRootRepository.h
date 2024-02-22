#pragma once

#include "IRepositoryFactory.h"
#include "IRepositoryBase.h"

#include "MyAggregateRoot.hpp"


class IMyAggregateRootRepository : public IRepositoryBase<MyAggregateRoot> {};
