#ifndef DistributedUniformMesh_hpp
#define DistributedUniformMesh_hpp

#include "Array.hpp"
#include "MPI.hpp"

namespace Cow
{
    class GuardZoneExtension;
    class DistributedUniformMesh;
}




class Cow::GuardZoneExtension
{
public:
    GuardZoneExtension();
    Region getRegionLower (int axis);
    Region getRegionUpper (int axis);
    int lower[5];
    int upper[5];
};




class Cow::DistributedUniformMesh
{
public:
    DistributedUniformMesh (std::vector<int> globalShape, MpiCartComm communicator, GuardZoneExtension guardZone);

    /**
    Synchronize data in guard regions with adjacent processors in the
    cartesian topology. Must be called by all processors in the cartesian
    communicator.
    */
    void synchronize (Array& A) const;

    /**
    Get the shape required for the local data block. This shape accounts for
    the user-specified size of the guard zone region.
    */
    Cow::Shape getLocalArrayShape() const;

    /** Generate a new array with the shape of the local data block. */
    Cow::Array createArray() const;

private:
    /** @internal */
    int bestPartition (int numElements, int numPartitions, int whichPartition) const;

    const std::vector<int> globalShape;
    std::vector<int> guardCellCount;
    Cow::MpiCartComm communicator;
    GuardZoneExtension guardZone;
};


#endif
