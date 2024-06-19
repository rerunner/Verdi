// entitypp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "infrastructure/base/UnitOfWork.hpp"

#include "Position.hpp"
#include "Measurement.hpp"
#include "WaferHeightMap.hpp"
#include "IWaferHeightMapRepository.hpp"

unsigned int GSL::ACTIVE_MESSAGES = GSL::FATAL | GSL::ERROR | GSL::WARNING | GSL::INFO;

RepositoryType RepositoryTypeBase::REPOSITORY_TYPE = RepositoryType::ORM;
// RepositoryType::HMM
// RepositoryType::ORM
// RepositoryType::FFS
// RepositoryType::ODM


int main()
{
    unitofwork::UnitOfWorkFactory UoWFactory;

    GSL::Dprintf(GSL::INFO, "Versatile Data Infrastructure simple test");

    std::unique_ptr<unitofwork::UnitOfWork> context_ = UoWFactory.GetNewUnitOfWork();

    // Create wafer heightmap
    Position position_a(10.5,20.5);
    Measurement measurement_a(position_a, 5.5);
    std::shared_ptr<WaferHeightMap> waferHeightMap = std::make_shared<WaferHeightMap>(measurement_a);
    GSL::Dprintf(GSL::INFO, "WaferHeightMap created with ID = ", waferHeightMap->GetId().Get());
        
    context_->RegisterNew<WaferHeightMap>(waferHeightMap);
    context_->Commit();
    GSL::Dprintf(GSL::INFO, "WaferHeightMap with ID = ", waferHeightMap->GetId().Get(), " persisted.");

    // Get test
    std::unique_ptr<IRepositoryFactory<WaferHeightMap>> repositoryFactory = std::make_unique<RepositoryFactory<WaferHeightMap>>();
    auto repository = repositoryFactory->GetRepository(RepositoryTypeBase::REPOSITORY_TYPE, UoWFactory.GetDataBasePtr());
    auto whmClone = repository->Get(waferHeightMap->GetId()); //Fetch all heightmaps in the database
    GSL::Dprintf(GSL::INFO, "Get WaferHeightMap with ID = ", waferHeightMap->GetId().Get()," returned ", whmClone.GetId().Get());
    //

    // GetAll test
    auto whmList = repository->GetAll(); //Fetch all heightmaps in the database
    GSL::Dprintf(GSL::INFO, "Searched for all waferheightmaps. found number = ", whmList.size());
    //

    // GetAllChildren test
    auto whmList2 = repository->GetAllChildren(whmClone.GetParentId()); //Fetch all heightmaps in the database
    GSL::Dprintf(GSL::INFO, "Searched for specific waferheightmaps. found number = ", whmList2.size());
    //

    context_->RegisterDeleted<WaferHeightMap>(waferHeightMap);
    context_->Commit();
    GSL::Dprintf(GSL::INFO, "WaferHeightMap with ID = ", waferHeightMap->GetId().Get(), " deleted.");

    return 0;
}
