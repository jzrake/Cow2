#ifndef CowMatrix_hpp
#define CowMatrix_hpp

#include <cstdlib>
#include <array>
#include <vector>
#include "Array.hpp"




namespace Cow
{
    class Matrix;
};




class Cow::Matrix
{
public:

    /** Default constructor is an empty (0, 0) matrix. */
    Matrix();

    /** Construct the (ni, nj) identity matrix. */
    Matrix (int ni, int nj);

    /** Element access */
    double& operator() (int i, int j);

    /** Element access */
    const double& operator() (int i, int j) const;

    /** Scalar multiplication */
    Matrix operator* (double a) const;

    /** Scalar division */
    Matrix operator/ (double a) const;

    /** Matrix multiplication */
    Matrix operator* (const Matrix& B) const;

private:
    int ni, nj;
    HeapAllocation memory;
};

#endif