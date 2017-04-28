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
            Create a scalar data set below this location.
            */
            DataSet createDataSet (std::string name, const DataType& type);

            /**
            Create a data set below this location with the given name and shape.
            */
            DataSet createDataSet (std::string name, std::vector<int> shape);
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
            Write an array into the data space. The shape of A must match the
            data space dimensions. If A has Fortran-style internal data
            layout, then this function writes from a buffer that is a
            transposed copy of A, becuase HDF5 assumes 'C' style data layout.
            */
            void write (const Cow::Array& A);

            /**
            Write a string. Assumes the space is DataSpace::scalar() and the
            type is DataType::nativeString().
            */
            void write (std::string S);

            /**
            Get a copy of this data set's data space.
            */
            DataSpace getSpace();

            /**
            Get a copy of this data set's data type.
            */
            DataType getType();

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
            static DataSpace scalar();

            /**
            Construct a DataSpace of the given shape, that is not extensible.
            */
            static DataSpace simple (std::vector<int> shape);

            /**
            Get the data space's shape.
            */
            std::vector<int> getShape();
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
