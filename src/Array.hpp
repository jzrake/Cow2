#ifndef CowArray_hpp
#define CowArray_hpp

#include <cstdlib>
#include <array>
#include <vector>




namespace Cow
{
    class Array;
    class HeapAllocation;
    class RegionIterator;


    /**
    A class to manage large allocations on the heap using RAII standard.
    */
    class HeapAllocation
    {
    public:

        /**
        Create a null allocation.
        */
        HeapAllocation();

        /**
        Allocate a heap block of the given size. Bytes are not zero-initialized.
        */
        HeapAllocation (std::size_t numberOfBytes);

        /**
        Create a heap allocation from a std::string.
        */
        HeapAllocation (std::string content);

        /**
        Construct this memory block from a deep copy of another one.
        */
        HeapAllocation (const HeapAllocation& other);

        /**
        Move constructor.
        */
        HeapAllocation (HeapAllocation&& other);

        /**
        Destructor.
        */
        ~HeapAllocation();

        /**
        Assign this block the contents of another (deep copy).
        */
        HeapAllocation& operator= (const HeapAllocation& other);

        /**
        Return the number of bytes in use.
        */
        std::size_t size() const;

        /**
        Return the contents of the buffer as a string.
        */
        std::string toString() const;

        void* begin() { return allocation; }

        const void* begin() const { return allocation; }

        template <class T> T& getElement (std::size_t index)
        {
            return static_cast<T*>(allocation)[index];
        }

        template <class T> const T& getElement (std::size_t index) const
        {
            return static_cast<T*>(allocation)[index];
        }

        template <class T> T* begin()
        {
            return static_cast<T*>(allocation);
        }

        template <class T> T* end()
        {
            return static_cast<T*>(allocation) + numberOfBytes / sizeof(T);
        }

    private:
        void* allocation;
        std::size_t numberOfBytes;
    };


    /**
    A type to represent the shape of an array or region.
    */
    using Shape = std::array<int, 5>;


    /**
    A type to represent a multi-dimensional index (i, j, k, m, n).
    */
    using Index = std::array<int, 5>;


    /**
    A class that represents a relative range along a single array axis.
    */
    class Range
    {
    public:
        /**
        Construct a relative or absolute range [i0:i1:di]. The default [0:0:1]
        is a relative range that covers the whole extent.
        */
        Range (int lower, int upper, int stride=1);

        /**
        Construct a relative range from the character ":" as a shortcut for
        the whole extent.
        */
        Range (const char*);

        /** Return true if the upper bound is relative to end. */
        bool isRelative() const;

        /**
        Get the size of a relative range for the given axis size, after
        strides are accounted for. The range is assumed to be absolute.
        */
        int size () const;

        /**
        Get the absolute size of a relative range for the given axis size,
        after strides are accounted for.
        */
        int size (int absoluteSize) const;

        /** Return an aboslute version of this range. */
        Range absolute (int absoluteSize) const;

        const int lower;
        const int upper;
        const int stride;
    };


    /**
    A class that represents a relative or absolute region within an array. If
    any component of upper is either zero or negative, then that value
    indicates a distance from the end of the array, and the region is called
    'relative'. An 'absolute' region is generated by providing a shape object
    to the absolute() method. By default each axis covers the range [0:0:1],
    which denotes the entire extent of the axis (note there is no empty
    range).
    */
    class Region
    {
    public:
        Region();

        /** Return true if the upper bound is relative to end. */
        bool isRelative() const;

        /**
        Return the number of elements along each axis, after strides are
        accounted for. The region is assumed to be absolute.
        */
        Shape shape() const;

        /**
        Return shape(), but with trailing axes of length 1 removed.
        */
        std::vector<int> getShapeVector() const;

        /**
        Return an absolute version of this region by providing a definite shape.
        */
        Region absolute (Shape shape) const;

        /**
        Return an absolute version of this region by providing a shape vector.
        Trailing axes are assumed to have size equal to 1.
        */
        Region absolute (std::vector<int> shapeVector) const;

        /**
        Return the absolute range of indices (with stride information) covered
        for the given axis.
        */
        Range range (int axis) const;

        Index lower;
        Index upper;
        Index stride;
    };


