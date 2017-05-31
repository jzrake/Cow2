#include <ostream>
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
DataSet::DataSet (Cow::Shape meshShape) : meshShape (meshShape)
{
    title = "title";
    binaryMode = true;
}

void DataSet::setTitle (std::string titleToUse)
{
    title = titleToUse;
}

void DataSet::setUseBinaryFormat (bool shouldUseBinaryFormat)
{
    binaryMode = shouldUseBinaryFormat;
}

void DataSet::addScalarField (std::string fieldName, Array data, MeshLocation location)
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

void DataSet::addVectorField (std::string fieldName, Array data, MeshLocation location)
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

void DataSet::write (std::ostream& stream) const
{
    auto S = meshShape;

    // This kludge must do until we pass mesh coordinates
    double Ax = meshShape[2] > 1 ? double (meshShape[2]) / meshShape[0] : 1.;
    double Ay = meshShape[2] > 1 ? double (meshShape[2]) / meshShape[1] : 1.;
    double dx = meshShape[0] > 1 ? 1.0 / meshShape[0] / Ax : 1e-2;
    double dy = meshShape[1] > 1 ? 1.0 / meshShape[1] / Ay : 1e-2;
    double dz = meshShape[2] > 1 ? 1.0 / meshShape[2] : 1e-2;

    auto xCoordinates = Array (meshShape[0] + 1);
    auto yCoordinates = Array (meshShape[1] + 1);
    auto zCoordinates = Array (meshShape[2] + 1);

    for (int n = 0; n < meshShape[0] + 1; ++n) xCoordinates[n] = -0.5 + dx * n;
    for (int n = 0; n < meshShape[1] + 1; ++n) yCoordinates[n] = -0.5 + dy * n;
    for (int n = 0; n < meshShape[2] + 1; ++n) zCoordinates[n] = -0.5 + dz * n;




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
    stream << "X_COORDINATES " << meshShape[0] + 1 << " double\n";

    if (binaryMode)
    {
        stream << xCoordinates.getAllocation().swapBytes (sizeof (double));
    }
    else
    {
        for (int n = 0; n < meshShape[0] + 1; ++n) stream << xCoordinates[n] << " ";
    }
    stream << std::endl;

    stream << "Y_COORDINATES " << meshShape[1] + 1 << " double\n";

    if (binaryMode)
    {
        stream << yCoordinates.getAllocation().swapBytes (sizeof (double));
    }
    else
    {
        for (int n = 0; n < meshShape[1] + 1; ++n) stream << yCoordinates[n] << " ";
    }
    stream << std::endl;

    stream << "Z_COORDINATES " << meshShape[2] + 1 << " double\n";

    if (binaryMode)
    {
        stream << zCoordinates.getAllocation().swapBytes (sizeof (double));
    }
    else
    {
        for (int n = 0; n < meshShape[2] + 1; ++n) stream << zCoordinates[n] << " ";
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
