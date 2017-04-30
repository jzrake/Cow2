#ifndef DistributedUniformMesh_hpp
#define DistributedUniformMesh_hpp

#include "Array.hpp"
#include "MPI.hpp"




namespace Cow
{
    class DistributedUniformMesh
    {
    public:
        DistributedUniformMesh (std::vector<int> globalShape, MpiCartComm communicator);

        void configureAxis (bool isDistributed, int numGuard);

        /** Communicate across processor boundaries. Must be called by all processors.*/
        void synchronize (Array& A) const;

        /** Get the shape required for the local data block. */
        Cow::Shape getLocalArrayShape() const;

        /** Generate a new array with the shape of the local data block. */
        Cow::Array createArray() const;

    private:
        /** @internal */
        int bestPartition (int numElements, int numPartitions, int whichPartition) const;

        const std::vector<int> globalShape;
        std::vector<int> guardCellCount;
        Cow::MpiCartComm communicator;
    };
}


#endif
