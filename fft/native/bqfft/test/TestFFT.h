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

#ifndef TEST_FFT_H
#define TEST_FFT_H

#include "bqfft/FFT.h"

#include <QObject>
#include <QtTest>

#include <cstdio>

#include "Compares.h"

namespace breakfastquay {

class TestFFT : public QObject
{
    Q_OBJECT

private:
    void idat() {
        QTest::addColumn<QString>("implementation");
        std::set<std::string> impls = FFT::getImplementations();
        foreach (std::string i, impls) {
            QTest::newRow(i.c_str()) << i.c_str();
        }
    }
    QString ifetch() {
        QFETCH(QString, implementation);
        FFT::setDefaultImplementation(implementation.toLocal8Bit().data());
        return implementation;
    }

    bool lackSingle() {
        return !(FFT(4).getSupportedPrecisions() & FFT::SinglePrecision);
    }
    bool lackDouble() {
        return !(FFT(4).getSupportedPrecisions() & FFT::DoublePrecision);
    }

private slots:

    void checkD() {
        QString impl = ifetch();
    }

    void dc() {
        ifetch();
	// DC-only signal. The DC bin is purely real
	double in[] = { 1, 1, 1, 1 };
	double re[3], im[3];
	FFT(4).forward(in, re, im);
	QCOMPARE(re[0], 4.0);
	COMPARE_ZERO(re[1]);
	COMPARE_ZERO(re[2]);
	COMPARE_ALL(im, 0.0);
	double back[4];
        FFT(4).inverse(re, im, back);
	COMPARE_SCALED(back, in, 4);
    }

    void sine() {
        ifetch();
	// Sine. Output is purely imaginary
	double in[] = { 0, 1, 0, -1 };
	double re[3], im[3];
	FFT(4).forward(in, re, im);
	COMPARE_ALL(re, 0.0);
	COMPARE_ZERO(im[0]);
	QCOMPARE(im[1], -2.0);
	COMPARE_ZERO(im[2]);
	double back[4];
        FFT(4).inverse(re, im, back);
	COMPARE_SCALED(back, in, 4);
    }

    void cosine() {
        ifetch();
	// Cosine. Output is purely real
	double in[] = { 1, 0, -1, 0 };
	double re[3], im[3];
	FFT(4).forward(in, re, im);
	COMPARE_ZERO(re[0]);
	QCOMPARE(re[1], 2.0);
	COMPARE_ZERO(re[2]);
	COMPARE_ALL(im, 0.0);
	double back[4];
        FFT(4).inverse(re, im, back);
	COMPARE_SCALED(back, in, 4);
    }
	
    void sineCosine() {
        ifetch();
	// Sine and cosine mixed
	double in[] = { 0.5, 1, -0.5, -1 };
	double re[3], im[3];
	FFT(4).forward(in, re, im);
	COMPARE_ZERO(re[0]);
	QCOMPARE(re[1], 1.0);
	COMPARE_ZERO(re[2]);
	COMPARE_ZERO(im[0]);
	QCOMPARE(im[1], -2.0);
	COMPARE_ZERO(im[2]);
	double back[4];
        FFT(4).inverse(re, im, back);
	COMPARE_SCALED(back, in, 4);
    }

    void nyquist() {
        ifetch();
	double in[] = { 1, -1, 1, -1 };
	double re[3], im[3];
	FFT(4).forward(in, re, im);
	COMPARE_ZERO(re[0]);
	COMPARE_ZERO(re[1]);
	QCOMPARE(re[2], 4.0);
	COMPARE_ALL(im, 0.0);
	double back[4];
        FFT(4).inverse(re, im, back);
	COMPARE_SCALED(back, in, 4);
    }
	
    void interleaved() {
        ifetch();
	// Sine and cosine mixed, test output format
	double in[] = { 0.5, 1, -0.5, -1 };
	double out[6];
	FFT(4).forwardInterleaved(in, out);
	COMPARE_ZERO(out[0]);
	COMPARE_ZERO(out[1]);
	QCOMPARE(out[2], 1.0);
	QCOMPARE(out[3], -2.0);
	COMPARE_ZERO(out[4]);
	COMPARE_ZERO(out[5]);
	double back[4];
        FFT(4).inverseInterleaved(out, back);
	COMPARE_SCALED(back, in, 4);
    }

