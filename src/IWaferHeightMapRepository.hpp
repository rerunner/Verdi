#pragma once

#include "infrastructure/base/IRepositoryFactory.h"
#include "infrastructure/base/IRepositoryBase.h"

#include "WaferHeightMap.hpp"


class IWaferHeightMapRepository : public IRepositoryBase<WaferHeightMap> {};
