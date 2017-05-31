#include <iostream> // DEBUG
#include <stdexcept>
#include <cstring>
#include "Array.hpp"
#include "DebugHelper.hpp"

// #define COW_DISABLE_BOUNDS_CHECK

#define INDEXC(i, j, k, m, n) (n5 * (n4 * (n3 * (n2 * i + j) + k) + m) + n)
#define INDEXF(i, j, k, m, n) (n1 * (n2 * (n3 * (n4 * n + m) + k) + j) + i)
#define INDEX(i, j, k, m, n) (ordering == 'C' ? INDEXC(i, j, k, m, n) : INDEXF(i, j, k, m, n))
#define INDEX_ERROR(ii, nn) std::logic_error(#ii "=" + std::to_string (ii) + " not in bounds [0 " + std::to_string (nn) + "]")
#ifndef COW_DISABLE_BOUNDS_CHECK
#define BOUNDS_CHECK(i, j, k, m, n) do { \
if ( !(0 <= i && i < n1)) throw INDEX_ERROR(i, n1);\
if ( !(0 <= j && j < n2)) throw INDEX_ERROR(j, n2);\
if ( !(0 <= k && k < n3)) throw INDEX_ERROR(k, n3);\
if ( !(0 <= m && m < n4)) throw INDEX_ERROR(m, n4);\
if ( !(0 <= n && n < n5)) throw INDEX_ERROR(n, n5); } while (0)
#else
#define BOUNDS_CHECK(i, j, k, m, n) do { } while (0)
#endif

using namespace Cow;




// ============================================================================
std::ostream& operator<< (std::ostream &stream, const Cow::HeapAllocation &memory)
{
    stream.write (static_cast<const char*>(memory.begin()), memory.size());
    return stream;
}




// ============================================================================
HeapAllocation::HeapAllocation() : numberOfBytes (0)
{
    allocation = nullptr;
}

HeapAllocation::HeapAllocation (std::size_t numberOfBytes) : numberOfBytes (numberOfBytes)
{
    allocation = std::malloc (numberOfBytes);
}

HeapAllocation::HeapAllocation (const HeapAllocation& other) : numberOfBytes (other.numberOfBytes)
{
    allocation = std::malloc (other.numberOfBytes);
    std::memcpy (allocation, other.allocation, numberOfBytes);
}

HeapAllocation::HeapAllocation (std::string content) : numberOfBytes (content.size())
{
    allocation = std::malloc (numberOfBytes);
    std::memcpy (allocation, content.data(), numberOfBytes);
}

HeapAllocation::HeapAllocation (HeapAllocation&& other)
{
    allocation = other.allocation;
    numberOfBytes = other.numberOfBytes;
    other.allocation = nullptr;
    other.numberOfBytes = 0;
}

HeapAllocation::~HeapAllocation()
{
    std::free (allocation);
}

HeapAllocation& HeapAllocation::operator= (const HeapAllocation& other)
{
    if (&other != this)
    {
        numberOfBytes = other.numberOfBytes;
        allocation = std::realloc (allocation, numberOfBytes);
        std::memcpy (allocation, other.allocation, numberOfBytes);
    }
    return *this;
}

std::size_t HeapAllocation::size() const
{
    return numberOfBytes;
}

std::string HeapAllocation::toString() const
{
    auto message = std::string (numberOfBytes, 0);
    std::memcpy (&message[0], allocation, numberOfBytes);
    return message;
}

HeapAllocation HeapAllocation::swapBytes (std::size_t bytesPerEntry) const
{
    HeapAllocation M (numberOfBytes);
    const int numEntries = numberOfBytes / bytesPerEntry;

    for (int n = 0; n < numEntries; ++n)
    {
        const char* startSource = static_cast<const char*>(allocation) + n * bytesPerEntry;
        char* startTarget = static_cast<char*>(M.allocation) + n * bytesPerEntry;

        for (int b = 0; b < bytesPerEntry; ++b)
        {
            startTarget[bytesPerEntry - b - 1] = startSource[b];
        }
    }
    return M;
}




// ============================================================================
Range::Range (int lower, int upper, int stride) : lower (lower), upper (upper), stride (stride)
{

}

Range::Range (const char* colon) : lower (0), upper (0), stride (1)
{
    assert (std::strcmp (colon, ":") == 0);
}

bool Range::isRelative() const
{
    return upper <= 0 || lower < 0;
}

int Range::size () const
{
    assert (! isRelative());
    return (upper - lower) / stride;    
}

int Range::size (int absoluteSize) const
{
    return absolute (absoluteSize).size();
}

