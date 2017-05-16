#include <cassert>
#include "Matrix.hpp"
using namespace Cow;



Matrix::Matrix() : ni (0), nj (0)
{

}

Matrix::Matrix (int ni, int nj) : ni (ni), nj (nj)
{
    memory = HeapAllocation (ni * nj * sizeof (double));

    for (int i = 0; i < ni; ++i)
    {
        for (int j = 0; j < nj; ++j)
        {
            this->operator() (i, j) = double (i == j);
        }
    }
}

double& Matrix::operator() (int i, int j)
{
    return memory.getElement<double> (i * nj + j);
}

const double& Matrix::operator() (int i, int j) const
{
    return memory.getElement<double> (i * nj + j);
}

Matrix Matrix::operator* (double a) const
{
    Matrix C (ni, nj);

    for (int n = 0; n < ni * nj; ++n)
    {
        C.memory.getElement<double> (n) = memory.getElement<double> (n) * a;
    }
    return C;
}

Matrix Matrix::operator/ (double a) const
{
    Matrix C (ni, nj);

    for (int n = 0; n < ni * nj; ++n)
    {
        C.memory.getElement<double> (n) = memory.getElement<double> (n) / a;
    }
    return C;
}

Matrix Matrix::operator* (const Matrix& B) const
{
    assert (nj == B.ni);
    Matrix C (ni, B.nj);

    for (int i = 0; i < C.ni; ++i)
    {
        for (int j = 0; j < C.nj; ++j)
        {
            C (i, j) = 0.0;

            for (int k = 0; k < nj; ++k)
            {
                C (i, j) += this->operator() (i, k) * B (k, j);
            }
        }
    }

    return C;
}
