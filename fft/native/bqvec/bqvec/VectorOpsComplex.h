/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    bqvec

    A small library for vector arithmetic and allocation in C++ using
    raw C pointer arrays.

    Copyright 2007-2015 Particular Programs Ltd.

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Except as contained in this notice, the names of Chris Cannam and
    Particular Programs Ltd shall not be used in advertising or
    otherwise to promote the sale, use or other dealings in this
    Software without prior written authorization.
*/

#ifndef BQVEC_VECTOR_OPS_COMPLEX_H
#define BQVEC_VECTOR_OPS_COMPLEX_H

#include "VectorOps.h"
#include "ComplexTypes.h"

#include <cmath>

namespace breakfastquay {

#ifndef NO_COMPLEX_TYPES

template<> 
inline void v_zero(bq_complex_t *const BQ_R__ ptr, 
                   const int count)
{
#if defined HAVE_IPP
    if (sizeof(bq_complex_element_t) == sizeof(float)) {
        ippsZero_32fc((Ipp32fc *)ptr, count);
    } else {
        ippsZero_64fc((Ipp64fc *)ptr, count);
    }
#elif defined HAVE_VDSP
    if (sizeof(bq_complex_element_t) == sizeof(float)) {
        vDSP_vclr((float *)ptr, 1, count * 2);
    } else {
        vDSP_vclrD((double *)ptr, 1, count * 2);
    }
#else
    const bq_complex_element_t value = 0.0;
    for (int i = 0; i < count; ++i) {
        ptr[i].re = value;
        ptr[i].im = value;
    }
#endif
}

#if defined HAVE_IPP
template<>
inline void v_copy(bq_complex_t *const BQ_R__ dst,
                   const bq_complex_t *const BQ_R__ src,
                   const int count)
{
    if (sizeof(bq_complex_element_t) == sizeof(float)) {
        ippsCopy_32fc((const Ipp32fc *)src, (Ipp32fc *)dst, count);
    } else {
        ippsCopy_64fc((const Ipp64fc *)src, (Ipp64fc *)dst, count);
    }
}
template<>
inline void v_move(bq_complex_t *const BQ_R__ dst,
                   const bq_complex_t *const BQ_R__ src,
                   const int count)
{
    if (sizeof(bq_complex_element_t) == sizeof(float)) {
        ippsMove_32fc((const Ipp32fc *)src, (Ipp32fc *)dst, count);
    } else {
        ippsMove_64fc((const Ipp64fc *)src, (Ipp64fc *)dst, count);
    }
}
#endif

template<>
inline void v_convert(bq_complex_t *const BQ_R__ dst,
                      const bq_complex_element_t *const BQ_R__ src,
                      const int srccount)
{
    const int targetcount = srccount / 2;
    int srcidx = 0;
    for (int i = 0; i < targetcount; ++i) {
        dst[i].re = src[srcidx++];
        dst[i].im = src[srcidx++];
    }
}

template<>
inline void v_convert(bq_complex_element_t *const BQ_R__ dst,
                      const bq_complex_t *const BQ_R__ src,
                      const int srccount)
{
    int targetidx = 0;
    for (int i = 0; i < srccount; ++i) {
        dst[targetidx++] = src[i].re;
        dst[targetidx++] = src[i].im;
    }
}

inline void c_add(bq_complex_t &dst,
                  const bq_complex_t &src)
{
    dst.re += src.re;
    dst.im += src.im;
}

inline void c_add_with_gain(bq_complex_t &dst,
                            const bq_complex_t &src,
                            const bq_complex_element_t gain)
{
    dst.re += src.re * gain;
    dst.im += src.im * gain;
}

inline void c_multiply(bq_complex_t &dst,
                       const bq_complex_t &src1,
                       const bq_complex_t &src2)
{
    // Note dst may alias src1 or src2.

    // The usual formula -- four multiplies, one add and one subtract
    // 
    // (x1 + y1i)(x2 + y2i) = (x1x2 - y1y2) + (x1y2 + y1x2)i
    //
    // Alternative formula -- three multiplies, two adds, three
    // subtracts
    // 
    // (x1 + y1i)(x2 + y2i) = (x1x2 - y1y2) + ((x1 + y1)(x2 + y2) - x1x2 - y1y2)i
    //
    // The first formulation tests marginally quicker here.

    bq_complex_element_t real = src1.re * src2.re - src1.im * src2.im;
    bq_complex_element_t imag = src1.re * src2.im + src1.im * src2.re;

    dst.re = real;
    dst.im = imag;
}

inline void c_multiply(bq_complex_t &dst,
                       const bq_complex_t &src)
{
    c_multiply(dst, dst, src);
}

inline void c_multiply_and_add(bq_complex_t &dst,
                               const bq_complex_t &src1,
                               const bq_complex_t &src2)
{
    bq_complex_t tmp;
    c_multiply(tmp, src1, src2);
    c_add(dst, tmp);
}

template<>
inline void v_add(bq_complex_t *const BQ_R__ dst,
                  const bq_complex_t *const BQ_R__ src,
                  const int count)
{
#if defined HAVE_IPP
    if (sizeof(bq_complex_element_t) == sizeof(float)) {
        ippsAdd_32fc_I((Ipp32fc *)src, (Ipp32fc *)dst, count);
    } else {
        ippsAdd_64fc_I((Ipp64fc *)src, (Ipp64fc *)dst, count);
    }
#else
    for (int i = 0; i < count; ++i) {
        dst[i].re += src[i].re;
        dst[i].im += src[i].im;
    }
#endif
}    

template<>
inline void v_add_with_gain(bq_complex_t *const BQ_R__ dst,
                            const bq_complex_t *const BQ_R__ src,
                            const bq_complex_element_t gain,
                            const int count)
{
    for (int i = 0; i < count; ++i) {
        dst[i].re += src[i].re * gain;
        dst[i].im += src[i].im * gain;
    }
}

template<>
inline void v_multiply(bq_complex_t *const BQ_R__ dst,
                       const bq_complex_t *const BQ_R__ src,
                       const int count)
{
#ifdef HAVE_IPP
    if (sizeof(bq_complex_element_t) == sizeof(float)) {
        ippsMul_32fc_I((const Ipp32fc *)src, (Ipp32fc *)dst, count);
    } else {
        ippsMul_64fc_I((const Ipp64fc *)src, (Ipp64fc *)dst, count);
    }
#else
    for (int i = 0; i < count; ++i) {
        c_multiply(dst[i], src[i]);
    }
#endif
}

template<>
inline void v_multiply(bq_complex_t *const BQ_R__ dst,
                       const bq_complex_t *const BQ_R__ src1,
                       const bq_complex_t *const BQ_R__ src2,
                       const int count)
{
#ifdef HAVE_IPP
    if (sizeof(bq_complex_element_t) == sizeof(float)) {
        ippsMul_32fc((const Ipp32fc *)src1, (const Ipp32fc *)src2,
                     (Ipp32fc *)dst, count);
    } else {
        ippsMul_64fc((const Ipp64fc *)src1, (const Ipp64fc *)src2,
                     (Ipp64fc *)dst, count);
    }
#else
    for (int i = 0; i < count; ++i) {
        c_multiply(dst[i], src1[i], src2[i]);
    }
#endif
}

template<>
inline void v_multiply_and_add(bq_complex_t *const BQ_R__ dst,
                               const bq_complex_t *const BQ_R__ src1,
                               const bq_complex_t *const BQ_R__ src2,
                               const int count)
{
#ifdef HAVE_IPP
    if (sizeof(bq_complex_element_t) == sizeof(float)) {
        ippsAddProduct_32fc((const Ipp32fc *)src1, (const Ipp32fc *)src2,
                            (Ipp32fc *)dst, count);
    } else {
        ippsAddProduct_64fc((const Ipp64fc *)src1, (const Ipp64fc *)src2,
                            (Ipp64fc *)dst, count);
    }
#else
    for (int i = 0; i < count; ++i) {
        c_multiply_and_add(dst[i], src1[i], src2[i]);
    }
#endif
}

#if defined( __GNUC__ ) && defined( _WIN32 )
// MinGW doesn't appear to have sincos, so define it -- it's
// a single x87 instruction anyway
static inline void sincos(double x, double *sin, double *cos) {
    __asm__ ("fsincos;" : "=t" (*cos), "=u" (*sin) : "0" (x) : "st(7)");
}
static inline void sincosf(float fx, float *fsin, float *fcos) {
    double sin, cos;
    sincos(fx, &sin, &cos);
    *fsin = sin;
    *fcos = cos;
}
#endif

#endif /* !NO_COMPLEX_TYPES */

template<typename T>
inline void c_phasor(T *real, T *imag, T phase)
{
    //!!! IPP contains ippsSinCos_xxx in ippvm.h -- these are
    //!!! fixed-accuracy, test and compare
#if defined HAVE_VDSP
    int one = 1;
    if (sizeof(T) == sizeof(float)) {
        vvsincosf((float *)imag, (float *)real, (const float *)&phase, &one);
    } else {
        vvsincos((double *)imag, (double *)real, (const double *)&phase, &one);
    }
#elif defined LACK_SINCOS
    if (sizeof(T) == sizeof(float)) {
        *real = cosf(phase);
        *imag = sinf(phase);
    } else {
        *real = cos(phase);
        *imag = sin(phase);
    }
#elif defined __GNUC__
    if (sizeof(T) == sizeof(float)) {
        sincosf(float(phase), (float *)imag, (float *)real);
    } else {
        sincos(phase, (double *)imag, (double *)real);
    }
#else
    if (sizeof(T) == sizeof(float)) {
        *real = cosf(phase);
        *imag = sinf(phase);
    } else {
        *real = cos(phase);
        *imag = sin(phase);
    }
#endif
}

template<typename T>
inline void c_magphase(T *mag, T *phase, T real, T imag)
{
    *mag = sqrt(real * real + imag * imag);
    *phase = atan2(imag, real);
}

#ifdef USE_APPROXIMATE_ATAN2
// NB arguments in opposite order from usual for atan2f
extern float approximate_atan2f(float real, float imag);
template<>
inline void c_magphase(float *mag, float *phase, float real, float imag)
{
    float atan = approximate_atan2f(real, imag);
    *phase = atan;
    *mag = sqrtf(real * real + imag * imag);
}
#else
template<>
inline void c_magphase(float *mag, float *phase, float real, float imag)
{
    *mag = sqrtf(real * real + imag * imag);
    *phase = atan2f(imag, real);
}
#endif

#ifndef NO_COMPLEX_TYPES

inline bq_complex_t c_phasor(bq_complex_element_t phase)
{
    bq_complex_t c;
    c_phasor<bq_complex_element_t>(&c.re, &c.im, phase);
    return c;
}

inline void c_magphase(bq_complex_element_t *mag, bq_complex_element_t *phase,
                       bq_complex_t c)
{
    c_magphase<bq_complex_element_t>(mag, phase, c.re, c.im);
}

void v_polar_to_cartesian(bq_complex_t *const BQ_R__ dst,
                          const bq_complex_element_t *const BQ_R__ mag,
                          const bq_complex_element_t *const BQ_R__ phase,
                          const int count);

void v_polar_interleaved_to_cartesian(bq_complex_t *const BQ_R__ dst,
                                      const bq_complex_element_t *const BQ_R__ src,
                                      const int count);

inline void v_cartesian_to_polar(bq_complex_element_t *const BQ_R__ mag,
                                 bq_complex_element_t *const BQ_R__ phase,
                                 const bq_complex_t *const BQ_R__ src,
                                 const int count)
{
    for (int i = 0; i < count; ++i) {
        c_magphase<bq_complex_element_t>(mag + i, phase + i, src[i].re, src[i].im);
    }
}

inline void v_cartesian_to_polar_interleaved(bq_complex_element_t *const BQ_R__ dst,
                                             const bq_complex_t *const BQ_R__ src,
                                             const int count)
{
    for (int i = 0; i < count; ++i) {
        c_magphase<bq_complex_element_t>(&dst[i*2], &dst[i*2+1],
                                    src[i].re, src[i].im);
    }
}

#endif /* !NO_COMPLEX_TYPES */

template<typename S, typename T> // S source, T target
void v_polar_to_cartesian(T *const BQ_R__ real,
                          T *const BQ_R__ imag,
                          const S *const BQ_R__ mag,
                          const S *const BQ_R__ phase,
                          const int count)
{
    for (int i = 0; i < count; ++i) {
        c_phasor<T>(real + i, imag + i, phase[i]);
    }
    v_multiply(real, mag, count);
    v_multiply(imag, mag, count);
}

template<typename T>
void v_polar_interleaved_to_cartesian_inplace(T *const BQ_R__ srcdst,
                                              const int count)
{
    T real, imag;
    for (int i = 0; i < count*2; i += 2) {
        c_phasor(&real, &imag, srcdst[i+1]);
        real *= srcdst[i];
        imag *= srcdst[i];
        srcdst[i] = real;
        srcdst[i+1] = imag;
    }
}

template<typename S, typename T> // S source, T target
void v_polar_to_cartesian_interleaved(T *const BQ_R__ dst,
                                      const S *const BQ_R__ mag,
                                      const S *const BQ_R__ phase,
                                      const int count)
{
    T real, imag;
    for (int i = 0; i < count; ++i) {
        c_phasor<T>(&real, &imag, phase[i]);
        real *= mag[i];
        imag *= mag[i];
        dst[i*2] = real;
        dst[i*2+1] = imag;
    }
}    

#if defined USE_POMMIER_MATHFUN
void v_polar_to_cartesian_pommier(float *const BQ_R__ real,
                                  float *const BQ_R__ imag,
                                  const float *const BQ_R__ mag,
                                  const float *const BQ_R__ phase,
                                  const int count);
void v_polar_interleaved_to_cartesian_inplace_pommier(float *const BQ_R__ srcdst,
                                                      const int count);
void v_polar_to_cartesian_interleaved_pommier(float *const BQ_R__ dst,
                                              const float *const BQ_R__ mag,
                                              const float *const BQ_R__ phase,
                                              const int count);

template<>
inline void v_polar_to_cartesian(float *const BQ_R__ real,
                                 float *const BQ_R__ imag,
                                 const float *const BQ_R__ mag,
                                 const float *const BQ_R__ phase,
                                 const int count)
{
    v_polar_to_cartesian_pommier(real, imag, mag, phase, count);
}

template<>
inline void v_polar_interleaved_to_cartesian_inplace(float *const BQ_R__ srcdst,
                                                     const int count)
{
    v_polar_interleaved_to_cartesian_inplace_pommier(srcdst, count);
}

template<>
inline void v_polar_to_cartesian_interleaved(float *const BQ_R__ dst,
                                             const float *const BQ_R__ mag,
                                             const float *const BQ_R__ phase,
                                             const int count)
{
    v_polar_to_cartesian_interleaved_pommier(dst, mag, phase, count);
}

#endif

template<typename S, typename T> // S source, T target
void v_cartesian_to_polar(T *const BQ_R__ mag,
                          T *const BQ_R__ phase,
                          const S *const BQ_R__ real,
                          const S *const BQ_R__ imag,
                          const int count)
{
    for (int i = 0; i < count; ++i) {
        c_magphase<T>(mag + i, phase + i, real[i], imag[i]);
    }
}

template<typename S, typename T> // S source, T target
void v_cartesian_interleaved_to_polar(T *const BQ_R__ mag,
                                      T *const BQ_R__ phase,
                                      const S *const BQ_R__ src,
                                      const int count)
{
    for (int i = 0; i < count; ++i) {
        c_magphase<T>(mag + i, phase + i, src[i*2], src[i*2+1]);
    }
}

#ifdef HAVE_VDSP
template<>
inline void v_cartesian_to_polar(float *const BQ_R__ mag,
                                 float *const BQ_R__ phase,
                                 const float *const BQ_R__ real,
                                 const float *const BQ_R__ imag,
                                 const int count)
{
    DSPSplitComplex c;
    c.realp = const_cast<float *>(real);
    c.imagp = const_cast<float *>(imag);
    vDSP_zvmags(&c, 1, phase, 1, count); // using phase as a temporary dest
    vvsqrtf(mag, phase, &count); // using phase as the source
    vvatan2f(phase, imag, real, &count);
}
template<>
inline void v_cartesian_to_polar(double *const BQ_R__ mag,
                                 double *const BQ_R__ phase,
                                 const double *const BQ_R__ real,
                                 const double *const BQ_R__ imag,
                                 const int count)
{
    // double precision, this is significantly faster than using vDSP_polar
    DSPDoubleSplitComplex c;
    c.realp = const_cast<double *>(real);
    c.imagp = const_cast<double *>(imag);
    vDSP_zvmagsD(&c, 1, phase, 1, count); // using phase as a temporary dest
    vvsqrt(mag, phase, &count); // using phase as the source
    vvatan2(phase, imag, real, &count);
}
#endif

template<typename T>
void v_cartesian_to_polar_interleaved_inplace(T *const BQ_R__ srcdst,
                                              const int count)
{
    T mag, phase;
    for (int i = 0; i < count * 2; i += 2) {
        c_magphase(&mag, &phase, srcdst[i], srcdst[i+1]);
        srcdst[i] = mag;
        srcdst[i+1] = phase;
    }
}

}

#endif