Range Range::absolute (int absoluteSize) const
{
    const int L = lower <  0 ? lower + absoluteSize : lower;
    const int U = upper <= 0 ? upper + absoluteSize : upper;
    return Range (L, U, stride);
}




// ============================================================================
Region Region::empty()
{
    Region R;

    for (int n = 0; n < 5; ++n)
    {
        R.lower[n] = 0;
        R.upper[n] = 0;
        R.stride[n] = 0;
    }
    return R;
}

Region Region::whole (Shape shape)
{
    return Region().absolute (shape);
}

Region::Region()
{
    for (int n = 0; n < 5; ++n)
    {
        lower[n] = 0;
        upper[n] = 0;
        stride[n] = 1;
    }
}

bool Region::isRelative() const
{
    for (int n = 0; n < 5; ++n)
    {
        if (upper[n] <= 0) return true;
    }
    return false;
}

bool Region::isEmpty() const
{
    for (int n = 0; n < 5; ++n)
    {
        if (upper[n] != 0 || lower[n] != 0 || stride[n] != 0) return false;
    }
    return true;
}

Shape Region::shape() const
{
    return {{
        range (0).size(),
        range (1).size(),
        range (2).size(),
        range (3).size(),
        range (4).size() }};
}

std::vector<int> Region::getShapeVector() const
{
    return Array::vectorFromShape (shape());
}

Region Region::absolute (Shape shape) const
{
    Region R = *this;

    for (int n = 0; n < 5; ++n)
    {
        if (R.lower[n] <  0) R.lower[n] += shape[n];
        if (R.upper[n] <= 0) R.upper[n] += shape[n];
    }
    return R;
}

Region Region::absolute (std::vector<int> shapeVector) const
{
    Region R = *this;

    for (int n = 0; n < 5; ++n)
    {
        if (n < shapeVector.size())
        {
            if (R.lower[n] <  0) R.lower[n] += shapeVector[n];
            if (R.upper[n] <= 0) R.upper[n] += shapeVector[n];
        }
        else
        {
            R.lower[n] = 0;
            R.upper[n] = 1;
        }
    }
    return R;
}

Range Region::range (int axis) const
{
    return Range (lower[axis], upper[axis], stride[axis]);
}




// ============================================================================
Array::Array() : Array (0, 1, 1, 1, 1)
{

}

Array::Array (Shape shape) : Array (shape[0], shape[1], shape[2], shape[3], shape[4])
{

}

Array::Array (Reference reference)
{
    *this = reference.A.extract (reference.R);
}

Array::Array (int n1) : Array (n1, 1, 1, 1, 1)
{

}

Array::Array (int n1, int n2) : Array (n1, n2, 1, 1, 1)
{

}

Array::Array (int n1, int n2, int n3) : Array (n1, n2, n3, 1, 1)
{

}

Array::Array (int n1, int n2, int n3, int n4) : Array (n1, n2, n3, n4, 1)
{

}

Array::Array (int n1, int n2, int n3, int n4, int n5) :
ordering ('C'),
n1 (n1),
n2 (n2),
n3 (n3),
n4 (n4),
n5 (n5),
memory (n1 * n2 * n3 * n4 * n5 * sizeof (double))
{
    for (int i = 0; i < size(); ++i)
    {
        memory.getElement<double> (i) = 0.0;
    }
}

Array::Array (const Array& other)
{
    memory = other.memory;
    ordering = other.ordering;
    n1 = other.n1;
    n2 = other.n2;
    n3 = other.n3;
    n4 = other.n4;
    n5 = other.n5;
}

Array::Array (Array&& other)
{
    memory = std::move (other.memory);
    ordering = other.ordering;
    n1 = other.n1;
    n2 = other.n2;
    n3 = other.n3;
    n4 = other.n4;
    n5 = other.n5;
    other = Array();
}

Array& Array::operator= (const Array& other)
{
    if (&other != this)
    {
        memory = other.memory;
        ordering = other.ordering;
        n1 = other.n1;
        n2 = other.n2;
        n3 = other.n3;
        n4 = other.n4;
        n5 = other.n5;
    }
    return *this;
}

void Array::setOrdering (char orderingMode)
{
    assert (orderingMode == 'C' || orderingMode == 'F');
    ordering = orderingMode;
}

int Array::size() const
{
    return n1 * n2 * n3 * n4 * n5;
}

int Array::size (int axis) const
{
    switch (axis)
    {
        case 0: return n1;
        case 1: return n2;
        case 2: return n3;
        case 3: return n4;
        case 4: return n5;
        default: assert (false); return 0;
    }
}

Shape Array::shape() const
{
    return {{n1, n2, n3, n4, n5}};
}

char Array::getOrdering() const
{
    return ordering;
}

