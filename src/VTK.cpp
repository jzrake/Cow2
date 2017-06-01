#include <ostream>
#include <cassert>
#include "VTK.hpp"

using namespace Cow;
using namespace Cow::VTK;



/*
# vtk DataFile Version 2.0
Example VTK data file
ASCII
DATASET RECTILINEAR_GRID
DIMENSIONS 10 2 1

X_COORDINATES 10 float
0.0 1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0

Y_COORDINATES 1 float
0.0

Z_COORDINATES 1 float
0.0

FIELD FieldData 2
density 1 10 float
3.8 3.1 3.2 3.3 3.4 3.5 3.6 3.7 3.8 3.9

temperature 1 10 float
2.3 2.1 2.2 2.2 2.4 2.5 2.6 2.7 2.8 2.9
*/




// ============================================================================
RectilinearGrid::RectilinearGrid (Cow::Shape cellsShape) : cellsShape (cellsShape)
{
    title = "title";
    binaryMode = true;

    pointCoordinates[0] = Array (cellsShape[0] + 1);
    pointCoordinates[1] = Array (cellsShape[1] + 1);
    pointCoordinates[2] = Array (cellsShape[2] + 1);

    for (int n = 0; n < cellsShape[0] + 1; ++n) pointCoordinates[0][n] = n;
    for (int n = 0; n < cellsShape[1] + 1; ++n) pointCoordinates[1][n] = n;
    for (int n = 0; n < cellsShape[2] + 1; ++n) pointCoordinates[2][n] = n;
}

void RectilinearGrid::setPointCoordinates (Cow::Array coordinates, int axis)
{
    assert (0 <= axis && axis < 3);
    assert (coordinates.size() == cellsShape[axis] + 1);
    pointCoordinates[axis] = coordinates;
}

void RectilinearGrid::setTitle (std::string titleToUse)
{
    title = titleToUse;
}

void RectilinearGrid::setUseBinaryFormat (bool shouldUseBinaryFormat)
{
    binaryMode = shouldUseBinaryFormat;
}

void RectilinearGrid::addScalarField (std::string fieldName, Array data, MeshLocation location)
{
    if (data.size(3) != 1)
    {
        throw std::runtime_error ("VTK vector field must have shape[3] == 1");
    }

    switch (location)
    {
        case MeshLocation::cell: scalarFieldsCells.emplace (fieldName, data); break;
        case MeshLocation::vert: scalarFieldsVerts.emplace (fieldName, data); break;
        default: break;
    }
}

void RectilinearGrid::addVectorField (std::string fieldName, Array data, MeshLocation location)
{
    if (data.size(3) != 3)
    {
        throw std::runtime_error ("VTK vector field must have shape[3] == 3");
    }

    switch (location)
    {
        case MeshLocation::cell: vectorFieldsCells.emplace (fieldName, data); break;
        case MeshLocation::vert: vectorFieldsVerts.emplace (fieldName, data); break;
        default: break;
    }
}

void RectilinearGrid::write (std::ostream& stream) const
{
    auto S = cellsShape;




    // ------------------------------------------------------------------------
    // Write header
    // ------------------------------------------------------------------------
    stream << "# vtk DataFile Version 3.0\n";
    stream << title << "\n";
    stream << (binaryMode ? "BINARY\n" : "ASCII\n");
    stream << "DATASET RECTILINEAR_GRID\n";
    stream << "DIMENSIONS " << S[0] + 1 << " " << S[1] + 1 << " " << S[2] + 1 << "\n";




    // ------------------------------------------------------------------------
    // Write coordinate data
    // ------------------------------------------------------------------------
    stream << "X_COORDINATES " << cellsShape[0] + 1 << " double\n";

    if (binaryMode)
    {
        stream << pointCoordinates[0].getAllocation().swapBytes (sizeof (double));
    }
    else
    {
        for (int n = 0; n < cellsShape[0] + 1; ++n) stream << pointCoordinates[0][n] << " ";
    }
    stream << std::endl;

    stream << "Y_COORDINATES " << cellsShape[1] + 1 << " double\n";

    if (binaryMode)
    {
        stream << pointCoordinates[1].getAllocation().swapBytes (sizeof (double));
    }
    else
    {
        for (int n = 0; n < cellsShape[1] + 1; ++n) stream << pointCoordinates[1][n] << " ";
    }
    stream << std::endl;

    stream << "Z_COORDINATES " << cellsShape[2] + 1 << " double\n";

    if (binaryMode)
    {
        stream << pointCoordinates[2].getAllocation().swapBytes (sizeof (double));
    }
    else
    {
        for (int n = 0; n < cellsShape[2] + 1; ++n) stream << pointCoordinates[2][n] << " ";
    }
    stream << std::endl;




    // ------------------------------------------------------------------------
    // Write CELL data
    // ------------------------------------------------------------------------
    stream << "CELL_DATA " << S[0] * S[1] * S[2] << "\n";

    for (auto field : scalarFieldsCells)
    {
        stream << "SCALARS " << field.first << " double\n";
        stream << "LOOKUP_TABLE default\n";
        auto Atranspose = field.second.transpose (0, 2);

        if (binaryMode)
        {
            stream << Atranspose.getAllocation().swapBytes (sizeof (double)) << std::endl;
        }
        else
        {
            for (auto x : Atranspose)
            {
                stream << x << " ";
            }
            stream << std::endl;
        }
    }

    for (auto field : vectorFieldsCells)
    {
        stream << "VECTORS " << field.first << " double\n";
        auto Atranspose = field.second.transpose (0, 2);

        if (binaryMode)
        {
            stream << Atranspose.getAllocation().swapBytes (sizeof (double)) << std::endl;
        }
        else
        {
            for (auto it = Atranspose.begin(); it != Atranspose.end(); it += 3)
            {
                stream << it[0] << " " << it[1] << " " << it[2] << std::endl;
            }
        }
    }




    // ------------------------------------------------------------------------
    // Write POINT data
    // ------------------------------------------------------------------------
    stream << "POINT_DATA " << (S[0] + 1) * (S[1] + 1) * (S[2] + 1) << "\n";

    for (auto field : scalarFieldsVerts)
    {
        stream << "SCALARS " << field.first << " double\n";
        stream << "LOOKUP_TABLE default\n";
        auto Atranspose = field.second.transpose (0, 2);

        if (binaryMode)
        {
            stream << Atranspose.getAllocation().swapBytes (sizeof (double)) << std::endl;
        }
        else
        {
            for (auto x : Atranspose)
            {
                stream << x << " ";
            }
            stream << std::endl;
        }
    }

    for (auto field : vectorFieldsVerts)
    {
        stream << "VECTORS " << field.first << " double\n";
        auto Atranspose = field.second.transpose (0, 2);

        if (binaryMode)
        {
            stream << Atranspose.getAllocation().swapBytes (sizeof (double)) << std::endl;
        }
        else
        {
            for (auto it = Atranspose.begin(); it != Atranspose.end(); it += 3)
            {
                stream << it[0] << " " << it[1] << " " << it[2] << std::endl;
            }
        }
    }
}
