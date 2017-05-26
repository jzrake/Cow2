#include <iostream>
#include <cassert>
#include "Array.hpp"
#include "MPI.hpp"
#include "HDF5.hpp"
#include "DistributedUniformMesh.hpp"
#include "Timer.hpp"
#include "DebugHelper.hpp"

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

    auto S = Array (12, 13, 14, 1, 1);

    for (int n = 0; n < S.size(); ++n)
    {
        S[n] = n;
    }

    auto T = S.transpose();
    auto R = S.transpose (0, 1);
    auto Q = R.transpose (0, 1); // should have Q == S

    assert(T.size(0) == 1);
    assert(T.size(1) == 1);
    assert(T.size(2) == 14);
    assert(T.size(3) == 13);
    assert(T.size(4) == 12);
    assert(R.size(0) == 13);
    assert(R.size(1) == 12);
    assert(Q.size(0) == S.size(0));
    assert(Q.size(1) == S.size(1));

    for (int n = 0; n < Q.size(); ++n)
    {
        assert(Q[n] == S[n]);
    }
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
        group1.createGroup ("nestedGroup").write ("property", 12345);

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
        auto message = std::string ("this data was written in H5F_ACC_RDWR mode");
        testFile.write ("writtenAfter", message);
        assert (testFile.getDataSet ("writtenAfter").readAll().toString() == message);

        auto A = testFile.readArray ("dset3");
        assert (A.size() == 8);

        auto nestedGroup = testFile.getGroup ("group1/nestedGroup");
        assert (nestedGroup.readInt ("property") == 12345);
    }
}


void testDistributedUniformMesh()
{
    auto world = MpiCommunicator::world();
    auto cart = world.createCartesian (1);
    auto guard = Cow::GuardZoneExtension();

    guard.lower[0] = 2;
    guard.upper[0] = 3;

    auto mesh = DistributedUniformMesh ({128}, cart, guard);
    auto shape = mesh.getLocalArrayShape();

    cart.inSequence ([&] (int rank)
    {
        std::cout
        << "MPI rank: "
        << rank
        << " shape: "
        << shape[0] 
        << " right: "
        << cart.shift (0, 1)
        << " left: "
        << cart.shift (0, -1)
        << std::endl;
    });

    auto mpiDouble = MpiDataType::nativeDouble();
    auto mpiInt = MpiDataType::nativeInt();
    assert (mpiDouble.size() == sizeof (double));
    assert (mpiInt.size() == sizeof (int));

    auto A = Array (12);

    Region send;
    Region recv;
    send.lower[0] = -4;
    send.upper[0] = -2;
    recv.lower[0] =  0;
    recv.upper[0] =  2;
    
    cart.shiftExchange (A, 0, 'R', send, recv);
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
}


void testIter()
{
    auto region = Region();
    auto A = Array (24, 24, 24, 24, 24);
    auto B = std::vector<double> (A.size());

    timeLoopEvaluation (A, "Cow::Array -> raw linear iteration");
    timeLoopEvaluation (A[region], "Cow::Array -> region iteration");
    timeLoopEvaluation (B, "std::vector -> linear iteration");
}


void testSlicing()
{
    auto source = Array ( 1, 12, 12);
    auto target = Array (12, 12, 12);
    auto R = Region();

    R.lower[0] = 6;
    R.upper[0] = 7;

    target.insert (source, R);
    target[R] = source;
}


int main (int argc, const char* argv[])
{
    MpiSession mpi;
    std::set_terminate (Cow::terminateWithBacktrace);

    testHeap();
    testArray();
    testHdf5();
    testDistributedUniformMesh();
    testIter();
    testSlicing();

    return 0;
}
