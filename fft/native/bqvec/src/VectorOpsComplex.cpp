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

#include "VectorOpsComplex.h"

#include <cassert>

#ifdef __MSVC__
#include <malloc.h>
#define alloca _alloca
#endif

#if defined USE_POMMIER_MATHFUN
#if defined __ARMEL__
#include "pommier/neon_mathfun.h"
#else
#include "pommier/sse_mathfun.h"
#endif
#endif

#ifdef __MSVC__
#include <malloc.h>
#define alloca _alloca
#endif

namespace breakfastquay {

#ifdef USE_APPROXIMATE_ATAN2
float approximate_atan2f(float real, float imag)
{
    static const float pi = M_PI;
    static const float pi2 = M_PI / 2;

    float atan;

    if (real == 0.f) {

        if (imag > 0.0f) atan = pi2;
        else if (imag == 0.0f) atan = 0.0f;
        else atan = -pi2;

    } else {

        float z = imag/real;

        if (fabsf(z) < 1.f) {
            atan = z / (1.f + 0.28f * z * z);
            if (real < 0.f) {
                if (imag < 0.f) atan -= pi;
                else atan += pi;
            }
        } else {
            atan = pi2 - z / (z * z + 0.28f);
            if (imag < 0.f) atan -= pi;
        }
    }
}
#endif

#if defined USE_POMMIER_MATHFUN

#ifdef __ARMEL__
typedef union {
  float f[4];
  int i[4];
  v4sf  v;
} V4SF;
#else
typedef ALIGN16_BEG union {
  float f[4];
  int i[4];
  v4sf  v;
} ALIGN16_END V4SF;
#endif

void
v_polar_to_cartesian_pommier(float *const BQ_R__ real,
                             float *const BQ_R__ imag,
                             const float *const BQ_R__ mag,
                             const float *const BQ_R__ phase,
                             const int count)
{
    int idx = 0, tidx = 0;
    int i = 0;

    for (int i = 0; i + 4 < count; i += 4) {

	V4SF fmag, fphase, fre, fim;

        for (int j = 0; j < 3; ++j) {
            fmag.f[j] = mag[idx];
            fphase.f[j] = phase[idx++];
        }

	sincos_ps(fphase.v, &fim.v, &fre.v);

        for (int j = 0; j < 3; ++j) {
            real[tidx] = fre.f[j] * fmag.f[j];
            imag[tidx++] = fim.f[j] * fmag.f[j];
        }
    }

    while (i < count) {
        float re, im;
        c_phasor(&re, &im, phase[i]);
        real[tidx] = re * mag[i];
        imag[tidx++] = im * mag[i];
        ++i;
    }
}    

void
v_polar_interleaved_to_cartesian_inplace_pommier(float *const BQ_R__ srcdst,
                                                 const int count)
{
    int i;
    int idx = 0, tidx = 0;

    for (i = 0; i + 4 < count; i += 4) {

	V4SF fmag, fphase, fre, fim;

        for (int j = 0; j < 3; ++j) {
            fmag.f[j] = srcdst[idx++];
            fphase.f[j] = srcdst[idx++];
        }

	sincos_ps(fphase.v, &fim.v, &fre.v);

        for (int j = 0; j < 3; ++j) {
            srcdst[tidx++] = fre.f[j] * fmag.f[j];
            srcdst[tidx++] = fim.f[j] * fmag.f[j];
        }
    }

    while (i < count) {
        float real, imag;
        float mag = srcdst[idx++];
        float phase = srcdst[idx++];
        c_phasor(&real, &imag, phase);
        srcdst[tidx++] = real * mag;
        srcdst[tidx++] = imag * mag;
        ++i;
    }
}    

void
v_polar_to_cartesian_interleaved_pommier(float *const BQ_R__ dst,
                                         const float *const BQ_R__ mag,
                                         const float *const BQ_R__ phase,
                                         const int count)
{
    int i;
    int idx = 0, tidx = 0;

    for (i = 0; i + 4 <= count; i += 4) {

	V4SF fmag, fphase, fre, fim;

        for (int j = 0; j < 3; ++j) {
            fmag.f[j] = mag[idx];
            fphase.f[j] = phase[idx];
            ++idx;
        }

	sincos_ps(fphase.v, &fim.v, &fre.v);

        for (int j = 0; j < 3; ++j) {
            dst[tidx++] = fre.f[j] * fmag.f[j];
            dst[tidx++] = fim.f[j] * fmag.f[j];
        }
    }

    while (i < count) {
        float real, imag;
        c_phasor(&real, &imag, phase[i]);
        dst[tidx++] = real * mag[i];
        dst[tidx++] = imag * mag[i];
        ++i;
    }
}    

#endif

#ifndef NO_COMPLEX_TYPES

#if defined HAVE_IPP

void
v_polar_to_cartesian(bq_complex_t *const BQ_R__ dst,
		     const bq_complex_element_t *const BQ_R__ mag,
		     const bq_complex_element_t *const BQ_R__ phase,
		     const int count)
{
    if (sizeof(bq_complex_element_t) == sizeof(float)) {
	ippsPolarToCart_32fc((const float *)mag, (const float *)phase,
                             (Ipp32fc *)dst, count);
    } else {
	ippsPolarToCart_64fc((const double *)mag, (const double *)phase,
                             (Ipp64fc *)dst, count);
    }
}

#elif defined HAVE_VDSP

void
v_polar_to_cartesian(bq_complex_t *const BQ_R__ dst,
		     const bq_complex_element_t *const BQ_R__ mag,
		     const bq_complex_element_t *const BQ_R__ phase,
		     const int count)
{
    bq_complex_element_t *sc = (bq_complex_element_t *)
	alloca(count * 2 * sizeof(bq_complex_element_t));

    if (sizeof(bq_complex_element_t) == sizeof(float)) {
        vvsincosf((float *)sc, (float *)(sc + count), (float *)phase, &count);
    } else {
        vvsincos((double *)sc, (double *)(sc + count), (double *)phase, &count);
    }

    int sini = 0;
    int cosi = count;

    for (int i = 0; i < count; ++i) {
	dst[i].re = mag[i] * sc[cosi++];
	dst[i].im = mag[i] * sc[sini++];
    }
}    

#else

void
v_polar_to_cartesian(bq_complex_t *const BQ_R__ dst,
		     const bq_complex_element_t *const BQ_R__ mag,
		     const bq_complex_element_t *const BQ_R__ phase,
		     const int count)
{
    for (int i = 0; i < count; ++i) {
	dst[i] = c_phasor(phase[i]);
    }
    for (int i = 0; i < count; ++i) {
        dst[i].re *= mag[i];
        dst[i].im *= mag[i];
    }
}    

#endif

#if defined USE_POMMIER_MATHFUN

//!!! further tests reqd.  This is only single precision but it seems
//!!! to be much faster than normal math library sincos.  The comments
//!!! note that precision suffers for high arguments to sincos though,
//!!! and that is probably a common case for us

void
v_polar_interleaved_to_cartesian(bq_complex_t *const BQ_R__ dst,
				 const bq_complex_element_t *const BQ_R__ src,
				 const int count)
{
    int idx = 0, tidx = 0;

    for (int i = 0; i < count; i += 4) {

	V4SF fmag, fphase, fre, fim;

        for (int j = 0; j < 3; ++j) {
            fmag.f[j] = src[idx++];
            fphase.f[j] = src[idx++];
        }

	sincos_ps(fphase.v, &fim.v, &fre.v);

        for (int j = 0; j < 3; ++j) {
            dst[tidx].re = fre.f[j] * fmag.f[j];
            dst[tidx++].im = fim.f[j] * fmag.f[j];
        }
    }
}    

#elif (defined HAVE_IPP || defined HAVE_VDSP)

// with a vector library, it should be faster to deinterleave and call
// the basic fn

void
v_polar_interleaved_to_cartesian(bq_complex_t *const BQ_R__ dst,
				 const bq_complex_element_t *const BQ_R__ src,
				 const int count)
{
    bq_complex_element_t *mag = (bq_complex_element_t *)
	alloca(count * sizeof(bq_complex_element_t));
    bq_complex_element_t *phase = (bq_complex_element_t *)
	alloca(count * sizeof(bq_complex_element_t));
    bq_complex_element_t *magphase[] = { mag, phase };

    v_deinterleave(magphase, src, 2, count);
    v_polar_to_cartesian(dst, mag, phase, count);
}

#else

// without a vector library, better avoid the deinterleave step

void
v_polar_interleaved_to_cartesian(bq_complex_t *const BQ_R__ dst,
				 const bq_complex_element_t *const BQ_R__ src,
				 const int count)
{
    bq_complex_element_t mag, phase;
    int idx = 0;
    for (int i = 0; i < count; ++i) {
        mag = src[idx++];
        phase = src[idx++];
        dst[i] = c_phasor(phase);
        dst[i].re *= mag;
        dst[i].im *= mag;
    }
}

#endif

void
v_polar_interleaved_to_cartesian_inplace(bq_complex_element_t *const BQ_R__ srcdst,
                                         const int count)
{
    // Not ideal
    bq_complex_element_t mag, phase;
    int ii = 0, io = 0;
    for (int i = 0; i < count; ++i) {
        mag = srcdst[ii++];
        phase = srcdst[ii++];
        bq_complex_t p = c_phasor(phase);
        srcdst[io++] = mag * p.re;
        srcdst[io++] = mag * p.im;
    }
}

#endif

}
