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

/* Based on
 *
       SUBROUTINE DPOTF2( UPLO, N, A, LDA, INFO )
 *
 *  -- LAPACK routine (version 3.3.1) --
 *  -- LAPACK is a software package provided by Univ. of Tennessee,    --
 *  -- Univ. of California Berkeley, Univ. of Colorado Denver and NAG Ltd..--
 *  -- April 2011                                                      --
 */

#ifndef FLENS_LAPACK_PO_POTF2_TCC
#define FLENS_LAPACK_PO_POTF2_TCC 1

#include <algorithm>
#include <flens/blas/blas.h>
#include <flens/lapack/lapack.h>

namespace flens { namespace lapack {

//== generic lapack implementation =============================================

namespace generic {

template <typename MA>
typename SyMatrix<MA>::IndexType
potf2_impl(SyMatrix<MA> &A)
{
    using std::isnan;
    using std::sqrt;

    typedef typename SyMatrix<MA>::ElementType  T;
    typedef typename SyMatrix<MA>::IndexType    IndexType;

    const Underscore<IndexType> _;

    const IndexType n = A.dim();
    const bool upper = (A.upLo()==Upper);

    const T Zero(0), One(1);

    IndexType info = 0;
//
//  Quick return if possible
//
    if (n==0) {
        return info;
    }
    if (upper) {
//
//      Compute the Cholesky factorization A = U**T *U.
//
        for (IndexType j=1; j<=n; ++j) {
//
//          Partition matrix
//
            const auto range1 = _(1,j-1);
            const auto range3 = _(j+1,n);

            const auto a12 = A(range1,j);
            const auto A13 = A(range1,range3);
            auto a23 = A(j,range3);
//
//          Compute U(J,J) and test for non-positive-definiteness.
//
            T a22 = A(j,j) - a12*a12;
            if (a22<=Zero || isnan(a22)) {
                A(j,j) = a22;
                info = j;
                break;
            }
            a22 = sqrt(a22);
            A(j,j) = a22;
//
//          Compute elements J+1:N of row J.
//
            if (j<n) {
                blas::mv(Trans, -One, A13, a12, One, a23);
                a23 *= One / a22;
            }
        }
    } else {
//
//      Compute the Cholesky factorization A = L*L**T.
//
        for (IndexType j=1; j<=n; ++j) {
//
//          Partition matrix
//
            const auto range1 = _(1,j-1);
            const auto range3 = _(j+1,n);

            const auto a21 = A(j,range1);
            const auto A31 = A(range3,range1);
            auto a32 = A(range3,j);
//
//          Compute L(J,J) and test for non-positive-definiteness.
//
            T a22 = A(j,j) - a21*a21;
            if (a22<=Zero || isnan(a22)) {
                A(j,j) = a22;
                info = j;
                break;
            }
            a22 = sqrt(a22);
            A(j,j) = a22;
//
//          Compute elements J+1:N of column J.
//
            if (j<n) {
                blas::mv(NoTrans, -One, A31, a21, One, a32);
                a32 *= One / a22;
            }
        }
    }
    return info;
}

template <typename MA>
typename HeMatrix<MA>::IndexType
potf2_impl(HeMatrix<MA> &A)
{
    using std::isnan;
    using std::sqrt;

    typedef typename HeMatrix<MA>::ElementType        T;
    typedef typename ComplexTrait<T>::PrimitiveType   PT;
    typedef typename HeMatrix<MA>::IndexType          IndexType;

    const Underscore<IndexType> _;

    const IndexType n = A.dim();
    const bool upper = (A.upLo()==Upper);

    const PT  Zero(0), One(1);
    const T   COne(1);

    IndexType info = 0;
//
//  Quick return if possible
//
    if (n==0) {
        return info;
    }
    if (upper) {
//
//      Compute the Cholesky factorization A = U**H *U.
//
        for (IndexType j=1; j<=n; ++j) {
//
//          Partition matrix
//
            const auto range1 = _(1,j-1);
            const auto range3 = _(j+1,n);

            auto a12 = A(range1,j);
            const auto A13 = A(range1,range3);
            auto a23 = A(j,range3);
//
//          Compute U(J,J) and test for non-positive-definiteness.
//
            PT a22 = cxxblas::real(A(j,j) - conjugate(a12)*a12);
            if (a22<=Zero || isnan(a22)) {
                A(j,j) = a22;
                info = j;
                break;
            }
            a22 = sqrt(a22);
            A(j,j) = a22;
//
//          Compute elements J+1:N of row J.
//
            if (j<n) {
                blas::conj(a12);
                blas::mv(Trans, -COne, A13, a12, COne, a23);
                blas::conj(a12);
                a23 *= One / a22;
            }
        }
    } else {
//
//      Compute the Cholesky factorization A = L*L**H.
//
        for (IndexType j=1; j<=n; ++j) {
//
//          Partition matrix
//
            const auto range1 = _(1,j-1);
            const auto range3 = _(j+1,n);

            auto a21 = A(j,range1);
            const auto A31 = A(range3,range1);
            auto a32 = A(range3,j);
//
//          Compute L(J,J) and test for non-positive-definiteness.
//
            PT a22 = cxxblas::real(A(j,j) - conjugate(a21)*a21);
            if (a22<=Zero || isnan(a22)) {
                A(j,j) = a22;
                info = j;
                break;
            }
            a22 = sqrt(a22);
            A(j,j) = a22;
//
//          Compute elements J+1:N of column J.
//
            if (j<n) {
                blas::conj(a21);
                blas::mv(NoTrans, -COne, A31, a21, COne, a32);
                blas::conj(a21);
                a32 *= One / a22;
            }
        }
    }
    return info;
}


} // namespace generic

//== interface for native lapack ===============================================

#ifdef USE_CXXLAPACK

namespace external {

template <typename MA>
typename SyMatrix<MA>::IndexType
potf2_impl(SyMatrix<MA> &A)
{
    typedef typename SyMatrix<MA>::IndexType  IndexType;

    IndexType info = cxxlapack::potf2<IndexType>(getF77Char(A.upLo()),
                                                 A.dim(),
                                                 A.data(),
                                                 A.leadingDimension());
    ASSERT(info>=0);
    return info;
}

template <typename MA>
typename HeMatrix<MA>::IndexType
potf2_impl(HeMatrix<MA> &A)
{
    typedef typename HeMatrix<MA>::IndexType  IndexType;

    IndexType info = cxxlapack::potf2<IndexType>(getF77Char(A.upLo()),
                                                 A.dim(),
                                                 A.data(),
                                                 A.leadingDimension());
    ASSERT(info>=0);
    return info;
}


} // namespace external

#endif // USE_CXXLAPACK

//== public interface ==========================================================

template <typename MA>
typename RestrictTo<IsRealSyMatrix<MA>::value
                 || IsHeMatrix<MA>::value,
         typename RemoveRef<MA>::Type::IndexType>::Type
potf2(MA &&A)
{
//
//  Remove references from rvalue types
//
    typedef typename RemoveRef<MA>::Type    MatrixA;
    typedef typename MatrixA::IndexType     IndexType;

//
//  Test the input parameters
//
    ASSERT(A.firstRow()==1);
    ASSERT(A.firstCol()==1);

#   ifdef CHECK_CXXLAPACK
//
//  Make copies of output arguments
//
    typename MatrixA::GeneralNoView     A_org      = A.general();
#   endif

//
//  Call implementation
//
    IndexType info = LAPACK_SELECT::potf2_impl(A);

#   ifdef CHECK_CXXLAPACK
//
//  Make copies of generic results
//
    typename MatrixA::GeneralNoView     A_generic  = A.general();
//
//  Restore output arguments
//
    A.general() = A_org;

//
//  Compare results
//
    IndexType _info = external::potf2_impl(A);

    bool failed = false;
    if (! isIdentical(A_generic, A.general(), "A_generic", "_A")) {
        std::cerr << "CXXLAPACK: A_generic = " << A_generic << std::endl;
        std::cerr << "F77LAPACK: A = " << A.general() << std::endl;
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

} } // namespace lapack, flens

#endif // FLENS_LAPACK_PO_POTF2_TCC
