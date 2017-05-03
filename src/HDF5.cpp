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

H5::DataSet H5::DataSetCreator::write (std::string name, std::string value)
{
    auto ds = createDataSet (name, H5::DataType::nativeString (value.size()));
    ds.write (value);
    return ds;
}

H5::DataSet H5::DataSetCreator::write (std::string name, double value)
{
    auto ds = createDataSet (name, H5::DataType::nativeDouble());
    ds.write (value);
    return ds;
}

H5::DataSet H5::DataSetCreator::write (std::string name, int value)
{
    auto ds = createDataSet (name, H5::DataType::nativeInt());
    ds.write (value);
    return ds;
}

H5::DataSet H5::DataSetCreator::write (std::string name, const Array& A)
{
    auto ds = createDataSet (name, A.getShapeVector());
    ds.write (A);
    return ds;    
}

H5::DataSet H5::DataSetCreator::write (std::string name, const Array::Reference reference)
{
    auto ds = createDataSet (name, reference.getRegion().getShapeVector());
    ds[Region()] = reference;
    return ds;
}




// ============================================================================
H5::DataSet::DataSet (Object* object) : object (object)
{

}

void H5::DataSet::writeBuffer (DataType dataType,
    DataSpace memorySpace,
    DataSpace fileSpace,
    const HeapAllocation& buffer) const
{
    H5Dwrite (
        object->id,
        dataType.object->id,
        memorySpace.object->id,
        fileSpace.object->id,
        H5P_DEFAULT,
        buffer.begin());
}

void H5::DataSet::write (const Cow::Array& A) const
{
    auto space = getSpace();
    auto type = DataType::nativeDouble();

    assert (A.getShapeVector() == space.getShape());

    if (A.getOrdering() == 'F')
    {
        auto B = A.transpose();
        writeBuffer (type, space, space, B.getAllocation());
    }
    else
    {
        writeBuffer (type, space, space, A.getAllocation());
    }
}

void H5::DataSet::write (std::string S) const
{
    DataType type = getType();
    DataSpace space = getSpace();
    auto buffer = HeapAllocation (S);

    assert (space.getShape().size() == 0);
    assert (H5Tget_class (type.object->id) == H5T_STRING);
    assert (H5Tget_size (type.object->id) == S.size());

    writeBuffer (type, space, space, buffer);
}

H5::DataSpace H5::DataSet::getSpace() const
{
    hid_t spaceId = H5Dget_space (object->id);
    return new Object (spaceId, 'S');
}

H5::DataType H5::DataSet::getType() const
{
    hid_t typeId = H5Dget_type (object->id);
    return new Object (typeId, 'T');
}

H5::DataSet::Reference H5::DataSet::operator[] (Region region)
{
    return Reference (*this, region.absolute (getSpace().getShape()));
}




// ============================================================================
H5::DataSet::Reference::Reference (DataSet& D, Region R) : D(D), R(R)
{
    assert (! R.isRelative());
}

const Array& H5::DataSet::Reference::operator= (Array& A)
{
    this->operator= (A[Region()]);
    return A;
}

const Array::Reference& H5::DataSet::Reference::operator= (const Array::Reference& ref)
{
    auto dataType = DataType::nativeDouble();
    auto fileSpace = D.getSpace();
    auto memorySpace = DataSpace::simple (ref.getArray().getShapeVector());

    fileSpace.select (R);
    memorySpace.select (ref.getRegion());

    D.writeBuffer (dataType, memorySpace, fileSpace, ref.getArray().getAllocation());

    return ref;
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

std::vector<int> H5::DataSpace::getShape() const
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

void H5::DataSpace::select (Region R)
{
    auto region = R.absolute (getShape());
    auto start = std::vector<hsize_t> (5);
    auto strde = std::vector<hsize_t> (5);
    auto count = std::vector<hsize_t> (5);
    auto block = std::vector<hsize_t> (5);

    for (int n = 0; n < 5; ++n)
    {
        start[n] = region.lower[n];
        strde[n] = region.stride[n];
        count[n] = region.range (n).size();
        block[n] = 1;
    }

    H5Sselect_hyperslab (
        object->id,
        H5S_SELECT_SET,
        &start[0],
        &strde[0],
        &count[0],
        &block[0]);
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
