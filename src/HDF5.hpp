#ifndef HDF5_hpp
#define HDF5_hpp

#include <memory>
#include <string>
#include <vector>
#include "Array.hpp"




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
            virtual Object* getObject() = 0;
        };


        /**
        A base class for locations that can create groups (File and Group).
        */
        class GroupCreator : public virtual ObjectProvider
        {
        public:
            /**
            Create a group below this location with the given name.
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
            Create a scalar data set at this location with the given type.
            */
            DataSet createDataSet (std::string name, const DataType& type);

            /**
            Create an array at this location with the given shape, and type of double.
            */
            DataSet createDataSet (std::string name, std::vector<int> shape);

            /** Write a string to a new data set. */
            DataSet write (std::string name, std::string value);

            /** Write a double to a new data set. */
            DataSet write (std::string name, double value);

            /** Write an integer to a new data set. */
            DataSet write (std::string name, int value);

            /** Write an array to a new data set. */
            DataSet write (std::string name, const Array& A);

            /** Write an array to a new data set. */
            DataSet write (std::string name, const Array::Reference reference);
        };


        /**
        A base class for locations that can create attributes.
        */
        class AttributeCreator : public virtual ObjectProvider
        {
        public:
            /**
            Create a data set below this location with the given name and shape.
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
                const Array& operator= (Array& A);
                const Array::Reference& operator= (const Array::Reference& ref);
            private:
                DataSet& D;
                Region R;
            };

            /**
            General write function.
            */
            void writeBuffer (DataSpace memory, DataSpace file, const HeapAllocation& buffer) const;

            /**
            Write a buffer into the whole data space. The buffer size must
            match the size of the data space.
            */
            void writeBuffer (const HeapAllocation& buffer) const;

            /** Get a copy of this data set's data space. */
            DataSpace getSpace() const;

            /** Get a copy of this data set's data type. */
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
            static DataType nativeInt();
            static DataType nativeDouble();
            static DataType nativeString (int length);

            /** Return the size in bytes of this data type. */
            std::size_t bytes() const;

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
            File (std::string name);

            /**
            Return the number of HDF5 objects that are open and attached to
            this file.
            */
            int getObjectCount();

        private:
            Object* getObject() override { return object.get(); }
            std::shared_ptr<Object> object;
        };


        /**
        A class representing an HDF5 group.
        */
        class Group : public GroupCreator, public DataSetCreator
        {
        public:
            /**
            Create an attribute for this group. The data space is scalar.
            */
            Attribute createAttribute (std::string name, const DataType& type);

        private:
            friend class GroupCreator;
            Group (Object* object);
            Object* getObject() override { return object.get(); }
            std::shared_ptr<Object> object;
        };
    }
}

#endif
