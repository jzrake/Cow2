#ifndef CowArray_hpp
#define CowArray_hpp

#include <cstdlib>
#include <array>
#include <vector>




namespace Cow
{
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
        std::size_t size();

        template <class T> T& getElement (std::size_t index)
        {
            return static_cast<T*>(allocation)[index];
        }

        template <class T> const T& getElement (std::size_t index) const
        {
            return static_cast<T*>(allocation)[index];
        }

    private:
        void* allocation;
        std::size_t numberOfBytes;
    };

    using Shape = std::array<int, 5>;

    class Range
    {
    public:
        Range (int index);
        Range (int lower, int upper);
        Range (const char*);
        int absoluteLower (int size);
        int absoluteUpper (int size);
        int absoluteLength (int size);
    private:
        const int lower;
        const int upper;
    };

    class Array
    {
    public:
        Array();
        Array (int n1);
        Array (int n1, int n2);
        Array (int n1, int n2, int n3);
        Array (int n1, int n2, int n3, int n4);
        Array (int n1, int n2, int n3, int n4, int n5);

        /**
        Copy ocnstructor.
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
        Return the shape of this array as a vector, whose length is the number
        of axes that have size greater than 1.
        */
        std::vector<int> getShapeVector() const;

        /**
        Return a new array that is the transpose of this one,

        A.transpose() (i, j, k, m, n) == A (n, m, k, j, i).

        The returned array has same memory layout type as this.
        */
        Array transpose() const;

        /** Retrieve a value by linear index */
        double& operator[] (int index);

        /** Retrieve a const value by linear index */
        const double& operator[] (int index) const;

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

        Array extract (Range is) const;
        Array extract (Range is, Range js) const;
        Array extract (Range is, Range js, Range ks) const;
        Array extract (Range is, Range js, Range ks, Range ms) const;
        Array extract (Range is, Range js, Range ks, Range ms, Range ns) const;

        void insert (const Array& A, Range is);
        void insert (const Array& A, Range is, Range js);
        void insert (const Array& A, Range is, Range js, Range ks);
        void insert (const Array& A, Range is, Range js, Range ks, Range ms);
        void insert (const Array& A, Range is, Range js, Range ks, Range ms, Range ns);

    private:
        /** @internal */
        static void copyRange (Array& dst, const Array& src,
            Range is, Range js, Range ks, Range ms, Range ns,
            char mode);

        char ordering;
        int n1, n2, n3, n4, n5;
        HeapAllocation memory;
    };
};

#endif