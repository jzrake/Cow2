#ifndef HDF5_hpp
#define HDF5_hpp

#include <memory>
#include <string>
#include <vector>
#include "Array.hpp"
#include "Variant.hpp"




namespace Cow
{
    namespace H5
    {
        // ====================================================================
        // Forward declarations
        // ====================================================================


        class Attribute;
        class DataSet;
        class DataSpace;
        class DataType;
        class File;
        class Group;
        class Object;




        // ====================================================================
        // Base classes for shared functionality
        // ====================================================================


        /**
        A base class for HDF5 objects that implement virtual methods.
        */
        class ObjectProvider
        {
        public:
            /**
            This is required because of multiple inheritance.
            */
            ObjectProvider& operator= (const ObjectProvider&) = default;

            /**
            Derived classes must implement this method.
            */
            virtual const Object* getObject() const = 0;
        };


        /**
        A base class for locations that can create groups (File and Group).
        */
        class GroupCreator : public virtual ObjectProvider
        {
        public:
            /**
            Return true if the object has a group with the given name.
            */
            bool hasGroup (std::string name) const;

            /**
            Return true if this location contains all the given groups.
            */
            bool hasGroups (std::vector<std::string> names) const;

            /**
            Get a group below this location with the given name. The group
            must already exist.
            */
            Group getGroup (std::string name) const;

            /**
            Create a group below this location with the given name. The group
            must not already exist.
            */
            Group createGroup (std::string name);
        };


        /**
        A base class for locations that can create data sets (File and Group).
        */
        class DataSetCreator : public virtual ObjectProvider
        {
        public:
            /**
            Return true if the object has a data set with the given name.
            */
            bool hasDataSet (std::string name) const;

            /**
            Return true if this location contains all the given data sets.
            */
            bool hasDataSets (std::vector<std::string> names) const;

            /**
            Get a data set at this location with the given name. The data set
            must already exist.
            */
            DataSet getDataSet (std::string name) const;

            /**
            Create a scalar data set at this location with the given type.
            */
            DataSet createDataSet (std::string name, const DataType& type);

            /**
            Create an array data set at this location with the given shape,
            and type of double.
            */
            DataSet createDataSet (std::string name, std::vector<int> shape);

            /**
            Create an array data set at this location with the given shape and
            type.
            */
            DataSet createDataSet (std::string name, std::vector<int> shape, const DataType& type);

            /**
            Iterate over all HDF5 locations below this one, invoking the given
            callback with the name of the location.
            */
            void iterate (std::function<void (std::string)> callback);

            /**
            Read several data sets and return an array with those data sets
            stacked along the given axis. Each of the source arrays must have
            the same size, and have size 1 along the stacked axis.
            */
            Array readArrays (std::vector<std::string> names, int stackedAxis, Cow::Region sourceRegion=Region()) const;

            bool readBool (std::string name) const;
            int readInt (std::string name) const;
            double readDouble (std::string name) const;
            std::string readString (std::string name) const;
            Variant readVariant (std::string name) const; // DOWN
            Array readArray (std::string name) const;
            std::vector<int> readVectorInt (std::string name);
            std::vector<double> readVectorDouble (std::string name);

            DataSet writeBool (std::string name, bool value);
            DataSet writeInt (std::string name, int value);
            DataSet writeDouble (std::string name, double value);
            DataSet writeString (std::string name, std::string value);
            DataSet writeVariant (std::string name, Variant value);
            DataSet writeArray (std::string name, const Array& A);
            DataSet writeArray (std::string name, const Array::Reference reference);
            DataSet writeVectorInt (std::string name, const std::vector<int>& value);
            DataSet writeVectorDouble (std::string name, const std::vector<double>& value);
        };


        /**
        A base class for locations that can create attributes (not yet
        implemented).
        */
        class AttributeCreator : public virtual ObjectProvider
        {
        public:
            /**
            Create a data set below this location with the given name and
            shape.
            */
            Attribute createAttribute (std::string name, const DataType& type);
        };




