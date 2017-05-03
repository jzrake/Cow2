#include <iostream>
#include <cassert>
#include "Array.hpp"
#include "MPI.hpp"
#include "HDF5.hpp"
#include "DistributedUniformMesh.hpp"
#include "Timer.hpp"

using namespace Cow;



void testHeap()
{
    auto A = HeapAllocation (36);
    assert (A.size() == 36);

    auto B = A;
    assert (A.size() == 36);
    assert (B.size() == 36);

    auto C = std::move (B);
    assert (A.size() == 36);
    assert (B.size() == 0);
    assert (C.size() == 36);
}


void testArray()
{
    auto A = Array (128);
    assert (A.size() == 128);

    auto B = std::move (A);

    assert (A.size() == 0);
    assert (B.size() == 128);
}


void testHdf5()
{
    {
        auto testFile = H5::File ("test.h5", "w");
        auto group1 = testFile.createGroup ("group1");
        auto group2 = testFile.createGroup ("group2");
        auto A = Array (2, 2, 1, 2, 1);

        int n = 0;

        for (auto &x : A)
        {
            x = ++n;
        }

        group1.write ("nameOfCat", "orange cat");
        group1.write ("someData", A);
        group1.write ("doubleParameter", 1.234);

        auto dset1 = testFile.createDataSet ("dset1", A.getShapeVector());
        auto dset2 = testFile.createDataSet ("dset2", A.getShapeVector());

        Region region1;
        Region region2;

        dset1[region1] = A[region2];
        dset2[region1] = A;

        testFile.write ("dset3", A);
        testFile.write ("dset4", A[region1]);
    }

    {
        auto testFile = H5::File ("test.h5", "a");
        testFile.write ("wrttenAfter", "this data was written in H5F_ACC_RDWR mode");
    }
}


void testDistributedUniformMesh()
{
    auto world = MpiCommunicator::world();
    auto cart = world.createCartesian (1);
    auto mesh = DistributedUniformMesh ({128}, cart);
    auto shape = mesh.getLocalArrayShape();

    cart.inSequence ([&] (int rank)
    {
        std::cout << "MPI rank: " << rank << ", proc shape = " << shape[0] << std::endl;
    });
}


template <class T> void timeLoopEvaluation (T ref, std::string message)
{
    auto timer = Timer();

    int n = 0;
    for (auto& x : ref)
    {
        x = n++;
    }

    std::cout << message << ": " << timer.age() << " s" << std::endl;
};


void testIter()
{
    auto region = Region();
    auto A = Array (24, 24, 24, 24, 24);
    auto B = std::vector<double> (A.size());

    timeLoopEvaluation (A, "Cow::Array -> raw linear iteration");
    timeLoopEvaluation (A[region], "Cow::Array -> region iteration");
    timeLoopEvaluation (B, "std::vector -> linear iteration");
}


int main (int argc, const char* argv[])
{
    MpiSession mpi;
    
    testHeap();
    testArray();
    testHdf5();
    //testDistributedUniformMesh();
    testIter();

    return 0;
}
