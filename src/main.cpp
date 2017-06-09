#include <iostream>
#include <fstream>
#include <cassert>

#define COW_DEBUG_USE_CASSERT
#include "Array.hpp"
#include "MPI.hpp"
#include "HDF5.hpp"
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

    std::ofstream normal ("normal.bin");
    std::ofstream swapped ("swapped.bin");

    normal << S.getAllocation();
    swapped << S.getAllocation().swapBytes (sizeof (double));
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

        group1.writeString ("nameOfCat", "orange cat");
        group1.writeArray ("someData", A);
        group1.writeDouble ("doubleParameter", 1.234);
        group1.createGroup ("nestedGroup").writeInt ("property", 12345);

        auto dset1 = testFile.createDataSet ("dset1", A.getShapeVector());
        auto dset2 = testFile.createDataSet ("dset2", A.getShapeVector());

        Region region1;
        Region region2;

        dset1[region1] = A[region2];
        dset2[region1] = A;

        testFile.writeArray ("dset3", A);
        testFile.writeArray ("dset4", A[region1]);
    }

    {
        auto testFile = H5::File ("test.h5", "a");
        auto message = std::string ("this data was written in H5F_ACC_RDWR mode");
        testFile.writeString ("writtenAfter", message);
        assert (testFile.getDataSet ("writtenAfter").readAll().toString() == message);

        auto A = testFile.readArray ("dset3");
        assert (A.size() == 8);

        auto nestedGroup = testFile.getGroup ("group1/nestedGroup");
        assert (nestedGroup.readInt ("property") == 12345);
    }

    {
        Variant blnvar = true;
        Variant intvar = 234;
        Variant dblvar = 3.12;
        Variant strvar = "str";

        auto testFile = H5::File ("test.h5", "w");
        testFile.writeVariant ("bln", blnvar);
        testFile.writeVariant ("int", intvar);
        testFile.writeVariant ("dbl", dblvar);
        testFile.writeVariant ("str", strvar);

        assert (bool        (testFile.readVariant ("bln")) == bool (true));
        assert (            (testFile.readVariant ("bln").getType() == 'b'));
        assert (int         (testFile.readVariant ("int")) == int (intvar));
        assert (            (testFile.readVariant ("int").getType() == 'i'));
        assert (double      (testFile.readVariant ("dbl")) == double (dblvar));
        assert (            (testFile.readVariant ("dbl").getType() == 'd'));
        assert (std::string (testFile.readVariant ("str")) == std::string (strvar));
        assert (            (testFile.readVariant ("str").getType() == 's'));
    }

    {
        auto A = Array (8, 8, 8);
        auto dtype = H5::DataType::nativeDouble();
        auto plist = H5::PropertyList::DataSetCreate().setChunk ({4, 4, 4});

        auto testFile = H5::File ("test.h5", "w");
        testFile.createDataSet ("chunked_array", A.getShapeVector(), dtype, plist);
    }

    {
        auto testFile = H5::File ("test.h5", "w");
        auto int_dset = testFile.createDataSet ("int_dataset", std::vector<int> (1, 10), H5::DataType::nativeInt());
        auto dbl_dset = testFile.createDataSet ("dbl_dataset", std::vector<int> (1, 10), H5::DataType::nativeDouble());

        assert (int_dset.getType().bytes() == sizeof (int));
        assert (dbl_dset.getType().bytes() == sizeof (double));
    }
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
    // std::set_terminate (Cow::terminateWithBacktrace);

    testHeap();
    testArray();
    testHdf5();
    testIter();
    testSlicing();

    return 0;
}