    void cosinePolar() {
        ifetch();
	double in[] = { 1, 0, -1, 0 };
	double mag[3], phase[3];
	FFT(4).forwardPolar(in, mag, phase);
	COMPARE_ZERO(mag[0]);
	QCOMPARE(mag[1], 2.0);
	COMPARE_ZERO(mag[2]);
        // No meaningful tests for phase[i] where mag[i]==0 (phase
        // could legitimately be anything)
	COMPARE_ZERO(phase[1]);
	double back[4];
        FFT(4).inversePolar(mag, phase, back);
	COMPARE_SCALED(back, in, 4);
    }

    void sinePolar() {
        ifetch();
	double in[] = { 0, 1, 0, -1 };
	double mag[3], phase[3];
	FFT(4).forwardPolar(in, mag, phase);
	COMPARE_ZERO(mag[0]);
	QCOMPARE(mag[1], 2.0);
	COMPARE_ZERO(mag[2]);
        // No meaningful tests for phase[i] where mag[i]==0 (phase
        // could legitimately be anything)
	QCOMPARE(phase[1], -M_PI/2.0);
	double back[4];
        FFT(4).inversePolar(mag, phase, back);
	COMPARE_SCALED(back, in, 4);
    }

    void magnitude() {
        ifetch();
	// Sine and cosine mixed
	double in[] = { 0.5, 1, -0.5, -1 };
	double out[3];
	FFT(4).forwardMagnitude(in, out);
	COMPARE_ZERO(out[0]);
	QCOMPARE(float(out[1]), sqrtf(5.0));
	COMPARE_ZERO(out[2]);
    }

    void dirac() {
        ifetch();
	double in[] = { 1, 0, 0, 0 };
	double re[3], im[3];
	FFT(4).forward(in, re, im);
	QCOMPARE(re[0], 1.0);
	QCOMPARE(re[1], 1.0);
	QCOMPARE(re[2], 1.0);
	COMPARE_ALL(im, 0.0);
	double back[4];
        FFT(4).inverse(re, im, back);
	COMPARE_SCALED(back, in, 4);
    }

    void cepstrum() {
        ifetch();
	double in[] = { 1, 0, 0, 0, 1, 0, 0, 0 };
	double mag[5];
	FFT(8).forwardMagnitude(in, mag);
	double cep[8];
	FFT(8).inverseCepstral(mag, cep);
	COMPARE_ZERO(cep[1]);
	COMPARE_ZERO(cep[2]);
	COMPARE_ZERO(cep[3]);
	COMPARE_ZERO(cep[5]);
	COMPARE_ZERO(cep[6]);
	COMPARE_ZERO(cep[7]);
	QVERIFY(fabs(-6.561181 - cep[0]/8) < 0.000001);
	QVERIFY(fabs( 7.254329 - cep[4]/8) < 0.000001);
    }

    void forwardArrayBounds() {
        ifetch();
	// initialise bins to something recognisable, so we can tell
	// if they haven't been written
	double in[] = { 1, 1, -1, -1 };
	double re[] = { 999, 999, 999, 999, 999 };
	double im[] = { 999, 999, 999, 999, 999 };
	FFT(4).forward(in, re+1, im+1);
	// And check we haven't overrun the arrays
	QCOMPARE(re[0], 999.0);
	QCOMPARE(im[0], 999.0);
	QCOMPARE(re[4], 999.0);
	QCOMPARE(im[4], 999.0);
    }

    void inverseArrayBounds() {
        ifetch();
	// initialise bins to something recognisable, so we can tell
	// if they haven't been written
	double re[] = { 0, 1, 0 };
	double im[] = { 0, -2, 0 };
	double out[] = { 999, 999, 999, 999, 999, 999 };
	FFT(4).inverse(re, im, out+1);
	// And check we haven't overrun the arrays
	QCOMPARE(out[0], 999.0);
	QCOMPARE(out[5], 999.0);
    }

    void checkF() {
        QString impl = ifetch();
    }