    /**
    A multidimensional array class, hard-coded to accommodate up to 5 axes.
    */
    class Array
    {
    public:
        class Reference;
        class Iterator;

        Array();
        Array (Shape shape);
        Array (Reference reference);
        Array (int n1);
        Array (int n1, int n2);
        Array (int n1, int n2, int n3);
        Array (int n1, int n2, int n3, int n4);
        Array (int n1, int n2, int n3, int n4, int n5);

        /**
        Copy constructor.
        */
        Array (const Array& other);

        /**
        Move constructor.
        */
        Array (Array&& other);

        /**
        Assignment operator.
        */
        Array& operator= (const Array& other);

        /** Get a reference to the underlying buffer. */
        const HeapAllocation& getAllocation() const { return memory; }

        /**
        Set the memory layout to either 'C' or 'F' (C or Fortran) type
        ordering. With 'C', the last index is contiguous in memory, while with
        'F' it is the first index that is contiguous. This function does not
        move any data in the internal buffer, so if the array is already
        populated, then its contents will appear transposed in any subsequent
        indexing.
        */
        void setOrdering (char orderingMode);

        /**
        Return the total number of doubles in this array.
        */
        int size() const;

        /**
        Return the Array's shape as a 5-component array.
        */
        Shape shape() const;

        /**
        Return the memory layout type, 'C' or 'F'.
        */
        char getOrdering() const;

        /**
        Return the shape of this array as a vector, with trailing axes that
        have length 1 removed.
        */
        std::vector<int> getShapeVector() const;

        /**
        Return a new array that is the transpose of this one, A.transpose()
        (i, j, k, m, n) == A (n, m, k, j, i). The returned array has same
        memory layout type as this.
        */
        Array transpose() const;

        /**
        Extract a deep copy of the given relative or absolute region of this
        array. This is equivalent to auto B = Array (A[region]);
        */
        Array extract (Region R) const;

        /**
        Insert all of the given array into the given region of this array.
        */
        void insert (const Array& A, Region R);

        /**
        Return a trivial iterator to the beginning of the array.
        */
        double* begin() { return memory.begin<double>(); }

        /**
        Return a trivial iterator to the end of the array.
        */
        double* end() { return memory.end<double>(); }

        /** Retrieve a value by linear index. */
        double& operator[] (int index);

        /** Retrieve a const value by linear index. */
        const double& operator[] (int index) const;

        /**
        Return a reference to a particular region in this array. It is the
        caller's responsibility to ensure the referenced array remains alive
        at least as long as the reference does.
        */
        Reference operator[] (Region R);

        double& operator() (int i);
        double& operator() (int i, int j);
        double& operator() (int i, int j, int k);
        double& operator() (int i, int j, int k, int m);
        double& operator() (int i, int j, int k, int m, int n);

        const double& operator() (int i) const;
        const double& operator() (int i, int j) const;
        const double& operator() (int i, int j, int k) const;
        const double& operator() (int i, int j, int k, int m) const;
        const double& operator() (int i, int j, int k, int m, int n) const;

        class Reference
        {
        public:
            /**
            Constructor. The region is assumed to be absolute.
            */
            Reference (Array& A, Region R);

            /**
            Return the referenced array.
            */
            const Array& getArray() const;

            /**
            Return the region of the referenced array.
            */
            const Region& getRegion() const;

            /**
            Return an iterator to the beginning of the array. When
            incremented, the result is equivalent to a multidimensional loop
            over the referenced rgion.
            */
            Iterator begin();

            /**
            End iterator.
            */
            Iterator end();
        private:
            friend class Array;
            Array& A;
            Region R;
        };

        class Iterator
        {
        public:
            Iterator (Array& A, Region R, bool isEnd=false);
            operator double*() const;
            double* operator++ ();
            bool operator== (const Iterator& other) const;
            void print (std::ostream& stream) const;
        private:
            double* getAddress() const;
            Array& A;
            Region R;
            Index currentIndex;
            double* currentAddress;
        };

    private:
        /** @internal */
        static void copyRegion (Array& dst, const Array& src, Region R, char mode);

        char ordering;
        int n1, n2, n3, n4, n5;
        HeapAllocation memory;
    };
};

#endif