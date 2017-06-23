#include <iostream> // DEBUG
#include <cassert>
#include <cstring>
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
            case 'P': H5Pclose (id); break;
            default: assert (false);
        }
    }

    hid_t id;
    char type;
};




// ============================================================================
H5::PropertyList::Base::Base (long long typeIdentifier)
{
    hid_t id = H5Pcreate (typeIdentifier);
    object.reset (new Object (id, 'P'));
}

const H5::Object* H5::PropertyList::Base::getObject() const
{
    return object.get();
}




// ============================================================================
H5::PropertyList::DataSetCreate::DataSetCreate() : Base (H5P_DATASET_CREATE) {}

H5::PropertyList::DataSetCreate& H5::PropertyList::DataSetCreate::setChunk (std::vector<int> dims)
{
    std::vector<hsize_t> hdims (dims.begin(), dims.end());
    H5Pset_chunk (getObject()->id, hdims.size(), &hdims[0]);
    return *this;
}




// ============================================================================
bool H5::Location::hasGroup (std::string name) const
{
    auto object = getObject();
    bool exists = H5Lexists (object->id, name.c_str(), H5P_DEFAULT);
    
    if (exists)
    {
        H5O_info_t info;
        H5Oget_info_by_name (object->id, name.c_str(), &info, H5P_DEFAULT);
        return info.type == H5O_TYPE_GROUP;
    }
    return false;
}

bool H5::Location::hasGroups (std::vector<std::string> names) const
{
    for (auto name : names)
    {
        if (! hasGroup (name)) return false;
    }
    return true;
}

H5::Group H5::Location::getGroup (std::string name) const
{
    auto object = getObject();
    auto id = H5Gopen (object->id, name.c_str(), H5P_DEFAULT);
    return H5::Group (new Object (id, 'G'));    
}

