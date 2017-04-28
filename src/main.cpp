#include <iostream>
#include <cassert>
#include "Array.hpp"
#include "MPI.hpp"
#include "HDF5.hpp"

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
    H5::File testFile ("test.h5");
    H5::Group group1 = testFile.createGroup ("group1");
    H5::DataSet dset1 = group1.createDataSet ("dataset1", {3, 3, 4});
    H5::DataSet dset2 = group1.createDataSet ("dataset2", {3, 3, 4});

    std::string value = "value for the property";
    H5::DataSet stringDataSet = testFile.createDataSet ("property", H5::DataType::nativeString (value.size()));
    stringDataSet.write (value);
}


void testMpi()
{
    MpiCommunicator world = MpiCommunicator::world();
    MpiCartComm cart = world.createCartesian (3);    
}


int main (int argc, const char* argv[])
{
    MpiSession mpi;
    
    testHeap();
    testArray();
    testHdf5();
    testMpi();

    return 0;
}
