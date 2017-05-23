
/* Utility functions to generate arbitrary input in various formats */

function inputReals(size) {
    var result = new Float32Array(size);
    for (var i = 0; i < result.length; i++)
	result[i] = (i % 2) / 4.0;
    return result;
}

function zeroReals(size) {
    var result = new Float32Array(size);
    for (var i = 0; i < result.length; i++)
	result[i] = 0.0;
    return result;
}

function inputInterleaved(size) {
    var result = new Float32Array(size*2);
    for (var i = 0; i < size; i++)
	result[i*2] = (i % 2) / 4.0;
    return result;
}

function inputReal64s(size) {
    var result = new Float64Array(size);
    for (var i = 0; i < result.length; i++)
	result[i] = (i % 2) / 4.0;
    return result;
}

function zeroReal64s(size) {
    var result = new Float64Array(size);
    for (var i = 0; i < result.length; i++)
	result[i] = 0.0;
    return result;
}

function inputComplexArray(size) {
    var result = new complex_array.ComplexArray(size);
    for (var i = 0; i < size; i++) {
	result.real[i] = (i % 2) / 4.0;
	result.imag[i] = 0.0;
    }
    return result;
}

var iterations = 2000;

function report(name, start, middle, end, total) {
    function addTo(tag, thing) {
	document.getElementById(name + "-" + tag).innerHTML += thing + "<br>";
    }
    addTo("result", total);
    addTo("1", Math.round(middle - start) + " ms");
    addTo("2", Math.round(end - middle) + " ms");
    addTo("itr", Math.round((1000.0 /
			     ((end - middle) / iterations))) + " itr/sec");
}

function testNayuki(size) {

    var start = performance.now();
    var middle = start;
    var end = start;

    var total = 0.0;

    for (var i = 0; i < 2*iterations; ++i) {
	if (i == iterations) {
	    middle = performance.now();
	}
	var real = inputReals(size);
	var imag = zeroReals(size);
	transform(real, imag);
	for (var j = 0; j < size; ++j) {
	    total += Math.sqrt(real[j] * real[j] + imag[j] * imag[j]);
	}
    }

    var end = performance.now();

    report("nayuki", start, middle, end, total);
}

function testNayukiObj(size) {

    var fft = new FFTNayuki(size);
    
    var start = performance.now();
    var middle = start;
    var end = start;

    var total = 0.0;

    for (var i = 0; i < 2*iterations; ++i) {
	if (i == iterations) {
	    middle = performance.now();
	}
	var real = inputReals(size);
	var imag = zeroReals(size);
	fft.forward(real, imag);
	for (var j = 0; j < size; ++j) {
	    total += Math.sqrt(real[j] * real[j] + imag[j] * imag[j]);
	}
    }

    var end = performance.now();

    report("nayukiobj", start, middle, end, total);
}

function testNayukiC(size) {

    var fft = new FFTNayukiC(size);
    
    var start = performance.now();
    var middle = start;
    var end = start;

    total = 0.0;

    for (var i = 0; i < 2*iterations; ++i) {
	if (i == iterations) {
	    middle = performance.now();
	}
	var real = inputReal64s(size);
	var imag = zeroReal64s(size);
	fft.forward(real, imag);
	for (var j = 0; j < size; ++j) {
	    total += Math.sqrt(real[j] * real[j] + imag[j] * imag[j]);
	}
    }
    
    var end = performance.now();

    fft.dispose();
    
    report("nayukic", start, middle, end, total);
}

function testNayukiCF(size) {

    var fft = new FFTNayukiCFloat(size);
    
    var start = performance.now();
    var middle = start;
    var end = start;

    total = 0.0;

    for (var i = 0; i < 2*iterations; ++i) {
	if (i == iterations) {
	    middle = performance.now();
	}
	var real = inputReals(size);
	var imag = zeroReals(size);
	fft.forward(real, imag);
	for (var j = 0; j < size; ++j) {
	    total += Math.sqrt(real[j] * real[j] + imag[j] * imag[j]);
	}
    }
    
    var end = performance.now();

    fft.dispose();
    
    report("nayukicf", start, middle, end, total);
}

function testNockert(size) {
    
    var fft = new FFT.complex(size, false);
    
    var start = performance.now();
    var middle = start;
    var end = start;

    total = 0.0;

    for (var i = 0; i < 2*iterations; ++i) {
	if (i == iterations) {
	    middle = performance.now();
	}
	var ri = inputReal64s(size);
	var co = new Float64Array(2 * size);
	fft.simple(co, ri, 'real');
	for (var j = 0; j < size; ++j) {
	    total += Math.sqrt(co[j*2] * co[j*2] + co[j*2+1] * co[j*2+1]);
	}
    }

    var end = performance.now();

    report("nockert", start, middle, end, total);
}

