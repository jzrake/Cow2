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
}

void DataSet::setTitle (std::string titleToUse)
{
    title = titleToUse;
}

void DataSet::addScalarField (std::string fieldName, Array data)
{
    scalarFields.emplace (fieldName, data);
}

void DataSet::addVectorField (std::string fieldName, Array data)
{
    if (data.size(3) != 3)
    {
        throw std::runtime_error ("VTK vector field must have shape[3] == 3");
    }
    vectorFields.emplace (fieldName, data);
}

void DataSet::write (std::ostream& stream) const
{
    auto S = meshShape;
    double dx = 1.0 / meshShape[0];
    double dy = 1.0 / meshShape[1];
    double dz = 1.0 / meshShape[2];

    stream << "# vtk DataFile Version 2.0\n";
    stream << title << "\n";
    stream << "ASCII\n";
    stream << "DATASET RECTILINEAR_GRID\n";
    stream << "DIMENSIONS " << S[0] + 1 << " " << S[1] + 1 << " " << S[2] + 1 << "\n";

    stream << "X_COORDINATES " << meshShape[0] + 1 << " float\n";
    for (int n = 0; n < meshShape[0] + 1; ++n) stream << -0.5 + dx * n << " "; stream << "\n";

    stream << "Y_COORDINATES " << meshShape[1] + 1 << " float\n";
    for (int n = 0; n < meshShape[1] + 1; ++n) stream << -0.5 + dy * n << " "; stream << "\n";

    stream << "Z_COORDINATES " << meshShape[2] + 1 << " float\n";
    for (int n = 0; n < meshShape[2] + 1; ++n) stream << -0.5 + dz * n << " "; stream << "\n";

    stream << "CELL_DATA " << S[0] * S[1] * S[2] << "\n";

    for (auto field : scalarFields)
    {
        stream << "SCALARS " << field.first << " float\n";
        stream << "LOOKUP_TABLE default\n";

        for (auto x : field.second.transpose())
        {
            stream << x << " ";
        }
        stream << "\n";
    }

    for (auto field : vectorFields)
    {
        stream << "VECTORS " << field.first << " float\n";
        const auto& A = field.second;

        for (int k = 0; k < A.size(2); ++k)
        {
            for (int j = 0; j < A.size(1); ++j)
            {
                for (int i = 0; i < A.size(0); ++i)
                {
                    stream << A (i, j, k, 0) << " " << A (i, j, k, 1) << " " << A (i, j, k, 2) << std::endl;
                }
            }   
        }
    }
}
