/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "bqvec/VectorOpsComplex.h"

#include <iostream>
#include <cstdlib>

#include <time.h>

using namespace std;

namespace breakfastquay {

namespace Test {

#ifdef _WIN32
#define drand48() (-1+2*((float)rand())/RAND_MAX)
#endif

bool
testMultiply()
{
    cerr << "testVectorOps: testing v_multiply complex" << endl;

    const int N = 1024;
    bq_complex_t target[N];
    bq_complex_t src1[N];
    bq_complex_t src2[N];

    for (int i = 0; i < N; ++i) {
	src1[i].re = drand48();
	src1[i].im = drand48();
	src2[i].re = drand48();
	src2[i].im = drand48();
    }

    double mean, first, last, total = 0;
    for (int i = 0; i < N; ++i) {
        bq_complex_t result;
        c_multiply(result, src1[i], src2[i]);
	if (i == 0) first = result.re;
	if (i == N-1) last = result.im;
	total += result.re;
	total += result.im;
    }
    mean = total / (N*2);
    cerr << "Naive method: mean = " << mean << ", first = " << first
	 << ", last = " << last << endl;

    v_multiply(target, src1, src2, N);
    total = 0;

    for (int i = 0; i < N; ++i) {
	if (i == 0) first = target[i].re;
	if (i == N-1) last = target[i].im;
	total += target[i].re;
	total += target[i].im;
    }
    mean = total / (N*2);
    cerr << "v_multiply: mean = " << mean << ", first = " << first
	 << ", last = " << last << endl;

    int iterations = 50000;
    cerr << "Iterations: " << iterations << endl;
	
    cerr << "CLOCKS_PER_SEC = " << CLOCKS_PER_SEC << endl;
    float divisor = float(CLOCKS_PER_SEC) / 1000.f;

    clock_t start = clock();

    for (int j = 0; j < iterations; ++j) {
	for (int i = 0; i < N; ++i) {
            c_multiply(target[i], src1[i], src2[i]);
	}
    }
    
    clock_t end = clock();

    cerr << "Time for naive method: " << float(end - start)/divisor << endl;

    start = clock();

    for (int j = 0; j < iterations; ++j) {
        v_multiply(target, src1, src2, N);
    }
    
    end = clock();

    cerr << "Time for v_multiply: " << float(end - start)/divisor << endl;

    return true;
}

bool
testPolarToCart()
{
    cerr << "testVectorOps: testing v_polar_to_cartesian" << endl;

    const int N = 1024;
    bq_complex_t target[N];
    double mag[N];
    double phase[N];

    for (int i = 0; i < N; ++i) {
	mag[i] = drand48();
	phase[i] = (drand48() * M_PI * 2) - M_PI;
    }

    double mean, first, last, total = 0;
    for (int i = 0; i < N; ++i) {
	double real = mag[i] * cos(phase[i]);
	double imag = mag[i] * sin(phase[i]);
	if (i == 0) first = real;
	if (i == N-1) last = imag;
	total += real;
	total += imag;
    }
    mean = total / (N*2);
    cerr << "Naive method: mean = " << mean << ", first = " << first
	 << ", last = " << last << endl;

    v_polar_to_cartesian(target, mag, phase, N);

    total = 0;

    for (int i = 0; i < N; ++i) {
	if (i == 0) first = target[i].re;
	if (i == N-1) last = target[i].im;
	total += target[i].re;
	total += target[i].im;
    }
    mean = total / (N*2);
    cerr << "v_polar_to_cartesian: mean = " << mean << ", first = " << first
	 << ", last = " << last << endl;

    int iterations = 10000;
    cerr << "Iterations: " << iterations << endl;
	
    cerr << "CLOCKS_PER_SEC = " << CLOCKS_PER_SEC << endl;
    float divisor = float(CLOCKS_PER_SEC) / 1000.f;

    clock_t start = clock();

    for (int j = 0; j < iterations; ++j) {
	for (int i = 0; i < N; ++i) {
	    target[i].re = mag[i] * cos(phase[i]);
	    target[i].im = mag[i] * sin(phase[i]);
	}
    }
    
    clock_t end = clock();

    cerr << "Time for naive method: " << float(end - start)/divisor << endl;

    start = clock();

    for (int j = 0; j < iterations; ++j) {
	v_polar_to_cartesian(target, mag, phase, N);
    }
    
    end = clock();

    cerr << "Time for v_polar_to_cartesian: " << float(end - start)/divisor << endl;

    return true;
}

bool
testPolarToCartInterleaved()
{
    cerr << "testVectorOps: testing v_polar_interleaved_to_cartesian" << endl;

    const int N = 1024;
    bq_complex_t target[N];
    double source[N*2];

    for (int i = 0; i < N; ++i) {
	source[i*2] = drand48();
	source[i*2+1] = (drand48() * M_PI * 2) - M_PI;
    }

    double mean, first, last, total = 0;
    for (int i = 0; i < N; ++i) {
	double real = source[i*2] * cos(source[i*2+1]);
	double imag = source[i*2] * sin(source[i*2+1]);
	if (i == 0) first = real;
	if (i == N-1) last = imag;
	total += real;
	total += imag;
    }
    mean = total / (N*2);
    cerr << "Naive method: mean = " << mean << ", first = " << first
	 << ", last = " << last << endl;

    v_polar_interleaved_to_cartesian(target, source, N);

    total = 0;

    for (int i = 0; i < N; ++i) {
	if (i == 0) first = target[i].re;
	if (i == N-1) last = target[i].im;
	total += target[i].re;
	total += target[i].im;
    }
    mean = total / (N*2);
    cerr << "v_polar_interleaved_to_cartesian: mean = " << mean << ", first = " << first
	 << ", last = " << last << endl;

    int iterations = 10000;
    cerr << "Iterations: " << iterations << endl;
	
    cerr << "CLOCKS_PER_SEC = " << CLOCKS_PER_SEC << endl;
    float divisor = float(CLOCKS_PER_SEC) / 1000.f;

    clock_t start = clock();

    for (int j = 0; j < iterations; ++j) {
	for (int i = 0; i < N; ++i) {
	    target[i].re = source[i*2] * cos(source[i*2+1]);
	    target[i].im = source[i*2] * sin(source[i*2+1]);
	}
    }
    
    clock_t end = clock();

    cerr << "Time for naive method: " << float(end - start)/divisor << endl;

    start = clock();

    for (int j = 0; j < iterations; ++j) {
	v_polar_interleaved_to_cartesian(target, source, N);
    }
    
    end = clock();

    cerr << "Time for v_polar_interleaved_to_cartesian: " << float(end - start)/divisor << endl;

    return true;
}

bool
testVectorOps()
{
    if (!testMultiply()) return false;
    if (!testPolarToCart()) return false;
    if (!testPolarToCartInterleaved()) return false;
    
    return true;
}

}

}

