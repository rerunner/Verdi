// entitypp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <raft>
#include <raftio>

#include "infrastructure/base/UnitOfWork.hpp"

#include "Position.hpp"
#include "Measurement.hpp"
#include "WaferHeightMap.hpp"
#include "IWaferHeightMapRepository.hpp"

// Streaming Kernel definitions

//Source unit generates positions to measure on the wafer.
class PositionSetUnit : public raft::kernel
{
public:
  PositionSetUnit() : kernel()
  {
    output.addPort< Position >( "outputPosition" );
  }

  virtual ~PositionSetUnit() = default;

  virtual raft::kstatus run()
  {
    double xpos = 0.0, ypos = 0.0;
    for (int i = 0; i < 10; i++)
    {
      const Position out{xpos++, ypos++};
      output[ "outputPosition" ].push( out );
    }
    return( raft::stop );
  }

private:
};

//Measure unit measures the height for every input position
class MeasureUnit : public raft::kernel
{
public:
    MeasureUnit() : kernel()
    {
      input.addPort< Position >("inputPosition");
      output.addPort< Measurement >( "outputMeasurement" );
    }

    virtual ~MeasureUnit() = default;

    virtual raft::kstatus run()
    {
      Position positionContainer;
      input[ "inputPosition" ].pop( positionContainer ); // Receive position from input port
      const Measurement measurementContainer{positionContainer, 1.0}; // Do Measurement
      output[ "outputMeasurement" ].push( measurementContainer ); // Push measurement to output port
      return( raft::proceed );
    }

private:
};

unsigned int GSL::ACTIVE_MESSAGES = GSL::FATAL | GSL::ERROR | GSL::WARNING | GSL::INFO;

int main()
{
    unitofwork::UnitOfWorkFactory UoWFactory;

    std::cout << "Versatile Data Infrastructure test \n\n";

    std::unique_ptr<unitofwork::UnitOfWork> context_ = UoWFactory.GetNewUnitOfWork();

    // Create empty wafer heightmap
    Uuid waferId;
    std::shared_ptr<WaferHeightMap> waferHeightMap = std::make_shared<WaferHeightMap>(waferId);
    std::cout << "WaferHeightMap created with ID = " << waferHeightMap->GetId().Get() << "\n";
    
    // Raft streaming start
    Measurement generatedMeasurement;
    PositionSetUnit positionSetUnit;
    MeasureUnit measureUnit;

    using SinkLambda = raft::lambdak<Measurement>;
    SinkLambda sinkLambda(1,/* input port */
		          0, /* output port */
			  [&](Port &input,
			      Port &output)
			  {
			    UNUSED( output );
			    input[ "0" ].pop( generatedMeasurement ); // Take the measurement from the input
			    waferHeightMap->AddMeasurement(generatedMeasurement); // And add it to the heightmap
			    return( raft::proceed ); // Wait for the next measurement or stream end tag.
			  });

    raft::map m;
    m += positionSetUnit >> measureUnit >> sinkLambda;
    m.exe();
    //Raft streaming End
    
    context_->RegisterNew<WaferHeightMap>(waferHeightMap);
    context_->Commit();
    std::cout << "WaferHeightMap with ID = " << waferHeightMap->GetId().Get() << " persisted.\n";

    // Get test
    std::unique_ptr<IRepositoryFactory<WaferHeightMap>> repositoryFactory = std::make_unique<RepositoryFactory<WaferHeightMap>>();
    auto repository = repositoryFactory->GetRepository(REPOSITORY_TYPE, UoWFactory.GetDataBasePtr());
    auto whmClone = repository->Get(waferHeightMap->GetId()); //Fetch all heightmaps in the database
    std::cout << "Get WaferHeightMap with ID = " << waferHeightMap->GetId().Get() << " returned ";
    std::cout << whmClone.GetId().Get() << std::endl;
    //

    // GetAll test
    //std::unique_ptr<IRepositoryFactory<WaferHeightMap>> repositoryFactory = std::make_unique<RepositoryFactory<WaferHeightMap>>();
    //auto repository = repositoryFactory->GetRepository(REPOSITORY_TYPE, UoWFactory.GetDataBasePtr());
    auto whmList = repository->GetAll(); //Fetch all heightmaps in the database
    std::cout << "Searched for all waferheightmaps. found number = " << whmList.size() << std::endl;
    //

    // GetAllChildren test
    auto whmList2 = repository->GetAllChildren(whmClone.GetParentId()); //Fetch all heightmaps in the database
    std::cout << "Searched for specific waferheightmaps. found number = " << whmList2.size() << std::endl;
    //

    context_->RegisterDeleted<WaferHeightMap>(waferHeightMap);
    context_->Commit();
    std::cout << "WaferHeightMap with ID = " << waferHeightMap->GetId().Get() << " deleted.\n";

    return 0;
}
