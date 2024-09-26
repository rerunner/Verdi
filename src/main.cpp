#include <iostream>
#include "infrastructure/base/UnitOfWork.hpp"
#include "Position.hpp"
#include "Measurement.hpp"
#include "WaferHeightMap.hpp"
#include "IWaferHeightMapRepository.hpp"

using namespace Verdi;

unsigned int GSL::ACTIVE_MESSAGES = GSL::FATAL | GSL::ERROR | GSL::WARNING | GSL::INFO | GSL::DEBUG;

RepositoryType RepositoryTypeBase::REPOSITORY_TYPE = RepositoryType::ORM;
// RepositoryType::HMM
// RepositoryType::ORM
// RepositoryType::FFS
// RepositoryType::ODM


int main()
{
    unitofwork::UnitOfWorkFactory UoWFactory;
    Uuid testId;

    GSL::Dprintf(GSL::INFO, "Versatile Data Infrastructure simple test suite");

    std::unique_ptr<unitofwork::UnitOfWork> context0_ = UoWFactory.GetNewUnitOfWork();

    // Create wafer heightmap and rollback
    {
      GSL::Dprintf(GSL::INFO, "--- TEST 1 START ---");
      Position position_a(10.5,20.5);
      Measurement measurement_a(position_a, 5.5);
      std::shared_ptr<WaferHeightMap> waferHeightMap = std::make_shared<WaferHeightMap>(measurement_a);
      GSL::Dprintf(GSL::INFO, "WaferHeightMap created with ID = ", waferHeightMap->GetId().Get());
        
      context0_->RegisterNew<WaferHeightMap>(waferHeightMap);
      context0_->Rollback();
      GSL::Dprintf(GSL::INFO, "WaferHeightMap with ID = ", waferHeightMap->GetId().Get(), " rolled back.");
      GSL::Dprintf(GSL::INFO, "--- TEST 1 END ---");
    }

    std::unique_ptr<unitofwork::UnitOfWork> context_ = UoWFactory.GetNewUnitOfWork();

    // Create wafer heightmap and commit
    {
      GSL::Dprintf(GSL::INFO, "--- TEST 2 START ---");
      Position position_a(10.5,20.5);
      Position position_b(8.8,9.9);
      Measurement measurement_a(position_a, 3.3);
      Measurement measurement_b(position_b, 7.7);
      std::shared_ptr<WaferHeightMap> waferHeightMap = std::make_shared<WaferHeightMap>(measurement_a);
      waferHeightMap->AddMeasurement(measurement_b);
      GSL::Dprintf(GSL::INFO, "WaferHeightMap created with ID = ", waferHeightMap->GetId().Get());
      
      context_->RegisterNew<WaferHeightMap>(waferHeightMap);
      context_->Commit();
      GSL::Dprintf(GSL::INFO, "WaferHeightMap with ID = ", waferHeightMap->GetId().Get(), " persisted.");
      testId = waferHeightMap->GetId();
      GSL::Dprintf(GSL::INFO, "--- TEST 2 END ---");
    }

    // Get test
    GSL::Dprintf(GSL::INFO, "--- TEST 3 START ---");
    std::unique_ptr<IRepositoryFactory<WaferHeightMap>> repositoryFactory = std::make_unique<RepositoryFactory<WaferHeightMap>>();
    auto repository = repositoryFactory->GetRepository(RepositoryTypeBase::REPOSITORY_TYPE, UoWFactory.GetDataBasePtr());
    auto whmClone = repository->Get(testId); //Fetch all heightmaps in the database
    std::shared_ptr<WaferHeightMap> whmClonePtr = std::make_shared<WaferHeightMap>(whmClone);
    GSL::Dprintf(GSL::INFO, "Get WaferHeightMap with ID = ", testId.Get()," returned ", whmClone.GetId().Get());
    GSL::Dprintf(GSL::INFO, "--- TEST 3 END ---");
    //

    // GetAll test
    GSL::Dprintf(GSL::INFO, "--- TEST 4 START ---");
    auto whmList = repository->GetAll(); //Fetch all heightmaps in the database
    GSL::Dprintf(GSL::INFO, "Searched for all waferheightmaps. found number = ", whmList.size());
    for (WaferHeightMap whmIterator : whmList)
    {
        GSL::Dprintf(GSL::INFO, "WaferHeightMap found with ID = ", whmIterator.GetId().Get());
        auto measurementsList = whmIterator.GetHeightMap();
        for (Measurement measurementIterator : measurementsList)
        {
            GSL::Dprintf(GSL::INFO, "Measurement found with x = ", measurementIterator.GetPosition().GetX()
                         ," y = ", measurementIterator.GetPosition().GetY()
                         ," z = ", measurementIterator.GetZ());
        }
    }
    GSL::Dprintf(GSL::INFO, "--- TEST 4 END ---");
    //

    // GetAllChildren test
    GSL::Dprintf(GSL::INFO, "--- TEST 5 START ---");
    auto whmList2 = repository->GetAllChildren(whmClone.GetParentId()); //Fetch all heightmaps in the database
    GSL::Dprintf(GSL::INFO, "Searched for specific waferheightmaps. found number = ", whmList2.size());
    GSL::Dprintf(GSL::INFO, "--- TEST 5 END ---");
    //

    // Delete test
    GSL::Dprintf(GSL::INFO, "--- TEST 6 START ---");
    context_->RegisterDeleted<WaferHeightMap>(whmClonePtr);
    context_->Commit();
    GSL::Dprintf(GSL::INFO, "WaferHeightMap with ID = ", whmClonePtr->GetId().Get(), " deleted.");
    // Now try to see what is left
    auto whmList3 = repository->GetAll(); //Fetch all heightmaps in the database
    GSL::Dprintf(GSL::INFO, "Searched for all waferheightmaps. found number = ", whmList3.size());
    for (WaferHeightMap whmIterator : whmList3)
    {
        GSL::Dprintf(GSL::INFO, "WaferHeightMap found with ID = ", whmIterator.GetId().Get());
        auto measurementsList = whmIterator.GetHeightMap();
        for (Measurement measurementIterator : measurementsList)
        {
            GSL::Dprintf(GSL::INFO, "Measurement found with x = ", measurementIterator.GetPosition().GetX()
                         ," y = ", measurementIterator.GetPosition().GetY()
                         ," z = ", measurementIterator.GetZ());
        }
    }
    GSL::Dprintf(GSL::INFO, "--- TEST 6 END ---");

    return 0;
}
