// entitypp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <raft>
#include <raftio>

#include "IMyAggregateRootRepository.h"
#include "RepositoryFactory.h"
#include "Point.hpp"

// Kernel definitions
template < class T > class SourceUnit : public raft::kernel
{
public:
   SourceUnit() : kernel()
    {
       output.addPort< T >( "outputPoint" );
    }


  virtual ~SourceUnit() = default;

  virtual raft::kstatus run()
  {
    const T out{20.0, 30.0, 40.0};
    output[ "outputPoint" ].push( out );
    return( raft::stop );
   }

private:
};

template < typename  T > class AdderUnit : public raft::kernel
{
public:
    AdderUnit() : kernel()
    {
      input.addPort< T >("inputPoint");
      output.addPort< T >( "outputPoint" );
    }

    virtual ~AdderUnit() = default;

    virtual raft::kstatus run()
    {
      T pointContainer;
      input[ "inputPoint" ].pop( pointContainer ); 
      const T out{pointContainer.GetX()+1, pointContainer.GetY()+1, pointContainer.GetZ()+1};
      output[ "outputPoint" ].push( out );
      return( raft::proceed );
    }

private:
};


int main()
{
    std::cout << "Data Infrastructure framework for C++\n\n";

    //Create Factory for MyAggregateRoot
    IRepositoryFactory<MyAggregateRoot> *repositoryFactory = new RepositoryFactory<MyAggregateRoot>;
    
    //Use factory to create specialized repository to store on Heap Memory or ORM
    auto *myRepo = repositoryFactory->GetRepository(RepositoryType::HeapRepository);
    //auto *myRepo = repositoryFactory->GetRepository(RepositoryType::ORM);

    Point myEndPoint;
    Point myPoint{20.0,30.0,40.0};
    
    // Raft start
    SourceUnit<Point> sourceUnit;
    AdderUnit<Point> adderUnit;

    using SinkLambda = raft::lambdak<Point>;
    SinkLambda sinkLambda(1,/* input port */
		          0, /* output port */
			  [&](Port &input,
			      Port &output)
			  {
			    UNUSED( output );
			    input[ "0" ].pop( myEndPoint );
			    return( raft::stop );
			  });

    raft::map m;
    m += sourceUnit >> adderUnit >> sinkLambda;
    m.exe();
    //Raft End

    std::cout << "X = " << myEndPoint.GetX();
    std::cout << ", Y = " << myEndPoint.GetY();
    std::cout << ", Z = " << myEndPoint.GetZ() << std::endl;

    MyAggregateRoot entity1{ 1.0,100.0,200.0, myEndPoint };
    std::cout << "AggregateRoot " << entity1.GetId() << " created.\n";
    
    myRepo->Store(entity1);
    std::cout << "AggregateRoot " << entity1.GetId() << " persisted.\n";

    MyAggregateRoot entity2 = myRepo->Get(entity1.GetId());
    std::cout << "Copy of AggregateRoot " << entity1.GetId() << " retrieved.\n";

    std::cout << "Copy of AggregateRoot, X = " << entity2.GetX() << std::endl;
    std::cout << "Copy of AggregateRoot, ID = " << entity2.GetId() << std::endl;
    const Point myRetrievedPoint = entity2.GetPoint();
    std::cout << "Copy of AggregateRoot myPoint.x = " << myRetrievedPoint.GetX() << std::endl;
	
    myRepo->Delete(entity1);

    std::cout << "trying again";
    myRepo->Delete(entity1);
    std::cout << " should do nothing" << std::endl;

    myRepo->Store(entity1);
    myRepo->Delete(entity1);

    return 0;
}
