#include <iostream> // DEBUG
#include <cassert>
#include <mpi.h>
#include "MPI.hpp"

using namespace Cow;




// ============================================================================
struct MpiCommunicator::Internals
{
public:
    Internals (MPI_Comm commToUse, bool stealOwnership=false)
    {
        if (stealOwnership)
        {
            comm = commToUse;
        }
        else
        {
            MPI_Comm_dup (commToUse, &comm);
        }
    }

    ~Internals()
    {
        MPI_Comm_free (&comm);
    }

    MPI_Comm comm;
};




// ============================================================================
struct MpiRequest::Internals
{
public:
    Internals (MPI_Request request) : request (request)
    {

    }

    ~Internals()
    {
        MPI_Request_free (&request);
    }

    MPI_Request request;
};




// ============================================================================
struct MpiDataType::Internals
{
public:
    Internals (MPI_Datatype typeToUse, bool stealOwnership=false)
    {
        if (stealOwnership)
        {
            type = typeToUse;
        }
        else
        {
            MPI_Type_dup (typeToUse, &type);
        }
    }

    ~Internals()
    {
        MPI_Type_free (&type);
    }

    MPI_Datatype type;
};




// ============================================================================
MpiRequest::MpiRequest (Internals* internals) : internals (internals)
{

}

MpiRequest::~MpiRequest()
{
    if (! test())
    {
        std::cerr
        << "Warning: an MPI request is going out of scope "
        << "before being fulfilled."
        << std::endl;
    }
}

void MpiRequest::cancel()
{
    // Note this function takes a pointer to the request handle, as it intends
    // to modify its value if the request is fulfilled.
    MPI_Cancel (&internals->request);
}

void MpiRequest::wait()
{
    // Note this function takes a pointer to the request handle, as it intends
    // to modify its value if the request is fulfilled.
    MPI_Status status;
    MPI_Wait (&internals->request, &status);
}

bool MpiRequest::test()
{
    // Note this function takes a pointer to the request handle, as it intends
    // to modify its value if the request is fulfilled.
    int result;
    MPI_Status status;
    MPI_Test (&internals->request, &result, &status);
    return result;
}

bool MpiRequest::getStatus() const
{
    // Note this function does *not* take a pointer to the request handle, as
    // it does not modify its value even if the request is fulfilled.
    int result;
    MPI_Status status;
    MPI_Request_get_status (internals->request, &result, &status);
    return result;
}



// ============================================================================
MpiCommunicator MpiCommunicator::world()
{
    return new Internals (MPI_COMM_WORLD);
}

MpiCommunicator::MpiCommunicator()
{

}

MpiCommunicator::~MpiCommunicator()
{
}

MpiCommunicator::MpiCommunicator (Internals* internals) : internals (internals)
{

}

int MpiCommunicator::rank() const
{
    int R;
    MPI_Comm_rank (internals->comm, &R);
    return R;
}

int MpiCommunicator::size() const
{
    int S;
    MPI_Comm_size (internals->comm, &S);
    return S;
}

void MpiCommunicator::inSequence (std::function<void (int)> callback) const
{
    for (int n = 0; n < size(); ++n)
    {
        if (rank() == n)
        {
            callback (n);
        }
        MPI_Barrier (internals->comm);
    }
}

void MpiCommunicator::onMasterOnly (std::function<void()> callback) const
{
    if (isThisMaster())
    {
        callback();
    }
    MPI_Barrier (internals->comm);
}

MpiCartComm MpiCommunicator::createCartesian (int ndims, std::vector<bool> axisIsDistributed) const
{
    std::vector<int> dims (ndims, 0);
    std::vector<int> periods (ndims, 1);
    int reorder = 1;
    int axisCounter = 0;

    // Set the dims array to have size 1 where axisIsDistributed is false. An
    // entry of 0 in dims causes MPI_Dims_create to decompose that axis.

    for (auto isDistributed : axisIsDistributed)
    {
        if (! isDistributed)
        {
            dims[axisCounter] = 1;
        }
        ++axisCounter;
    }

    MPI_Comm cart;
    MPI_Dims_create (size(), ndims, &dims[0]);
    MPI_Cart_create (internals->comm, ndims, &dims[0], &periods[0], reorder, &cart);
    return new Internals (cart, true);
}

MpiCommunicator MpiCommunicator::split (int color) const
{
    int key = 0; // Determines rank ordering, 0 uses old ordering.
    MPI_Comm comm;
    MPI_Comm_split (internals->comm, color, key, &comm);
    return new Internals (comm, true);
}

