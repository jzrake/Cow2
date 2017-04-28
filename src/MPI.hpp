#ifndef MPI_hpp
#define MPI_hpp

#include <memory>
#include <vector>
#include <functional>



namespace Cow
{
    class MpiCommunicator;
    class MpiCartComm;
    class MpiSession;

    class MpiCommunicator
    {
    public:
        struct Internals;
        static MpiCommunicator world();

        /**
        Default constructor. Initializes the communicator without any MPI
        resources.
        */
        MpiCommunicator();
        MpiCommunicator (Internals*);

        /** Return the rank of this communicator. */
        int rank() const;

        /** Return the number of processes in this communicator. */
        int size() const;

        /**
        Return true if the communicator actually holds an MPI resource. The
        default constructor leaves the communicator in an invalid state.
        */
        bool isValid() const { return internals != nullptr; }

        /**
        Return true if this is rank 0.
        */
        bool isThisMaster() const { return rank() == 0; }

        /**
        Execute the given function on each process in sequence. The callback
        is given the rank.
        */
        void inSequence (std::function<void (int)> callback) const;

        /**
        Create a cartesian topology, with the given number of dimensions,
        consisting of the processes in this communicator.
        */
        MpiCartComm createCartesian (int ndims, std::vector<bool> axisIsDistributed={});

    protected:
        std::shared_ptr<Internals> internals;
    };

    class MpiCartComm : public MpiCommunicator
    {
    public:
        /**
        Default constructor. This constructor will initialize the communicator
        to MPI_COMM_NULL, and is only here so that user classes do not need to
        initialize the cartesian communicator immediately.
        */
        MpiCartComm();

        /**
        Return the rank of the processor at the given coordinates in the
        cartesian topology.
        */
        int getCartRank (std::vector<int> coords) const;

        /**
        Return the rank of the processor that is offset by the given number of
        blocks along the given axis.
        */
        int shift (int axis, int offset) const;

        /** Return the number of dimensions in the array of blocks. */
        int getNumberOfDimensions() const;

        /** Return the dimensions of the array of blocks. */
        std::vector<int> getDimensions() const;

        /**
        Return the coordinate index of the given process in the array of
        blocks. If processRank is -1, then return the coordinates of this
        process.
        */
        std::vector<int> getCoordinates (int processRank=-1) const;
    private:
        MpiCartComm (Internals*);
        friend class MpiCommunicator;
    };

    class MpiSession
    {
    public:
        MpiSession (int argc=0, char** argv=nullptr);
        ~MpiSession();
    };
}

#endif
