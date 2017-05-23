#ifndef VTK_hpp
#define VTK_hpp

#include <map>
#include <string>
#include "Array.hpp"

namespace Cow
{
    namespace VTK
    {
        class DataSet;
    }
}




class Cow::VTK::DataSet
{
public:
    enum class MeshLocation { vert, edge, face, cell };
    DataSet (Cow::Shape meshShape);
    void setTitle (std::string titleToUse);
    void addScalarField (std::string fieldName, Cow::Array data, MeshLocation location=MeshLocation::cell);
    void addVectorField (std::string fieldName, Cow::Array data, MeshLocation location=MeshLocation::cell);
    void write (std::ostream& stream) const;
private:
    Cow::Shape meshShape;
    std::map<std::string, Cow::Array> scalarFieldsCells;
    std::map<std::string, Cow::Array> vectorFieldsCells;
    std::map<std::string, Cow::Array> scalarFieldsVerts;
    std::map<std::string, Cow::Array> vectorFieldsVerts;
    std::string title;
};

#endif
