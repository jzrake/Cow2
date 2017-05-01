#include <iostream> // DEBUG
#include <memory>
#include <cassert>
#include "Array.hpp"

#define BOUNDS_CHECK(i, j, k, m, n) assert (true \
&& 0 <= i && i < n1 \
&& 0 <= j && j < n2 \
&& 0 <= k && k < n3 \
&& 0 <= m && m < n4 \
&& 0 <= n && n < n5)
#define INDEXC(i, j, k, m, n) (n5 * (n4 * (n3 * (n2 * i + j) + k) + m) + n)
#define INDEXF(i, j, k, m, n) (n1 * (n2 * (n3 * (n4 * n + m) + k) + j) + i)
#define INDEX(i, j, k, m, n) (ordering == 'C' ? INDEXC(i, j, k, m, n) : INDEXF(i, j, k, m, n))

using namespace Cow;




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

std::size_t HeapAllocation::size()
{
    return numberOfBytes;
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
    return upper <= 0;
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
    const int L = lower;
    const int U = upper <= 0 ? upper + absoluteSize : upper;
    return Range (L, U, stride);
}




// ============================================================================
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

Shape Region::shape() const
{
    return {{ range (0).size(), 0, 0, 0, 0 }};
}

Region Region::absolute (Shape shape) const
{
    Region R = *this;

    for (int n = 0; n < 5; ++n)
    {
        if (R.upper[n] <= 0) R.upper[n] += shape[n];        
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
    std::vector<int> S;

    if (n1 > 1) S.push_back (n1);
    if (n2 > 1) S.push_back (n2);
    if (n3 > 1) S.push_back (n3);
    if (n4 > 1) S.push_back (n4);
    if (n5 > 1) S.push_back (n5);

    return S;
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

double& Array::operator() (int i)
{
    BOUNDS_CHECK(i, 0, 0, 0, 0);
    return memory.getElement<double> (i);
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
    return memory.getElement<double> (i);
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

void Array::insert (const Array& A, Region R)
{
    Region region = R.absolute (shape());
    copyRegion (*this, A, region, 'B');
}

Array::RangeExpression Array::iterate (Region R)
{
    Region absRange = R.absolute (shape());
    return RangeExpression (*this, absRange);
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
            case 'A': // The input range refers to the source array.
            {
                dst (i - i0, j - j0, k - k0, m - m0, n - n0) = src (i, j, k, m, n);
                break;
            }
            case 'B': // The input range refers to the target array.
            {
                dst (i, j, k, m, n) = src (i + i0, j + j0, k + k0, m + m0, n + n0);
                break;                
            }
        }
    }
}



// ============================================================================
Array::RangeExpression::RangeExpression (Array& A, Region& R) : A (A), R (R)
{
    assert (! R.isRelative());
}

Array::Iterator Array::RangeExpression::begin()
{
    return Iterator (A, R);
}

Array::Iterator Array::RangeExpression::end()
{
    return Iterator (A, R, true);
}




// ============================================================================
Array::Iterator::Iterator (Array& A, Region& R, bool isEnd) : A (A), R (R)
{
    assert (! R.isRelative());

    currentIndex = R.lower;
    currentAddress = getAddress();
    sentinal = isEnd ? A.end() : nullptr;
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
    return sentinal = A.end();
}

Array::Iterator::operator double*() const
{
    return currentAddress;
}

bool Array::Iterator::operator== (const Iterator& other) const
{
    return currentAddress == other.currentAddress;
}

double* Array::Iterator::getAddress() const
{
    if (sentinal)
    {
        return A.end();
    }

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
