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



DataSet::DataSet (Cow::Shape meshShape) : meshShape (meshShape)
{
    title = "title";
}

void DataSet::setTitle (std::string titleToUse)
{
    title = titleToUse;
}

void DataSet::addField (std::string fieldName, Array data)
{
    dataFields.emplace (fieldName, data);
}

void DataSet::write (std::ostream& stream) const
{
    auto fieldName = "fieldName";
    auto S = meshShape;
    stream << "# vtk DataFile Version 2.0\n";
    stream << title << "\n";
    stream << "ASCII\n";
    stream << "DATASET RECTILINEAR_GRID\n";
    stream << "DIMENSIONS " << S[0] << " " << S[1] << " " << S[2] << "\n";

    stream << "X_COORDINATES " << meshShape[0] << " float\n";
    for (int n = 0; n < meshShape[0]; ++n) stream << n << " "; stream << "\n";

    stream << "Y_COORDINATES " << meshShape[1] << " float\n";
    for (int n = 0; n < meshShape[1]; ++n) stream << n << " "; stream << "\n";

    stream << "Z_COORDINATES " << meshShape[2] << " float\n";
    for (int n = 0; n < meshShape[2]; ++n) stream << n << " "; stream << "\n";

    stream << "FIELD " << fieldName << " " << dataFields.size() << "\n";

    for (auto field : dataFields)
    {
        auto& A = field.second;
        int numComponents = 1;
        int numElements = A.size();
        
        stream << field.first << " " << numComponents << " " << numElements << " float \n";

        for (auto x : A)
        {
            stream << x << " ";
        }
        stream << "\n";
    }
}
