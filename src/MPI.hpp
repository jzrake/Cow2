#ifndef MPI_hpp
#define MPI_hpp

#include <memory>
#include <vector>
#include <functional>
#include "Array.hpp"




namespace Cow
{
    class MpiCommunicator;
    class MpiCartComm;
    class MpiDataType;
    class MpiRequest;
    class MpiSession;


    /**
    A mirror of the MPI_Status data structure, but also with flag which
    indicates the result of probe and test calls.
    */
    class MpiStatus
    {
    public:
        int flag;
        int count;
        int cancelled;
        int source;
        int tag;
        int error;
    };


    /**
    Class to encapulate certain responsibilities of an MPI request, which
    facilitate non-blocking communications. There are helpful resources at
    this link:

    https://cvw.cac.cornell.edu/mpip2p/waittestfree
    */
    class MpiRequest
    {
    public:
        /**
        Destructor, called when the request object goes out of scope. Will log
        a message to stderr if going out of scope before the request has been
        completed or canceled, which acts as some built-in assurance that
        dangling request handles are not accumulating in the MPI system.
        */
        ~MpiRequest();

        /**
        Cancels the request. It is safe for the requeset to go out of scope
        after this call.
        */
        void cancel();

        /**
        Blocking call that returns only when a specified operation has been
        completed (e.g., the send buffer is safe to access). This call should
        be inserted at the point where the next section of code depends on the
        buffer, because it forces the process to block until the buffer is
        ready. After this function returns, it is safe for the request to go
        out of scope.
        */
        void wait();

        /**
        Non-blocking counterpart to wait. Returns true if the request has been
        completed. If it has, then test also deactivates the request; it is safe
        for the request to go out of scope.
        */
        bool test();

        /**
        Calls MPI_Request_get_status, which returns the status of the request
        without triggering completion of the message. Even if this method
        returns true, it is still necessary to call test() before alloing the
        object to go out of scope.
        */
        bool getStatus() const;

    private:
        friend class MpiCommunicator;
        struct Internals;
        MpiRequest (Internals*);
        std::unique_ptr<Internals> internals;
    };


    /**
    Class to encapulate certain responsibilities of an MPI communicator.
    */
    class MpiCommunicator
    {
    public:
        static MpiCommunicator world();

        /**
        Default constructor. Initializes the communicator without any MPI
        resources.
        */
        MpiCommunicator();

        /**
        Return the rank of this communicator.
        */
        int rank() const;

        /**
        Return the number of processes in this communicator.
        */
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
        Execute the given function on the master process, while other processes
        wait.
        */
        void onMasterOnly (std::function<void()> callback) const;

        /**
        Create a cartesian topology, with the given number of dimensions,
        consisting of the processes in this communicator. If axisIsDistributed
        is empty, then the topology will be distributed on all ndims axes.
        Otherwise, the axes n for which (axisIsDistributed[n] == false) will
        have size 1 in the communicator.
        */
        MpiCartComm createCartesian (int ndims, std::vector<bool> axisIsDistributed={}) const;

        /**
        Create a new communicator by calling MPI_Comm_split, with the given color.
        */
        MpiCommunicator split (int color) const;

        /**
        Return the minimum value over all participating processes, to all
        processes. This invokes an MPI_Allreduce opertion.
        */
        double minimum (double x) const;

        /**
        Return the maximum value over all participating processes, to all
        processes. This invokes an MPI_Allreduce opertion.
        */
        double maximum (double x) const;

        /**
        Return a vector of quantities, each of which is summed over all
        participating ranks.
        */
        std::vector<double> sum (const std::vector<double>& A) const;

        /**
        These methods are a draft of an interface for synchronous and
        asynchronous send and receive operations. They are not implemented yet.
        Consider having non-blocking sends and receives (post and request) keep
        the request objects as member data in the communicator.
        */
        void send (const HeapAllocation& bufferToSend, int rank, int tag=0) const;
        MpiRequest post (const HeapAllocation& bufferToPost, int rank, int tag=0) const;
        HeapAllocation receive (int rank, int tag=0) const;
        MpiRequest request (int rank, int tag=0) const;

    protected:
        struct Internals;
        MpiCommunicator (Internals*);
        std::shared_ptr<Internals> internals;
    };



    /**
    Class to encapulate certain responsibilities of an MPI cartesian topology.
    */
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

        /**
        Return the number of dimensions in the array of blocks.
        */
        int getNumberOfDimensions() const;

        /**
        Return the dimensions of the array of blocks.
        */
        std::vector<int> getDimensions() const;

        /**
        Return the coordinate index of the given process in the array of
        blocks. If processRank is -1, then return the coordinates of this
        process.
        */
        std::vector<int> getCoordinates (int processRank=-1) const;

        /**
        Execute an MPI send-recv operation by shifting the cartesian topology
        along the given axis. Data will be sent in the direction specified, by
        either 'L' or 'R'. For example, if sendDirection is 'R' then the
        region of A covered by 'send' is sent to the process on the right,
        while data in the 'recv' region of A is over-written with data
        received from the process to the left. The send and receive regions
        may be relative or absolute, but must not overlap.
        */
        void shiftExchange (Array& A, int axis, char sendDirection, Region send, Region recv) const;

    private:
        MpiCartComm (Internals*);
        friend class MpiCommunicator;
    };



    /**
    Class to encapulate certain responsibilities of an MPI data type.
    */
    class MpiDataType
    {
    public:
        static MpiDataType nativeInt();
        static MpiDataType nativeDouble();

        /**
        Create a new MPI array data type, of doubles, which corresponds to the
        given absolute region. The returned array data type has C ordering.
        The region must have stride length equal to 1 on each axis.
        */
        static MpiDataType subarray (Cow::Shape S, Cow::Region R);

        /**
        Default constructor, creates an unusable data type.
        */
        MpiDataType();

        /**
        Return the size, in bytes, of the data type.
        */
        std::size_t size() const;

    protected:
        friend class MpiCartComm;
        struct Internals;
        MpiDataType (Internals*);
        std::shared_ptr<Internals> internals;
    };




    /**
    Class to initialize and finalize an MPI session using RAII. Just place one
    of these in the main() function, and MPI will be initialized. It is
    finalized when this object goes out of scope. Be sure to only create one
    of these in your application. Most MPI implementations do not allow
    MPI_Init to be called again after MPI_Finalize.
    */
    class MpiSession
    {
    public:
        MpiSession (int argc=0, char** argv=nullptr);
        ~MpiSession();
    };
}

#endif
