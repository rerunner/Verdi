// entitypp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "IMyAggregateRootRepository.h"
#include "RepositoryFactory.h"
#include "Point.hpp"

int main()
{
    std::cout << "Data Infrastructure framework for C++\n\n";

    //Create Factory for MyAggregateRoot
    IRepositoryFactory<MyAggregateRoot> *repositoryFactory = new RepositoryFactory<MyAggregateRoot>;
    
    //Use factory to create specialized repository to store on Heap Memory or ORM
    //auto *myRepo = repositoryFactory->GetRepository(RepositoryType::HeapRepository);
    auto *myRepo = repositoryFactory->GetRepository(RepositoryType::ORM);

    const Point myPoint{22.0,33.0,44.0};
    MyAggregateRoot entity1{ 1.0,100.0,200.0, myPoint };
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
