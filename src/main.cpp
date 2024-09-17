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
    Uuid testId;

    GSL::Dprintf(GSL::INFO, "Versatile Data Infrastructure simple test");

    std::unique_ptr<unitofwork::UnitOfWork> context0_ = UoWFactory.GetNewUnitOfWork();

    // Create wafer heightmap and rollback
    {
      Position position_a(10.5,20.5);
      Measurement measurement_a(position_a, 5.5);
      std::shared_ptr<WaferHeightMap> waferHeightMap = std::make_shared<WaferHeightMap>(measurement_a);
      GSL::Dprintf(GSL::INFO, "WaferHeightMap created with ID = ", waferHeightMap->GetId().Get());
        
      context0_->RegisterNew<WaferHeightMap>(waferHeightMap);
      context0_->Rollback();
      GSL::Dprintf(GSL::INFO, "WaferHeightMap with ID = ", waferHeightMap->GetId().Get(), " rolled back.");
    }

    std::unique_ptr<unitofwork::UnitOfWork> context_ = UoWFactory.GetNewUnitOfWork();

    // Create wafer heightmap and commit
    {
      Position position_a(10.5,20.5);
      Measurement measurement_a(position_a, 5.5);
      std::shared_ptr<WaferHeightMap> waferHeightMap = std::make_shared<WaferHeightMap>(measurement_a);
      GSL::Dprintf(GSL::INFO, "WaferHeightMap created with ID = ", waferHeightMap->GetId().Get());
      
      context_->RegisterNew<WaferHeightMap>(waferHeightMap);
      context_->Commit();
      GSL::Dprintf(GSL::INFO, "WaferHeightMap with ID = ", waferHeightMap->GetId().Get(), " persisted.");
      testId = waferHeightMap->GetId();
    }

    // Get test
    std::unique_ptr<IRepositoryFactory<WaferHeightMap>> repositoryFactory = std::make_unique<RepositoryFactory<WaferHeightMap>>();
    auto repository = repositoryFactory->GetRepository(RepositoryTypeBase::REPOSITORY_TYPE, UoWFactory.GetDataBasePtr());
    auto whmClone = repository->Get(testId); //Fetch all heightmaps in the database
    std::shared_ptr<WaferHeightMap> whmClonePtr = std::make_shared<WaferHeightMap>(whmClone);
    GSL::Dprintf(GSL::INFO, "Get WaferHeightMap with ID = ", testId.Get()," returned ", whmClone.GetId().Get());
    //

    // GetAll test
    auto whmList = repository->GetAll(); //Fetch all heightmaps in the database
    GSL::Dprintf(GSL::INFO, "Searched for all waferheightmaps. found number = ", whmList.size());
    //

    // GetAllChildren test
    auto whmList2 = repository->GetAllChildren(whmClone.GetParentId()); //Fetch all heightmaps in the database
    GSL::Dprintf(GSL::INFO, "Searched for specific waferheightmaps. found number = ", whmList2.size());
    //

    // Delete test
    context_->RegisterDeleted<WaferHeightMap>(whmClonePtr);
    context_->Commit();
    GSL::Dprintf(GSL::INFO, "WaferHeightMap with ID = ", whmClonePtr->GetId().Get(), " deleted.");

    return 0;
}
