#include <iostream> // DEBUG
#include <cassert>
#include "DistributedUniformMesh.hpp"

using namespace Cow;




// ============================================================================
GuardZoneExtension::GuardZoneExtension()
{
    for (int n = 0; n < 5; ++n)
    {
        lower[n] = 0;
        upper[n] = 0;
    }
}

Region GuardZoneExtension::getRegionLower (int axis)
{
    Region R;
    R.lower[axis] = 0;
    R.upper[axis] = lower[axis];
    R.stride[axis] = lower[axis] == 0 ? 0 : 1;
    return R;
}

Region GuardZoneExtension::getRegionUpper (int axis)
{
    Region R;
    R.lower[axis] = -upper[axis];
    R.upper[axis] = 0;
    R.stride[axis] = upper[axis] == 0 ? 0 : 1;
    return R;
}




// ============================================================================
DistributedUniformMesh::DistributedUniformMesh (
    std::vector<int> globalShape,
    MpiCartComm communicator,
    GuardZoneExtension guardZone) :
globalShape (globalShape),
communicator (communicator),
guardZone (guardZone)
{
    assert (globalShape.size() == communicator.getDimensions().size());
    assert (globalShape.size() <= 5);
}

Shape DistributedUniformMesh::getLocalArrayShape() const
{
    auto cartCoords = communicator.getCoordinates();
    auto globalDims = communicator.getDimensions();
    auto localArrayShape = Shape ({{1, 1, 1, 1, 1}});

    for (int n = 0; n < globalShape.size(); ++n)
    {
        localArrayShape[n] = bestPartition (globalShape[n], globalDims[n], cartCoords[n]);
        localArrayShape[n] += guardZone.lower[n] + guardZone.upper[n];
    }
    return localArrayShape;
}

Cow::Array DistributedUniformMesh::createArray() const
{
    return Array (getLocalArrayShape());
}

int DistributedUniformMesh::bestPartition (int numElements, int numPartitions, int whichPartition) const
{
    int elementsPerPartition = numElements / numPartitions;
    int elementsLeftOver = numElements % numPartitions;
    return elementsPerPartition + (whichPartition < elementsLeftOver ? 1 : 0);
}
