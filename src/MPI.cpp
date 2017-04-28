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
MpiCommunicator MpiCommunicator::world()
{
    return new Internals (MPI_COMM_WORLD);
}

MpiCommunicator::MpiCommunicator()
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

MpiCartComm MpiCommunicator::createCartesian (int ndims, std::vector<bool> axisIsDistributed)
{
    std::vector<int> dims (ndims, 0);
    std::vector<int> periods (ndims, 1);
    int reorder = 1;
    MPI_Comm cart;
    MPI_Dims_create (size(), ndims, &dims[0]);
    MPI_Cart_create (internals->comm, ndims, &dims[0], &periods[0], reorder, &cart);
    return new Internals (cart, true);
}




// ============================================================================
MpiCartComm::MpiCartComm()
{

}

MpiCartComm::MpiCartComm (Internals* internals) : MpiCommunicator (internals)
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
    assert (false);
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




// ============================================================================
MpiSession::MpiSession (int argc, char** argv)
{
    MPI_Init (&argc, &argv);
}

MpiSession::~MpiSession()
{
    MPI_Finalize();
}
