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
Range::Range (int index) : Range (index, index + 1)
{

}

Range::Range (int lower, int upper) : lower (lower), upper (upper)
{

}

Range::Range (const char* colon) : lower (0), upper (0)
{
    assert (std::strcmp (colon, ":") == 0);
}

int Range::absoluteLower (int size)
{
    return lower;
}

int Range::absoluteUpper (int size)
{
    return upper <= 0 ? upper + size : upper;
}

int Range::absoluteLength (int size)
{
    return absoluteUpper (size) - absoluteLower (size);
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

bool Region::isRelative()
{
    for (int n = 0; n < 5; ++n)
    {
        if (upper[n] <= 0) return true;
    }
    return false;
}

Region Region::absolute (Shape shape)
{
    Region R = *this;

    for (int n = 0; n < 5; ++n)
    {
        if (R.upper[n] <= 0) R.upper[n] += shape[n];        
    }
    return R;
}




// ============================================================================
Array::Array() : Array (0, 1, 1, 1, 1)
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

Array Array::extract (Range is) const
{
    Array A (is.absoluteLength (n1));
    copyRange (A, *this, is, ":", ":", ":", ":", 'A');
    return A;
}

Array Array::extract (Range is, Range js) const
{
    Array A (
        is.absoluteLength (n1),
        js.absoluteLength (n2));
    copyRange (A, *this, is, js, ":", ":", ":", 'A');
    return A;
}

Array Array::extract (Range is, Range js, Range ks) const
{
    Array A (
        is.absoluteLength (n1),
        js.absoluteLength (n2),
        ks.absoluteLength (n3));
    copyRange (A, *this, is, js, ks, ":", ":", 'A');
    return A;
}

Array Array::extract (Range is, Range js, Range ks, Range ms) const
{
    Array A (
        is.absoluteLength (n1),
        js.absoluteLength (n2),
        ks.absoluteLength (n3),
        ms.absoluteLength (n4));
    copyRange (A, *this, is, js, ks, ms, ":", 'A');
    return A;
}

Array Array::extract (Range is, Range js, Range ks, Range ms, Range ns) const
{
    Array A (
        is.absoluteLength (n1),
        js.absoluteLength (n2),
        ks.absoluteLength (n3),
        ms.absoluteLength (n4),
        ns.absoluteLength (n5));
    copyRange (A, *this, is, js, ks, ms, ns, 'A');
    return A;
}

void Array::insert (const Array& A, Range is)
{
    copyRange (*this, A, is, ":", ":", ":", ":", 'B');
}

void Array::insert (const Array& A, Range is, Range js)
{
    copyRange (*this, A, is, js, ":", ":", ":", 'B');
}

void Array::insert (const Array& A, Range is, Range js, Range ks)
{
    copyRange (*this, A, is, js, ks, ":", ":", 'B');
}

void Array::insert (const Array& A, Range is, Range js, Range ks, Range ms)
{
    copyRange (*this, A, is, js, ks, ms, ":", 'B');
}

void Array::insert (const Array& A, Range is, Range js, Range ks, Range ms, Range ns)
{
    copyRange (*this, A, is, js, ks, ms, ns, 'B');
}

Array::RangeExpression Array::iterate (Region R)
{
    Region absRange = R.absolute (shape());
    return RangeExpression (*this, absRange);
}

void Array::copyRange (Array& dst, const Array& src, Range is, Range js, Range ks, Range ms, Range ns, char mode)
{
    const int i0 = is.absoluteLower (src.n1);
    const int j0 = js.absoluteLower (src.n2);
    const int k0 = ks.absoluteLower (src.n3);
    const int m0 = ms.absoluteLower (src.n4);
    const int n0 = ns.absoluteLower (src.n5);
    const int i1 = is.absoluteUpper (src.n1);
    const int j1 = js.absoluteUpper (src.n2);
    const int k1 = ks.absoluteUpper (src.n3);
    const int m1 = ms.absoluteUpper (src.n4);
    const int n1 = ns.absoluteUpper (src.n5);

    for (int i = i0; i < i1; ++i)
    for (int j = j0; j < j1; ++j)
    for (int k = k0; k < k1; ++k)
    for (int m = m0; m < m1; ++m)
    for (int n = n0; n < n1; ++n)
    {
        switch (mode)
        {
            case 'A':
            {
                dst (i - i0, j - j0, k - k0, m - m0, n - n0) = src (i, j, k, m, n);
                break;
            }
            case 'B':
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
            return getAddress();
        }
        I[n] = R.lower[n];        
    }
    return sentinal = A.end();
}

Array::Iterator::operator double*() const
{
    return getAddress();
}

bool Array::Iterator::operator== (const Iterator& other) const
{
    return getAddress() == other.getAddress();
}

double* Array::Iterator::getAddress() const
{
    if (sentinal)
    {
        return A.end();
    }
    const Index& I = currentIndex;
    return &A (I[0], I[1], I[2], I[3], I[4]);
}