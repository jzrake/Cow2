#include <iostream> // DEBUG
#include <cassert>
#include "DistributedUniformMesh.hpp"

using namespace Cow;




// ============================================================================
DistributedUniformMesh::DistributedUniformMesh (std::vector<int> globalShape, MpiCartComm communicator) :
globalShape (globalShape),
communicator (communicator)
{
    assert (globalShape.size() == communicator.getDimensions().size());
    assert (globalShape.size() <= 5);
}

Shape DistributedUniformMesh::getLocalArrayShape()
{
    Shape localArrayShape;

    auto cartCoords = communicator.getCoordinates();
    auto globalDims = communicator.getDimensions();

    for (int n = 0; n < 5; ++n)
    {
        if (n < globalShape.size())
        {
            localArrayShape[n] = bestPartition (globalShape[n], globalDims[n], cartCoords[n]);
        }
        else
        {
            localArrayShape[n] = 1;
        }
    }

    return localArrayShape;
}

int DistributedUniformMesh::bestPartition (int numElements, int numPartitions, int whichPartition) const
{
    int elementsPerPartition = numElements / numPartitions;
    int elementsLeftOver = numElements % numPartitions;
    return elementsPerPartition + (whichPartition < elementsLeftOver ? 1 : 0);
}