std::vector<int> Array::getShapeVector() const
{
    return vectorFromShape (shape());
}

Array Array::transpose() const
{
    Array A (n5, n4, n3, n2, n1);
    A.ordering = ordering;

    for (int i = 0; i < n1; ++i)
    for (int j = 0; j < n2; ++j)
    for (int k = 0; k < n3; ++k)
    for (int m = 0; m < n4; ++m)
    for (int n = 0; n < n5; ++n)
    {
        A (n, m, k, j, i) = this->operator() (i, j, k, m, n);
    }
    return A;
}

Array Array::transpose (int axis1, int axis2) const
{
    Shape sourceShape = shape();
    Shape targetShape = shape();
    targetShape[axis1] = sourceShape[axis2];
    targetShape[axis2] = sourceShape[axis1];

    Array A (targetShape);
    A.ordering = ordering;

    for (int i = 0; i < n1; ++i)
    for (int j = 0; j < n2; ++j)
    for (int k = 0; k < n3; ++k)
    for (int m = 0; m < n4; ++m)
    for (int n = 0; n < n5; ++n)
    {
        Index sourceIndex {{i, j, k, m, n}};
        Index targetIndex {{i, j, k, m, n}};
        targetIndex[axis1] = sourceIndex[axis2];
        targetIndex[axis2] = sourceIndex[axis1];

        const int it = targetIndex[0];
        const int jt = targetIndex[1];
        const int kt = targetIndex[2];
        const int mt = targetIndex[3];
        const int nt = targetIndex[4];

        A (it, jt, kt, mt, nt) = this->operator() (i, j, k, m, n);
    }
    return A;
}

double& Array::operator[] (int index)
{
    assert (0 <= index && index < size());
    return memory.getElement<double> (index);
}

const double& Array::operator[] (int index) const
{
    assert (0 <= index && index < size());
    return memory.getElement<double> (index);
}

Array::Reference Array::operator[] (Region R)
{
    return Reference (*this, R.absolute (shape()));
}

double& Array::operator() (int i)
{
    BOUNDS_CHECK(i, 0, 0, 0, 0);
    return memory.getElement<double> (INDEX(i, 0, 0, 0, 0));
}

double& Array::operator() (int i, int j)
{
    BOUNDS_CHECK(i, j, 0, 0, 0);
    return memory.getElement<double> (INDEX(i, j, 0, 0, 0));
}

double& Array::operator() (int i, int j, int k)
{
    BOUNDS_CHECK(i, j, k, 0, 0);
    return memory.getElement<double> (INDEX(i, j, k, 0, 0));
}

double& Array::operator() (int i, int j, int k, int m)
{
    BOUNDS_CHECK(i, j, k, m, 0);
    return memory.getElement<double> (INDEX(i, j, k, m, 0));
}

double& Array::operator() (int i, int j, int k, int m, int n)
{
    BOUNDS_CHECK(i, j, k, m, n);
    return memory.getElement<double> (INDEX(i, j, k, m, n));
}

const double& Array::operator() (int i) const
{
    BOUNDS_CHECK(i, 0, 0, 0, 0);
    return memory.getElement<double> (INDEX(i, 0, 0, 0, 0));
}

const double& Array::operator() (int i, int j) const
{
    BOUNDS_CHECK(i, j, 0, 0, 0);
    return memory.getElement<double> (INDEX(i, j, 0, 0, 0));
}

const double& Array::operator() (int i, int j, int k) const
{
    BOUNDS_CHECK(i, j, k, 0, 0);
    return memory.getElement<double> (INDEX(i, j, k, 0, 0));
}

const double& Array::operator() (int i, int j, int k, int m) const
{
    BOUNDS_CHECK(i, j, k, m, 0);
    return memory.getElement<double> (INDEX(i, j, k, m, 0));
}

const double& Array::operator() (int i, int j, int k, int m, int n) const
{
    BOUNDS_CHECK(i, j, k, m, n);
    return memory.getElement<double> (INDEX(i, j, k, m, n));
}

Array Array::extract (Region R) const
{
    Region region = R.absolute (shape());
    Array A (region.shape());
    copyRegion (A, *this, region, 'A');
    return A;
}

void Array::insert (const Array& source, Region R)
{
    Region region = R.absolute (shape());

    if (source.shape() != region.shape())
    {
        throw std::runtime_error ("source and target regions have different shapes");
    }

    copyRegion (*this, source, region, 'B');
}