    void dcF() {
        ifetch();
	// DC-only signal. The DC bin is purely real
	float in[] = { 1, 1, 1, 1 };
	float re[3], im[3];
	FFT(4).forward(in, re, im);
	QCOMPARE(re[0], 4.0f);
	COMPARE_ZERO_F(re[1]);
	COMPARE_ZERO_F(re[2]);
	COMPARE_ALL_F(im, 0.0f);
	float back[4];
        FFT(4).inverse(re, im, back);
	COMPARE_SCALED_F(back, in, 4);
    }

    void sineF() {
        ifetch();
	// Sine. Output is purely imaginary
	float in[] = { 0, 1, 0, -1 };
	float re[3], im[3];
	FFT(4).forward(in, re, im);
	COMPARE_ALL_F(re, 0.0f);
	COMPARE_ZERO_F(im[0]);
	QCOMPARE(im[1], -2.0f);
	COMPARE_ZERO_F(im[2]);
	float back[4];
        FFT(4).inverse(re, im, back);
	COMPARE_SCALED_F(back, in, 4);
    }

    void cosineF() {
        ifetch();
	// Cosine. Output is purely real
	float in[] = { 1, 0, -1, 0 };
	float re[3], im[3];
	FFT(4).forward(in, re, im);
	COMPARE_ZERO_F(re[0]);
	QCOMPARE(re[1], 2.0f);
	COMPARE_ZERO_F(re[2]);
	COMPARE_ALL_F(im, 0.0f);
	float back[4];
        FFT(4).inverse(re, im, back);
	COMPARE_SCALED_F(back, in, 4);
    }
	
    void sineCosineF() {
        ifetch();
	// Sine and cosine mixed
	float in[] = { 0.5, 1, -0.5, -1 };
	float re[3], im[3];
	FFT(4).forward(in, re, im);
	COMPARE_ZERO_F(re[0]);
	QCOMPARE(re[1], 1.0f);
	COMPARE_ZERO_F(re[2]);
	COMPARE_ZERO_F(im[0]);
	QCOMPARE(im[1], -2.0f);
	COMPARE_ZERO_F(im[2]);
	float back[4];
        FFT(4).inverse(re, im, back);
	COMPARE_SCALED_F(back, in, 4);
    }

    void nyquistF() {
        ifetch();
	float in[] = { 1, -1, 1, -1 };
	float re[3], im[3];
	FFT(4).forward(in, re, im);
	COMPARE_ZERO_F(re[0]);
	COMPARE_ZERO_F(re[1]);
	QCOMPARE(re[2], 4.0f);
	COMPARE_ALL_F(im, 0.0f);
	float back[4];
        FFT(4).inverse(re, im, back);
	COMPARE_SCALED_F(back, in, 4);
    }
	
    void interleavedF() {
        ifetch();
	// Sine and cosine mixed, test output format
	float in[] = { 0.5, 1, -0.5, -1 };
	float out[6];
	FFT(4).forwardInterleaved(in, out);
	COMPARE_ZERO_F(out[0]);
	COMPARE_ZERO_F(out[1]);
	QCOMPARE(out[2], 1.0f);
	QCOMPARE(out[3], -2.0f);
	COMPARE_ZERO_F(out[4]);
	COMPARE_ZERO_F(out[5]);
	float back[4];
        FFT(4).inverseInterleaved(out, back);
	COMPARE_SCALED_F(back, in, 4);
    }

    void cosinePolarF() {
        ifetch();
	float in[] = { 1, 0, -1, 0 };
	float mag[3], phase[3];
	FFT(4).forwardPolar(in, mag, phase);
	COMPARE_ZERO_F(mag[0]);
	QCOMPARE(mag[1], 2.0f);
	COMPARE_ZERO_F(mag[2]);
        // No meaningful tests for phase[i] where mag[i]==0 (phase
        // could legitimately be anything)
	COMPARE_ZERO_F(phase[1]);
	float back[4];
        FFT(4).inversePolar(mag, phase, back);
	COMPARE_SCALED_F(back, in, 4);
    }

    void sinePolarF() {
        ifetch();
	float in[] = { 0, 1, 0, -1 };
	float mag[3], phase[3];
	FFT(4).forwardPolar(in, mag, phase);
	COMPARE_ZERO_F(mag[0]);
	QCOMPARE(mag[1], 2.0f);
	COMPARE_ZERO_F(mag[2]);
        // No meaningful tests for phase[i] where mag[i]==0 (phase
        // could legitimately be anything)
	QCOMPARE(phase[1], -float(M_PI)/2.0f);
	float back[4];
        FFT(4).inversePolar(mag, phase, back);
	COMPARE_SCALED_F(back, in, 4);
    }