H5::Group H5::Location::createGroup (std::string name)
{
    auto object = getObject();
    auto id = H5Gcreate (object->id, name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    return H5::Group (new Object (id, 'G'));
}

H5::DataSet H5::Location::getDataSet (std::string name) const
{
    auto object = getObject();
    auto datasetId = H5Dopen (object->id, name.c_str(), H5P_DEFAULT);
    return H5::DataSet (new Object (datasetId, 'D'));
}

bool H5::Location::hasDataSet (std::string name) const
{
    auto object = getObject();
    bool exists = H5Lexists (object->id, name.c_str(), H5P_DEFAULT);

    if (exists)
    {
        H5O_info_t info;
        H5Oget_info_by_name (object->id, name.c_str(), &info, H5P_DEFAULT);
        return info.type == H5O_TYPE_DATASET;
    }
    return false;
}

bool H5::Location::hasDataSets (std::vector<std::string> names) const
{
    for (auto name : names)
    {
        if (! hasDataSet (name)) return false;
    }
    return true;
}

H5::DataSet H5::Location::createDataSet (std::string name, DataType type)
{
    auto space = DataSpace();

    hid_t datasetId = H5Dcreate (
        getObject()->id,
        name.c_str(),
        type.object->id,
        space.object->id,
        H5P_DEFAULT,
        H5P_DEFAULT,
        H5P_DEFAULT);

    return H5::DataSet (new Object (datasetId, 'D'));
}

H5::DataSet H5::Location::createDataSet (
    std::string name,
    std::vector<int> shape,
    DataType type,
    PropertyList::DataSetCreate properties)
{
    auto space = DataSpace (shape);

    hid_t datasetId = H5Dcreate (
        getObject()->id,
        name.c_str(),
        type.object->id,
        space.object->id,
        H5P_DEFAULT,
        properties.getObject()->id,
        H5P_DEFAULT);

    return H5::DataSet (new Object (datasetId, 'D'));
}

void H5::Location::iterate (std::function<void (std::string)> callback) const
{
    auto op = [] (hid_t g_id, const char *name, const H5L_info_t *info, void *op_data) -> herr_t
    {
        auto op = static_cast<std::function<int (std::string)>*> (op_data);
        (*op) (name);
        return 0;
    };
    auto object = getObject();
    auto idx = hsize_t (0);
    H5Literate (object->id, H5_INDEX_NAME, H5_ITER_INC, &idx, op, &callback);
}

bool H5::Location::readBool (std::string name) const
{
    auto ds = getDataSet (name);
    auto buffer = ds.readAll();
    return buffer.getElement<int>(0);
}

int H5::Location::readInt (std::string name) const
{
    auto ds = getDataSet (name);
    auto buffer = ds.readAll();
    return buffer.getElement<int>(0);
}

double H5::Location::readDouble (std::string name) const
{
    auto ds = getDataSet (name);
    auto buffer = ds.readAll();
    return buffer.getElement<double>(0);
}

std::string H5::Location::readString (std::string name) const
{
    auto ds = getDataSet (name);
    auto buffer = ds.readAll();
    return buffer.toString();
}

Variant H5::Location::readVariant (std::string name) const
{
    // This approach is a bit wasteful because many temporary objects are
    // create. However, it allows the DataType class to have say over the
    // mapping between C and HDF5 types; if boolean was changed to some other
    // HDF5 type, this would not break.
    auto dsNativeType = getDataSet (name).getType();

    if (dsNativeType == DataType::boolean()) return readBool (name);
    if (dsNativeType == DataType::nativeDouble()) return readDouble (name);
    if (dsNativeType == DataType::nativeInt()) return readInt (name);
    if (H5Tget_class (dsNativeType.object->id) == H5T_STRING) return readString (name);
    throw std::runtime_error ("data set " + name + " cannot be read as a variant");
}

Variant::NamedValues H5::Location::readNamedValues() const
{
    auto values = Variant::NamedValues();

    iterate ([&] (std::string name)
    {
        values[name] = readVariant (name);
    });
    return values;
}

Array H5::Location::readArray (std::string name) const
{
    auto ds = getDataSet (name);
    auto space = ds.getSpace();
    auto shape = Array::shapeFromVector (space.getShape());
    auto array = Array (shape);
    ds.readBuffer (space, space, array.getAllocation());
    return array;
}

std::vector<int> H5::Location::readVectorInt (std::string name)
{
    auto heap = getDataSet (name).readAll();
    auto size = heap.size() / sizeof (int);
    auto vect = std::vector<int> (size);

    for (unsigned int n = 0; n < size; ++n)
    {
        vect[n] = heap.getElement<int>(n);
    }
    return vect;
}

std::vector<double> H5::Location::readVectorDouble (std::string name)
{
    auto heap = getDataSet (name).readAll();
    auto size = heap.size() / sizeof (double);
    auto vect = std::vector<double> (size);

    for (unsigned int n = 0; n < size; ++n)
    {
        vect[n] = heap.getElement<double>(n);
    }
    return vect;
}

Array H5::Location::readArrays (std::vector<std::string> names, int stackedAxis,
    Cow::Region sourceRegion) const
{
    assert (! names.empty());
    assert (0 <= stackedAxis && stackedAxis < 5);
    auto shapeVector = getDataSet (names[0]).getSpace().getShape();
    auto sourceShape = Array::shapeFromVector (shapeVector);

    if (sourceRegion == Region())
    {
        sourceRegion = Region::whole (sourceShape);
    }
    if (sourceShape[stackedAxis] != 1)
    {
        throw std::runtime_error ("cannot stack data along axis with size != 1");
    }

    auto targetShape = sourceRegion.shape();
    targetShape[stackedAxis] = names.size();

    auto A = Array (targetShape);
    auto targetRegion = Region();

    for (unsigned int n = 0; n < names.size(); ++n)
    {
        targetRegion.lower[stackedAxis] = n;
        targetRegion.upper[stackedAxis] = n + 1;
        A[targetRegion] = getDataSet (names[n])[sourceRegion].value();
    }
    return A;
}

H5::DataSet H5::Location::writeBool (std::string name, bool value)
{
    auto ds = createDataSet (name, H5::DataType::boolean());
    auto buffer = HeapAllocation (sizeof (unsigned char));
    buffer.getElement<unsigned char>(0) = value;
    ds.writeAll (buffer);
    return ds;
}

H5::DataSet H5::Location::writeInt (std::string name, int value)
{
    auto ds = createDataSet (name, H5::DataType::nativeInt());
    auto buffer = HeapAllocation (sizeof (int));
    buffer.getElement<int>(0) = value;
    ds.writeAll (buffer);
    return ds;
}

H5::DataSet H5::Location::writeDouble (std::string name, double value)
{
    auto ds = createDataSet (name, H5::DataType::nativeDouble());
    auto buffer = HeapAllocation (sizeof (double));
    buffer.getElement<double>(0) = value;
    ds.writeAll (buffer);
    return ds;
}

H5::DataSet H5::Location::writeString (std::string name, std::string value)
{
    if (value.empty())
    {
        value = "<NULL>";
    }
    auto ds = createDataSet (name, H5::DataType::nativeString (value.size()));
    auto buffer = HeapAllocation (value);
    ds.writeAll (buffer);
    return ds;
}

H5::DataSet H5::Location::writeVariant (std::string name, Variant value)
{
    switch (value.getType())
    {
        case 'n': throw std::runtime_error ("cannot write null variant");
        case 'b': return writeBool (name, value);
        case 'i': return writeInt (name, value);
        case 'd': return writeDouble (name, value);
        case 's': return writeString (name, value);
        default: assert (false);
    }
}

H5::DataSet H5::Location::writeArray (std::string name, const Array& A)
{
    auto ds = createDataSet (name, A.getShapeVector());
    ds.writeAll (A.getAllocation());
    return ds;    
}

H5::DataSet H5::Location::writeArray (std::string name, const Array::Reference reference)
{
    auto ds = createDataSet (name, reference.getRegion().getShapeVector());
    ds[Region()] = reference;
    return ds;
}

H5::DataSet H5::Location::writeVectorInt (std::string name, const std::vector<int>& value)
{
    auto heap = HeapAllocation (value.size() * sizeof (int));
    auto dset = createDataSet (name, std::vector<int> (1, value.size()), DataType::nativeInt());

    for (unsigned int n = 0; n < value.size(); ++n)
    {
        heap.getElement<int>(n) = value[n];
    }
    dset.writeAll (heap);
    return dset;
}

H5::DataSet H5::Location::writeVectorDouble (std::string name, const std::vector<double>& value)
{
    auto heap = HeapAllocation (value.size() * sizeof (double));
    auto dset = createDataSet (name, std::vector<int> (1, value.size()), DataType::nativeDouble());

    for (unsigned int n = 0; n < value.size(); ++n)
    {
        heap.getElement<double>(n) = value[n];
    }
    dset.writeAll (heap);
    return dset;
}




// ============================================================================
H5::DataSet::DataSet (Object* object) : object (object)
{

}
void H5::DataSet::readBuffer (DataSpace memory, DataSpace file, HeapAllocation& buffer) const
{
    DataType type = getType();
    H5Dread (
        object->id,
        type.object->id,
        memory.object->id,
        file.object->id,
        H5P_DEFAULT,
        buffer.begin());   
}

HeapAllocation H5::DataSet::readAll() const
{
    auto space = getSpace();
    auto buffer = HeapAllocation (space.size() * getType().bytes());
    readBuffer (space, space, buffer);
    return buffer;
}

void H5::DataSet::writeBuffer (DataSpace memory, DataSpace file, const HeapAllocation& buffer) const
{
    DataType type = getType();
    H5Dwrite (
        object->id,
        type.object->id,
        memory.object->id,
        file.object->id,
        H5P_DEFAULT,
        buffer.begin());
}

void H5::DataSet::writeAll (const HeapAllocation& buffer) const
{
    auto space = getSpace();
    assert (buffer.size() == space.size() * getType().bytes());
    writeBuffer (space, space, buffer);
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

Array H5::DataSet::Reference::value() const
{
    auto targetArray = Array (R.shape());
    auto memory = DataSpace (targetArray.getShapeVector());
    auto file = D.getSpace();
    file.select (R);
    D.readBuffer (memory, file, targetArray.getAllocation());
    return targetArray;
}

const Array& H5::DataSet::Reference::operator= (Array& A)
{
    this->operator= (A[Region()]);
    return A;
}

const Array::Reference& H5::DataSet::Reference::operator= (const Array::Reference& ref)
{
    auto memory = DataSpace (ref.getArray().getShapeVector());
    auto file = D.getSpace();
    file.select (R);
    memory.select (ref.getRegion());
    D.writeBuffer (memory, file, ref.getArray().getAllocation());
    return ref;
}




// ============================================================================
H5::DataSpace::DataSpace()
{
    hid_t id = H5Screate (H5S_SCALAR);
    object.reset (new Object (id, 'S'));
}

H5::DataSpace::DataSpace (std::vector<int> shape)
{
    std::vector<hsize_t> current_dims;
    std::vector<hsize_t> maximum_dims;

    for (auto n : shape)
    {
        current_dims.push_back (n);
        maximum_dims.push_back (n);
    }
    hid_t id = H5Screate_simple (shape.size(), &current_dims[0], &maximum_dims[0]);
    object.reset (new Object (id, 'S'));
}

H5::DataSpace::DataSpace (Object* object) : object (object)
{

}

std::vector<int> H5::DataSpace::getShape() const
{
    int ndims = H5Sget_simple_extent_ndims (object->id);

    if (ndims == -1) // An HDF5 error occured. Don't pollute the error stack.
    {
        return std::vector<int>();
    }

    std::vector<hsize_t> hdims (ndims);
    H5Sget_simple_extent_dims (object->id, &hdims[0], nullptr);
    std::vector<int> dims;

    for (auto n : hdims)
    {
        dims.push_back (n);
    }
    return dims;
}

int H5::DataSpace::size() const
{
    int S = 1;

    for (auto N : getShape())
    {
        S *= N;
    }
    return S;
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
H5::DataType H5::DataType::boolean()
{
    hid_t id = H5Tcopy (H5T_NATIVE_UCHAR);
    return new Object (id, 'T');
}

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

std::size_t H5::DataType::bytes() const
{
return H5Tget_size (object->id);
}

H5::DataType H5::DataType::native() const
{
    return new Object (H5Tget_native_type (object->id, H5T_DIR_ASCEND), 'T');
}

bool H5::DataType::operator== (const DataType& other) const
{
    return H5Tequal (object->id, other.object->id);
}





// ============================================================================
H5::File::File (std::string name, const char* mode)
{
    if (std::strcmp (mode, "r") == 0)
    {
        hid_t id = H5Fopen (name.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
        object.reset (new Object (id, 'F'));
        return;        
    }
    if (std::strcmp (mode, "a") == 0)
    {
        hid_t id = H5Fopen (name.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
        object.reset (new Object (id, 'F'));
        return;        
    }
    if (std::strcmp (mode, "w") == 0)
    {
        hid_t id = H5Fcreate (name.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        object.reset (new Object (id, 'F'));
        return;
    }
    assert (false);
};

int H5::File::getObjectCount() const
{
    return H5Fget_obj_count (object->id, H5F_OBJ_ALL);
}




// ============================================================================
H5::Group::Group (Object* object) : object (object)
{

}