        // ====================================================================
        // Classes for concrete HDF5 objects
        // ====================================================================


        /**
        A class representing an HDF5 attribute.
        */
        class Attribute
        {
        private:
            friend class AttributeCreator;
            Attribute (Object* object);
            std::shared_ptr<Object> object;
        };


        /**
        A class representing an HDF5 data set.
        */
        class DataSet
        {
        public:

            /**
            A class that refers to a selection within a data set.
            */
            class Reference
            {
            public:
                /**
                Create a reference into a subset of this data set. The given
                region is assumed to be absolute.
                */
                Reference (DataSet& D, Region R);
                Array value() const;
                const Array& operator= (Array& A);
                const Array::Reference& operator= (const Array::Reference& ref);
            private:
                DataSet& D;
                Region R;
            };

            /**
            General read function.
            */
            void readBuffer (DataSpace memory, DataSpace file, HeapAllocation& buffer) const;

            /**
            Read all of the data set and return it as a new heap allocation.
            */
            HeapAllocation readAll() const;

            /**
            General write function.
            */
            void writeBuffer (DataSpace memory, DataSpace file, const HeapAllocation& buffer) const;

            /**
            Write a buffer into the whole data space. The buffer size must
            match the size of the data space.
            */
            void writeAll (const HeapAllocation& buffer) const;

            /**
            Get a copy of this data set's data space.
            */
            DataSpace getSpace() const;

            /**
            Get a copy of this data set's data type.
            */
            DataType getType() const;

            /**
            Return a reference to a subset of this data set. It is the
            caller's responsibility to ensure this data set lives at least as
            long as the reference. This function does not make sense for
            scalar data sets.
            */
            Reference operator[] (Cow::Region region);

        private:
            friend class DataSetCreator;
            DataSet (Object* object);
            std::shared_ptr<Object> object;
        };


        /**
        A class representing an HDF5 data space.
        */
        class DataSpace
        {
        public:
            /**
            Construct a scalar DataSpace.
            */
            DataSpace();

            /**
            Construct a DataSpace of the given shape, that is not extensible.
            */
            DataSpace (std::vector<int> shape);

            /**
            Get the data space's shape.
            */
            std::vector<int> getShape() const;

            /**
            Return the total number of elements in the data space.
            */
            int size() const;

            /**
            Sets the active selection to correspond to the given region, which
            may be relative or absolute. This utilizes the HDF5 hyperslab
            functions.
            */
            void select (Region R);

        private:
            friend class DataSet;
            friend class DataSetCreator;
            DataSpace (Object* object);
            std::shared_ptr<Object> object;
        };


        /**
        A class representing an HDF5 data type.
        */
        class DataType
        {
        public:
            static DataType boolean();
            static DataType nativeInt();
            static DataType nativeDouble();
            static DataType nativeString (int length);

            /** Return the size in bytes of this data type. */
            std::size_t bytes() const;

            /**
            Return the clostest matching native data type through H5Tget_native.
            */
            DataType native() const;

            /**
            Equality overload: calls the H5Tequal function.
            */
            bool operator== (const DataType& other) const;

        private:
            friend class DataSet;
            friend class DataSetCreator;
            DataType (Object* object);
            std::shared_ptr<Object> object;
        };


        /**
        A class representing an HDF5 file.
        */
        class File : public GroupCreator, public DataSetCreator
        {
        public:
            /**
            Create or open an HDF5 file. Mode may be "r", "a", or "w"
            corresponding to read, read/write, and truncate modes.
            */
            File (std::string name, const char* mode="r");

            /**
            Return the number of HDF5 objects that are open and attached to
            this file.
            */
            int getObjectCount() const;

        private:
            const Object* getObject() const override { return object.get(); }
            std::shared_ptr<Object> object;
        };


        /**
        A class representing an HDF5 group.
        */
        class Group : public GroupCreator, public DataSetCreator
        {
        private:
            friend class GroupCreator;
            Group (Object* object);
            const Object* getObject() const override { return object.get(); }
            std::shared_ptr<Object> object;
        };
    }
}

#endif