function testDntj(size) {

    var start = performance.now();
    var middle = start;
    var end = start;

    total = 0.0;
    var scale = Math.sqrt(size);

    for (var i = 0; i < 2*iterations; ++i) {
	if (i == iterations) {
	    middle = performance.now();
	}
	var ci = inputComplexArray(size);
	var co = ci.FFT();
	for (var j = 0; j < size; ++j) {
	    total += scale *
		Math.sqrt(co.real[j] * co.real[j] + co.imag[j] * co.imag[j]);
	}
    }

    var end = performance.now();

    report("dntj", start, middle, end, total);
}

function testDSPJs(size) {

    var fft = new RFFT(size);
    var start = performance.now();
    var middle = start;
    var end = start;

    total = 0.0;

    for (var i = 0; i < 2*iterations; ++i) {
        if (i == iterations) {
            middle = performance.now();
        }
        var ri = inputReals(size);
        var trans = fft.forward(ri);
        for (var j = 0; j < size / 2; ++j) {
            total += Math.sqrt(trans[j] * trans[j] + trans[j + (size / 2) - 1] * trans[j + (size / 2) - 1]);
        }
    }

    var end = performance.now();

    report("dspjs", start, middle, end, total);
}

function testCross(size) {

    var fft = new FFTCross(size);
    
    var start = performance.now();
    var middle = start;
    var end = start;

    total = 0.0;

    for (var i = 0; i < 2*iterations; ++i) {
	if (i == iterations) {
	    middle = performance.now();
	}
	var ri = inputReal64s(size);
	var out = fft.transformReal(ri, false);
	for (var j = 0; j < size; ++j) {
	    total += 
		Math.sqrt(out.real[j] * out.real[j] + out.imag[j] * out.imag[j]);
	}
    }

    var end = performance.now();
    
    report("cross", start, middle, end, total);

    fft.dispose();
}

function testKissFFT(size) {

    var fft = new KissFFTR(size);
    
    var start = performance.now();
    var middle = start;
    var end = start;

    total = 0.0;

    for (var i = 0; i < 2*iterations; ++i) {
	if (i == iterations) {
	    middle = performance.now();
	}
	var ri = inputReals(size);
	var out = fft.forward(ri);
	for (var j = 0; j <= size/2; ++j) {
	    total += Math.sqrt(out[j*2] * out[j*2] + out[j*2+1] * out[j*2+1]);
	}
	// KissFFTR returns only the first half of the output (plus
	// DC/Nyquist) -- synthesise the conjugate half
	for (var j = 1; j < size/2; ++j) {
	    total += Math.sqrt(out[j*2] * out[j*2] + out[j*2+1] * out[j*2+1]);
	}
    }

    var end = performance.now();
    
    report("kissfft", start, middle, end, total);

    fft.dispose();
}

function testKissFFTCC(size) {

    var fft = new KissFFT(size);
    
    var start = performance.now();
    var middle = start;
    var end = start;

    total = 0.0;

    for (var i = 0; i < 2*iterations; ++i) {
	if (i == iterations) {
	    middle = performance.now();
	}
	var cin = inputInterleaved(size);
	var out = fft.forward(cin);
	for (var j = 0; j < size; ++j) {
	    total += Math.sqrt(out[j*2] * out[j*2] + out[j*2+1] * out[j*2+1]);
	}
    }

    var end = performance.now();
    
    report("kissfftcc", start, middle, end, total);

    fft.dispose();
}

function testFFTW(size) {

    var fft = new FFTW(size);
    
    var start = performance.now();
    var middle = start;
    var end = start;

    total = 0.0;

    for (var i = 0; i < 2*iterations; ++i) {
	if (i == iterations) {
	    middle = performance.now();
	}
	var ri = inputReals(size);
	var out = fft.forward(ri);
	for (var j = 0; j <= size/2; ++j) {
	    total += Math.sqrt(out[j*2] * out[j*2] + out[j*2+1] * out[j*2+1]);
	}
	// FFTW returns only the first half of the output (plus
	// DC/Nyquist) -- synthesise the conjugate half
	for (var j = 1; j < size/2; ++j) {
	    total += Math.sqrt(out[j*2] * out[j*2] + out[j*2+1] * out[j*2+1]);
	}
    }

    var end = performance.now();
    
    report("fftw", start, middle, end, total);

    fft.dispose();
}

var sizes = [ 512, 2048 ];
var tests = [ testNayuki, testNayukiObj, testNayukiC, testNayukiCF,
	      testKissFFT, testKissFFTCC, testDSPJs, testCross, testFFTW,
	      testNockert, testDntj ];
var nextTest = 0;
var nextSize = 0;
var interval;

function test() {
    clearInterval(interval);
    if (nextTest == tests.length) {
	nextSize++;
	nextTest = 0;
	if (nextSize == sizes.length) {
	    return;
	}
    }
    f = tests[nextTest];
    size = sizes[nextSize];
    nextTest++;
    f(size);
    interval = setInterval(test, 100);
}

window.onload = function() {
    document.getElementById("test-description").innerHTML =
	"Running " + 2*iterations + " iterations per implementation.<br>Timings are given separately for the first half of the run (" + iterations + " iterations) and the second half, in case the JS engine takes some warming up.<br>Each cell contains results for the following sizes: " + sizes;
    interval = setInterval(test, 100);
}

