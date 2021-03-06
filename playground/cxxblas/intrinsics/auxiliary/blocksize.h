/*
 *   Copyright (c) 2012, Klaus Pototzky
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

#ifndef PLAYGROUND_CXXBLAS_INTRINSICS_AUXILIARY_BLOCKSIZE_H
#define PLAYGROUND_CXXBLAS_INTRINSICS_AUXILIARY_BLOCKSIZE_H 1

#include <math.h>

#ifdef USE_INTRINSIC

enum BlasFct
{
    GEMV,
    TRMV,
    TRSV,
    GEMM
};

template <typename T>
struct sizeOf {

};

template<>
struct sizeOf<float>
{
    static const size_t value = 4;
};


template<>
struct sizeOf<double>
{
    static const size_t value = 8;
};

template<>
struct sizeOf<std::complex<float> >
{
    static const size_t value = 8;
};

template<>
struct sizeOf<std::complex<double> >
{
    static const size_t value = 16;
};

template<BlasFct Fct, typename T, typename IndexType>
struct BlockSize {


};


template<typename T, typename IndexType>
struct BlockSize<TRMV, T, IndexType> {

    static const IndexType NBlockL1 = (L1_CACHE_SIZE/sizeOf<T>::value);
    static const IndexType NBlockL2 = (L2_CACHE_SIZE/sizeOf<T>::value);
    static const IndexType NBlockL3 = ((3*L3_CACHE_SIZE)/(2*sizeOf<T>::value));

};

template<typename T, typename IndexType>
struct BlockSize<TRSV, T, IndexType> {

    static const IndexType NBlockL1 = (L1_CACHE_SIZE/sizeOf<T>::value);
    static const IndexType NBlockL2 = (L2_CACHE_SIZE/sizeOf<T>::value);
    static const IndexType NBlockL3 = ((3*L3_CACHE_SIZE)/(2*sizeOf<T>::value));

};

template<typename T, typename IndexType>
struct BlockSize<GEMM, T, IndexType> {

    static const IndexType NBlockL1 = (L1_CACHE_SIZE/(3*sizeOf<T>::value));
    static const IndexType NBlockL2 = (L2_CACHE_SIZE/(3*sizeOf<T>::value));
    static const IndexType NBlockL3 = (L3_CACHE_SIZE/(3*sizeOf<T>::value));

};

#endif // USE_INTRINSIC

#endif // PLAYGROUND_CXXBLAS_INTRINSICS_AUXILIARY_BLOCKSIZE_H
