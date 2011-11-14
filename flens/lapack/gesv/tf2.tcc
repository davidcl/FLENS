/*
 *   Copyright (c) 2011, Michael Lehn
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *   1) Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2) Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *   3) Neither the name of the FLENS development group nor the names of
 *      its contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Baesed on
 *
      SUBROUTINE DGETF2( M, N, A, LDA, IPIV, INFO )
 *
 *  -- LAPACK routine (version 3.2) --
 *  -- LAPACK is a software package provided by Univ. of Tennessee,    --
 *  -- Univ. of California Berkeley, Univ. of Colorado Denver and NAG Ltd..--
 *     November 2006
 */

#ifndef FLENS_LAPACK_GESV_TF2_TCC
#define FLENS_LAPACK_GESV_TF2_TCC 1

#include <algorithm>
#include <flens/blas/blas.h>
#include <flens/lapack/lapack.h>

namespace flens { namespace lapack {

//== generic lapack implementation =============================================

template <typename MA, typename VP>
typename GeMatrix<MA>::IndexType
tf2_generic(GeMatrix<MA> &A, DenseVector<VP> &piv)
{
    using lapack::lamch;
    using std::abs;
    using std::min;

    typedef typename GeMatrix<MA>::IndexType    IndexType;
    typedef typename GeMatrix<MA>::ElementType  T;

    const Underscore<IndexType> _;

    const IndexType m = A.numRows();
    const IndexType n = A.numCols();

    IndexType info = 0;

//
//  Quick return if possible
//
    if ((m==0) || (n==0)) {
        return info;
    }
//
//     Compute machine safe minimum 
//
    T safeMin = lamch<T>(SafeMin);

    for (IndexType j=1; j<=min(m,n); ++j) {
//
//      Row range of current submatrix A(j:M, j:N)
//
        const auto rows = _(j, m);
//
//      Row and column range of trailing submatrix A(j+1:M, j+1:N)
//
        const auto _rows = _(j+1, m);
        const auto _cols = _(j+1, n);
//
//      Find pivot and test for singularity.
//
        IndexType jp = j - 1 + blas::iamax(A(rows,j));
        piv(j) = jp;
        if (A(jp, j)!=T(0)) {
//
//          Apply the interchange to columns 1:N.
//
            if (j!=jp) {
                blas::swap(A(j,_), A(jp,_));
            }
//
//          Compute elements J+1:M of J-th column.
//
            if (j<m) {
                if (abs(A(j,j))>=safeMin) {
                    blas::scal(T(1)/A(j, j), A(_rows,j));
                } else {
                    for (IndexType i=1; i<=m-j; ++i) {
                        A(j+i,j) = A(j+i,j)/A(j,j);
                    }
                }
            }
        } else {
            if (info==0) {
                info = j;
            }
        }
//
//      Update trailing submatrix A(j+1:M, j+1:N)
//
        blas::r(T(-1), A(_rows,j), A(j,_cols), A(_rows,_cols));
    }

    return info;
}

//== interface for native lapack ===============================================

#ifdef CHECK_CXXLAPACK

template <typename MA, typename VP>
typename GeMatrix<MA>::IndexType
tf2_native(GeMatrix<MA> &_A, DenseVector<VP> &piv)
{
    typedef typename GeMatrix<MA>::ElementType  T;

    const INTEGER    M = _A.numRows();
    const INTEGER    N = _A.numCols();
    T               *A = _A.data();
    const INTEGER    LDA = _A.leadingDimension();
    INTEGER         *IPIV = piv.data();
    INTEGER          INFO;

    if (IsSame<T, DOUBLE>::value) {
        LAPACK_IMPL(dgetf2)(&M, &N, A, &LDA, IPIV, &INFO);
    } else {
        ASSERT(0);
    }
    return INFO;
}

#endif // CHECK_CXXLAPACK

//== public interface ==========================================================

template <typename MA, typename VP>
typename GeMatrix<MA>::IndexType
tf2(GeMatrix<MA> &A, DenseVector<VP> &piv)
{
    typedef typename GeMatrix<MA>::IndexType    IndexType;

//
//  Test the input parameters
//
    ASSERT(A.firstRow()==1);
    ASSERT(A.firstCol()==1);
    ASSERT((piv.inc()>0 && piv.firstIndex()==1)
        || (piv.inc()<0 && piv.firstIndex()==A.numRows()));

#   ifdef CHECK_CXXLAPACK
//
//  Make copies of output arguments
//
    typename GeMatrix<MA>::NoView       _A      = A;
    typename DenseVector<VP>::NoView    _piv    = piv;
#   endif

//
//  Call implementation
//
    IndexType info = tf2_generic(A, piv);

#   ifdef CHECK_CXXLAPACK
//
//  Compare results
//
    IndexType _info = tf2_native(_A, _piv);

    bool failed = false;
    if (! isIdentical(A, _A, " A", "_A")) {
        std::cerr << "CXXLAPACK:  A = " << A << std::endl;
        std::cerr << "F77LAPACK: _A = " << _A << std::endl;
        failed = true;
    }

    if (! isIdentical(piv, _piv, " piv", "_piv")) {
        std::cerr << "CXXLAPACK:  piv = " << piv << std::endl;
        std::cerr << "F77LAPACK: _piv = " << _piv << std::endl;
        failed = true;
    }

    if (! isIdentical(info, _info, " info", "_info")) {
        std::cerr << "CXXLAPACK:  info = " << info << std::endl;
        std::cerr << "F77LAPACK: _info = " << _info << std::endl;
        failed = true;
    }

    if (failed) {
        ASSERT(0);
    }
#   endif

    return info;
}

//-- forwarding ----------------------------------------------------------------
template <typename MA, typename VP>
typename MA::IndexType
tf2(MA &&A, VP &&piv)
{
    typedef typename MA::IndexType  IndexType;

    CHECKPOINT_ENTER;
    IndexType info =  tf2(A, piv);
    CHECKPOINT_LEAVE;

    return info;
}

} } // namespace lapack, flens

#endif // FLENS_LAPACK_GESV_TF2_TCC