void Array::copyRegion (Array& dst, const Array& src, Region R, char mode)
{
    assert (! R.isRelative());

    const Range is = R.range (0);
    const Range js = R.range (1);
    const Range ks = R.range (2);
    const Range ms = R.range (3);
    const Range ns = R.range (4);
    const int i0 = is.lower;
    const int j0 = js.lower;
    const int k0 = ks.lower;
    const int m0 = ms.lower;
    const int n0 = ns.lower;

    for (int i = i0; i < is.upper; i += is.stride)
    for (int j = j0; j < js.upper; j += js.stride)
    for (int k = k0; k < ks.upper; k += ks.stride)
    for (int m = m0; m < ms.upper; m += ms.stride)
    for (int n = n0; n < ns.upper; n += ns.stride)
    {
        switch (mode)
        {
            case 'A': // Extract: the region refers to the source array.
            {
                dst (i - i0, j - j0, k - k0, m - m0, n - n0) = src (i, j, k, m, n);
                break;
            }
            case 'B': // Insert: the region refers to the target array.
            {
                dst (i, j, k, m, n) = src (i - i0, j - j0, k - k0, m - m0, n - n0);
                break;                
            }
        }
    }
}

Shape Array::shapeFromVector (std::vector<int> shapeVector)
{
    if (shapeVector.size() > 5)
    {
        throw std::runtime_error ("shape vector must have size <= 5");
    }

    return {{
        shapeVector.size() > 0 ? shapeVector[0] : 1,
        shapeVector.size() > 1 ? shapeVector[1] : 1,
        shapeVector.size() > 2 ? shapeVector[2] : 1,
        shapeVector.size() > 3 ? shapeVector[3] : 1,
        shapeVector.size() > 4 ? shapeVector[4] : 1,
    }};
}

std::vector<int> Array::vectorFromShape (Shape shape)
{
    int lastNonEmptyAxis = 4;

    while (shape[lastNonEmptyAxis] == 1 && lastNonEmptyAxis >= 1)
    {
        --lastNonEmptyAxis;
    }
    return std::vector<int> (&shape[0], &shape[lastNonEmptyAxis] + 1);
}





// ============================================================================
Array::Reference::Reference (Array& A, Region R) : A (A), R (R)
{
    assert (! R.isRelative());
}

const Array& Array::Reference::operator= (const Array& source)
{
    A.insert (source, R);
    return source;
}

const Array::Reference& Array::Reference::operator= (const Array::Reference& source)
{
    A.insert (Array (source), R);
    return source;
}

const Array& Array::Reference::getArray() const
{
    return A;
}

const Region& Array::Reference::getRegion() const
{
    return R;
}

Shape Array::Reference::shape() const
{
    return R.shape();
}

std::vector<int> Array::Reference::getShapeVector() const
{
    return getRegion().getShapeVector();
}

Array::Iterator Array::Reference::begin()
{
    return Iterator (A, R);
}

Array::Iterator Array::Reference::end()
{
    return Iterator (A, R, true);
}




// ============================================================================
Array::Iterator::Iterator (Array& A, Region R, bool isEnd) : A (A), R (R)
{
    assert (! R.isRelative());

    currentIndex = R.lower;
    currentAddress = isEnd ? A.end() : getAddress();
}

double* Array::Iterator::operator++ ()
{
    Index& I = currentIndex;

    for (int n = 4; n >= 0; --n)
    {
        I[n] += R.stride[n];

        if (I[n] < R.upper[n])
        {
            return currentAddress = getAddress();
        }
        I[n] = R.lower[n];
    }
    return currentAddress = A.end();
}

double& Array::Iterator::operator[] (int offset)
{
    return A[currentAddress - A.begin() + offset];
}

Array::Iterator::operator double*() const
{
    return currentAddress;
}

bool Array::Iterator::operator== (const Iterator& other) const
{
    return currentAddress == other.currentAddress;
}

void Array::Iterator::print (std::ostream& stream) const
{
    const Index& I = currentIndex;
    stream << I[0] << " " << I[1] << " " << I[2] << " " << I[3] << " " << I[4] << std::endl;
}

Index Array::Iterator::index() const
{
    return currentIndex;
}

Index Array::Iterator::relativeIndex() const
{
    Index I = currentIndex;
    for (int n = 0; n < 5; ++n)
    {
        I[n] -= R.lower[n];
    }
    return I;
}

double* Array::Iterator::getAddress() const
{
    // Computing the index directly may be 10-20% faster than calling
    // Array::operator().

    const Index& I = currentIndex;
    const char ordering = A.ordering;
    const int n1 = A.n1;
    const int n2 = A.n2;
    const int n3 = A.n3;
    const int n4 = A.n4;
    const int n5 = A.n5;
    return &A.memory.getElement<double> (INDEX(I[0], I[1], I[2], I[3], I[4]));
}