    void magnitudeF() {
        ifetch();
	// Sine and cosine mixed
	float in[] = { 0.5, 1, -0.5, -1 };
	float out[3];
	FFT(4).forwardMagnitude(in, out);
	COMPARE_ZERO_F(out[0]);
	QCOMPARE(float(out[1]), sqrtf(5.0f));
	COMPARE_ZERO_F(out[2]);
    }

    void diracF() {
        ifetch();
	float in[] = { 1, 0, 0, 0 };
	float re[3], im[3];
	FFT(4).forward(in, re, im);
	QCOMPARE(re[0], 1.0f);
	QCOMPARE(re[1], 1.0f);
	QCOMPARE(re[2], 1.0f);
	COMPARE_ALL_F(im, 0.0f);
	float back[4];
        FFT(4).inverse(re, im, back);
	COMPARE_SCALED_F(back, in, 4);
    }

    void cepstrumF() {
        ifetch();
	float in[] = { 1, 0, 0, 0, 1, 0, 0, 0 };
	float mag[5];
	FFT(8).forwardMagnitude(in, mag);
	float cep[8];
	FFT(8).inverseCepstral(mag, cep);
	COMPARE_ZERO_F(cep[1]);
	COMPARE_ZERO_F(cep[2]);
	COMPARE_ZERO_F(cep[3]);
	COMPARE_ZERO_F(cep[5]);
	COMPARE_ZERO_F(cep[6]);
	COMPARE_ZERO_F(cep[7]);
	QVERIFY(fabsf(-6.561181 - cep[0]/8) < 0.000001);
	QVERIFY(fabsf( 7.254329 - cep[4]/8) < 0.000001);
    }

    void forwardArrayBoundsF() {
        ifetch();
	// initialise bins to something recognisable, so we can tell
	// if they haven't been written
	float in[] = { 1, 1, -1, -1 };
	float re[] = { 999, 999, 999, 999, 999 };
	float im[] = { 999, 999, 999, 999, 999 };
	FFT(4).forward(in, re+1, im+1);
	// And check we haven't overrun the arrays
	QCOMPARE(re[0], 999.0f);
	QCOMPARE(im[0], 999.0f);
	QCOMPARE(re[4], 999.0f);
	QCOMPARE(im[4], 999.0f);
    }

    void inverseArrayBoundsF() {
        ifetch();
	// initialise bins to something recognisable, so we can tell
	// if they haven't been written
	float re[] = { 0, 1, 0 };
	float im[] = { 0, -2, 0 };
	float out[] = { 999, 999, 999, 999, 999, 999 };
	FFT(4).inverse(re, im, out+1);
	// And check we haven't overrun the arrays
	QCOMPARE(out[0], 999.0f);
	QCOMPARE(out[5], 999.0f);
    }

    void checkD_data() { idat(); }
    void dc_data() { idat(); }
    void sine_data() { idat(); }
    void cosine_data() { idat(); }
    void sineCosine_data() { idat(); }
    void sineCosineDC_data() { idat(); }
    void nyquist_data() { idat(); }
    void interleaved_data() { idat(); }
    void cosinePolar_data() { idat(); }
    void sinePolar_data() { idat(); }
    void magnitude_data() { idat(); }
    void dirac_data() { idat(); }
    void cepstrum_data() { idat(); }
    void forwardArrayBounds_data() { idat(); }
    void inverseArrayBounds_data() { idat(); }

    void checkF_data() { idat(); }
    void dcF_data() { idat(); }
    void sineF_data() { idat(); }
    void cosineF_data() { idat(); }
    void sineCosineF_data() { idat(); }
    void sineCosineDCF_data() { idat(); }
    void nyquistF_data() { idat(); }
    void interleavedF_data() { idat(); }
    void cosinePolarF_data() { idat(); }
    void sinePolarF_data() { idat(); }
    void magnitudeF_data() { idat(); }
    void diracF_data() { idat(); }
    void cepstrumF_data() { idat(); }
    void forwardArrayBoundsF_data() { idat(); }
    void inverseArrayBoundsF_data() { idat(); }
};

}

#endif