double MpiCommunicator::minimum (double x) const
{
    double result;
    MPI_Allreduce (&x, &result, 1, MPI_DOUBLE, MPI_MIN, internals->comm);
    return result;
}

double MpiCommunicator::maximum (double x) const
{
    double result;
    MPI_Allreduce (&x, &result, 1, MPI_DOUBLE, MPI_MAX, internals->comm);
    return result;
}

std::vector<double> MpiCommunicator::sum (const std::vector<double>& A) const
{
    auto ret = A;
    MPI_Allreduce (&A[0], &ret[0], ret.size(), MPI_DOUBLE, MPI_SUM, internals->comm);
    return ret;
}




// ============================================================================
MpiCartComm::MpiCartComm()
{
}

MpiCartComm::MpiCartComm (Internals* internals) : MpiCommunicator (internals)
{
}

MpiCartComm::~MpiCartComm()
{
}

int MpiCartComm::getCartRank (std::vector<int> coords) const
{
    int R;
    MPI_Cart_rank (internals->comm, &coords[0], &R);
    return R;
}

int MpiCartComm::shift (int axis, int offset) const
{
    int source = rank();
    int dest;
    MPI_Cart_shift (internals->comm, axis, offset, &source, &dest);
    return dest;
}

int MpiCartComm::getNumberOfDimensions() const
{
    int ndims;
    MPI_Cartdim_get (internals->comm, &ndims);
    return ndims;
}

std::vector<int> MpiCartComm::getDimensions() const
{
    int ndims = getNumberOfDimensions();
    std::vector<int> dims (ndims);
    std::vector<int> periods (ndims);
    std::vector<int> coords (ndims);
    MPI_Cart_get (internals->comm, ndims, &dims[0], &periods[0], &coords[0]);
    return dims;
}

std::vector<int> MpiCartComm::getCoordinates (int processRank) const
{
    int R = processRank == -1 ? rank() : processRank;
    int ndims = getNumberOfDimensions();
    std::vector<int> coords (ndims);
    MPI_Cart_coords (internals->comm, R, ndims, &coords[0]);
    return coords;
}

void MpiCartComm::shiftExchange (Array& A, int axis, char sendDirection, Region send, Region recv) const
{
    assert (sendDirection == 'L' || sendDirection == 'R');

    auto sendType = MpiDataType::subarray (A.shape(), send);
    auto recvType = MpiDataType::subarray (A.shape(), recv);

    int sendRank = shift (axis, sendDirection == 'L' ? -1 : +1);
    int recvRank = shift (axis, sendDirection == 'L' ? +1 : -1);

    MPI_Status status;

    MPI_Sendrecv (
        A.begin(), 1, sendType.internals->type, sendRank, 12345,
        A.begin(), 1, recvType.internals->type, recvRank, 12345,
        internals->comm, &status);
}




// ============================================================================
MpiDataType MpiDataType::nativeInt()
{
    return new Internals (MPI_INT);
}

MpiDataType MpiDataType::nativeDouble()
{
    return new Internals (MPI_DOUBLE);
}

MpiDataType MpiDataType::subarray (Shape S, Region R)
{
    R = R.absolute (S);
    int ndims = R.getShapeVector().size();
    auto sizes = std::vector<int>();
    auto subsizes = std::vector<int>();
    auto starts = std::vector<int>();

    for (int n = 0; n < ndims; ++n)
    {
        auto range = R.range(n);
        sizes.push_back (S[n]);
        subsizes.push_back (range.size());
        starts.push_back (range.lower);
        assert (range.stride == 1);
    }

    MPI_Datatype type;
    MPI_Type_create_subarray (ndims,
        &sizes[0],
        &subsizes[0],
        &starts[0],
        MPI_ORDER_C,
        MPI_DOUBLE,
        &type);
    MPI_Type_commit (&type);

    return new Internals (type, true);
}

MpiDataType::MpiDataType()
{

}

MpiDataType::MpiDataType (Internals* internals) : internals (internals)
{

}

MpiDataType::~MpiDataType()
{
}

std::size_t MpiDataType::size() const
{
    int S;
    MPI_Type_size (internals->type, &S);
    return S;    
}


// ============================================================================
MpiSession::MpiSession (int argc, char** argv)
{
    MPI_Init (&argc, &argv);
}

MpiSession::~MpiSession()
{
    MPI_Finalize();
}
