#include <iostream> // DEBUG
#include <cassert>
#include <hdf5.h>
#include "HDF5.hpp"

using namespace Cow;




// ============================================================================
class H5::Object
{
public:
    Object (hid_t id, char type) : id (id), type (type) {}
    ~Object()
    {
        switch (type)
        {
            case 'F': H5Fclose (id); break;
            case 'G': H5Gclose (id); break;
            case 'D': H5Dclose (id); break;
            case 'S': H5Sclose (id); break;
            case 'T': H5Tclose (id); break;
            default: assert (false);
        }
    }

    hid_t id;
    char type;
};




// ============================================================================
H5::Group H5::GroupCreator::createGroup (std::string name)
{
    Object* object = getObject();
    hid_t id = H5Gcreate (object->id, name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    return H5::Group (new Object (id, 'G'));
}




// ============================================================================
H5::DataSet H5::DataSetCreator::createDataSet (std::string name, const DataType& type)
{
    Object* object = getObject();
    DataSpace space = DataSpace::scalar();

    hid_t datasetId = H5Dcreate (
        object->id,
        name.c_str(),
        type.object->id,
        space.object->id,
        H5P_DEFAULT,
        H5P_DEFAULT,
        H5P_DEFAULT);

    return H5::DataSet (new Object (datasetId, 'D'));
}

H5::DataSet H5::DataSetCreator::createDataSet (std::string name, std::vector<int> shape)
{
    Object* object = getObject();
    DataSpace space = DataSpace::simple (shape);

    hid_t datasetId = H5Dcreate (
        object->id,
        name.c_str(),
        H5T_NATIVE_DOUBLE,
        space.object->id,
        H5P_DEFAULT,
        H5P_DEFAULT,
        H5P_DEFAULT);

    return H5::DataSet (new Object (datasetId, 'D'));
}




// ============================================================================
H5::DataSet::DataSet (Object* object) : object (object)
{

}

void H5::DataSet::write (const Cow::Array& A)
{
    DataSpace space = getSpace();

    assert (A.getShapeVector() == space.getShape());

    const void* buffer = &A[0];
    Array B;

    if (A.getOrdering() == 'F')
    {
        B = A.transpose();
        buffer = &B[0];
    }

    H5Dwrite (
        object->id,
        H5T_NATIVE_DOUBLE,
        space.object->id,
        space.object->id,
        H5P_DEFAULT,
        buffer);
}

void H5::DataSet::write (std::string S)
{
    DataType type = getType();
    DataSpace space = getSpace();

    assert (space.getShape().size() == 0);
    assert (H5Tget_class (type.object->id) == H5T_STRING);
    assert (H5Tget_size (type.object->id) == S.size());

    const void* buffer = S.c_str();

    H5Dwrite (
        object->id,
        type.object->id,
        space.object->id,
        space.object->id,
        H5P_DEFAULT,
        buffer);
}

H5::DataSpace H5::DataSet::getSpace()
{
    hid_t spaceId = H5Dget_space (object->id);
    return new Object (spaceId, 'S');
}

H5::DataType H5::DataSet::getType()
{
    hid_t typeId = H5Dget_type (object->id);
    return new Object (typeId, 'T');
}




// ============================================================================
H5::DataSpace::DataSpace (Object* object) : object (object)
{

}

H5::DataSpace H5::DataSpace::scalar()
{
    hid_t id = H5Screate (H5S_SCALAR);
    return new Object (id, 'S');
}

H5::DataSpace H5::DataSpace::simple (std::vector<int> shape)
{
    std::vector<hsize_t> current_dims;
    std::vector<hsize_t> maximum_dims;

    for (auto n : shape)
    {
        current_dims.push_back (n);
        maximum_dims.push_back (n);
    }
    hid_t id = H5Screate_simple (shape.size(), &current_dims[0], &maximum_dims[0]);
    return new Object (id, 'S');
}

std::vector<int> H5::DataSpace::getShape()
{
    int ndims = H5Sget_simple_extent_ndims (object->id);
    std::vector<hsize_t> hdims (ndims);
    H5Sget_simple_extent_dims (object->id, &hdims[0], nullptr);
    std::vector<int> dims;

    for (auto n : hdims)
    {
        dims.push_back (n);
    }
    return dims;
}




// ============================================================================
H5::DataType H5::DataType::nativeInt()
{
    hid_t id = H5Tcopy (H5T_NATIVE_INT);
    return new Object (id, 'T');
}

H5::DataType H5::DataType::nativeDouble()
{
    hid_t id = H5Tcopy (H5T_NATIVE_DOUBLE);
    return new Object (id, 'T');
}

H5::DataType H5::DataType::nativeString (int length)
{
    hid_t id = H5Tcopy (H5T_C_S1);
    H5Tset_size (id, length);
    return new Object (id, 'T');
}

H5::DataType::DataType (Object* object) : object (object)
{

}




// ============================================================================
H5::File::File (std::string name)
{
    hid_t id = H5Fcreate (name.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    object.reset (new Object (id, 'F'));
};

int H5::File::getObjectCount()
{
    return H5Fget_obj_count (object->id, H5F_OBJ_ALL);
}




// ============================================================================
H5::Group::Group (Object* object) : object (object)
{

}
