/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    bqfft

    A small library wrapping various FFT implementations for some
    common audio processing use cases.

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

#ifndef TEST_COMPARES_H
#define TEST_COMPARES_H

// These macros are used for comparing generated results, and they
// aren't always going to be exact. Adding 0.1 to each value gives
// us a little more fuzz in qFuzzyCompare (which ultimately does
// the comparison).

#define COMPARE_ZERO(a) \
    QCOMPARE(a + 0.1, 0.1)

#define COMPARE_ZERO_F(a) \
    QCOMPARE(a + 0.1f, 0.1f)

#define COMPARE_FUZZIER(a, b) \
    QCOMPARE(a + 0.1, b + 0.1)

#define COMPARE_FUZZIER_F(a, b) \
    QCOMPARE(a + 0.1f, b + 0.1f)

#define COMPARE_ALL(a, n) \
    for (int cmp_i = 0; cmp_i < (int)(sizeof(a)/sizeof(a[0])); ++cmp_i) { \
        COMPARE_FUZZIER(a[cmp_i], n); \
    }

#define COMPARE_SCALED(a, b, s)						\
    for (int cmp_i = 0; cmp_i < (int)(sizeof(a)/sizeof(a[0])); ++cmp_i) { \
        COMPARE_FUZZIER(a[cmp_i] / s, b[cmp_i]); \
    }

#define COMPARE_ALL_F(a, n) \
    for (int cmp_i = 0; cmp_i < (int)(sizeof(a)/sizeof(a[0])); ++cmp_i) { \
        COMPARE_FUZZIER_F(a[cmp_i], n); \
    }

#define COMPARE_SCALED_F(a, b, s)						\
    for (int cmp_i = 0; cmp_i < (int)(sizeof(a)/sizeof(a[0])); ++cmp_i) { \
        COMPARE_FUZZIER_F(a[cmp_i] / s, b[cmp_i]); \
    }

#endif

