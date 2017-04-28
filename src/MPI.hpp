#ifndef MPI_hpp
#define MPI_hpp

#include <memory>
#include <vector>




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
        static MpiCommunicator null();
        MpiCommunicator();
        MpiCommunicator (Internals*);
        MpiCartComm createCartesian (int ndims);
        int rank() const;
        int size() const;
    protected:
        std::shared_ptr<Internals> internals;
    };

    class MpiCartComm : public MpiCommunicator
    {
    public:
        int shift (int dim, int offset) const;
        int getCartRank (std::vector<int> coords) const;
        int getNumberOfDimensions() const;
        std::vector<int> getCoordinates() const;
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
