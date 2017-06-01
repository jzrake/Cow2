#ifndef VTK_hpp
#define VTK_hpp

#include <map>
#include <string>
#include "Array.hpp"

namespace Cow
{
    namespace VTK
    {
        class RectilinearGrid;
    }
}




class Cow::VTK::RectilinearGrid
{
public:
    enum class MeshLocation { vert, edge, face, cell };
    RectilinearGrid (Cow::Shape cellsShape);
    void setPointCoordinates (Cow::Array coordinates, int axis);
    void setTitle (std::string titleToUse);
    void setUseBinaryFormat (bool shouldUseBinaryFormat);
    void addScalarField (std::string fieldName, Cow::Array data, MeshLocation location=MeshLocation::cell);
    void addVectorField (std::string fieldName, Cow::Array data, MeshLocation location=MeshLocation::cell);
    void write (std::ostream& stream) const;
private:
    Cow::Shape cellsShape;
    Cow::Array pointCoordinates[3];
    std::map<std::string, Cow::Array> scalarFieldsCells;
    std::map<std::string, Cow::Array> vectorFieldsCells;
    std::map<std::string, Cow::Array> scalarFieldsVerts;
    std::map<std::string, Cow::Array> vectorFieldsVerts;
    std::string title;
    bool binaryMode;
};

#endif
