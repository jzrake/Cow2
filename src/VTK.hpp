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
    DataSet (Cow::Shape meshShape);
    void setTitle (std::string titleToUse);
    void addField (std::string fieldName, Cow::Array data);
    void write (std::ostream& stream) const;
private:
    Cow::Shape meshShape;
    std::map<std::string, Cow::Array> dataFields;
    std::string title;
};

#endif
