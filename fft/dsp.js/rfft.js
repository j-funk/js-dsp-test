/*
 *  RFFT Originally found in DSP.js, IRFFT was on a PR of the repo and needed some fixing.
 *
 *  RFFT is a class for calculating the Discrete Fourier Transform of a signal
 *  with the Fast Fourier Transform algorithm.
 *
 *  @param {Number} bufferSize The size of the sample buffer to be computed. Must be power of 2
 *
 *  @constructor
 */

// lookup tables don't really gain us any speed, but they do increase
// cache footprint, so don't use them in here

// also we don't use separate arrays for real/imaginary parts

// this one a little more than twice as fast as the one in FFT
// however I only did the forward transform

// the rest of this was translated from C, see http://www.jjj.de/fxt/
// this is the real split radix FFT

function RFFT(bufferSize) {
  this.bufferSize = bufferSize;
  this.trans = new Float32Array(bufferSize);
  this.itrans = new Float32Array(bufferSize);

  // don't use a lookup table to do the permute, use this instead
  this._reverseBinPermute = function (dest, source) {
    var bufferSize  = this.bufferSize,
        halfSize    = bufferSize >>> 1,
        nm1         = bufferSize - 1,
        i = 1, r = 0, h;

    dest[0] = source[0];

    do {
      r += halfSize;
      dest[i] = source[r];
      dest[r] = source[i];

      i++;

      h = halfSize << 1;
      while (h = h >> 1, !((r ^= h) & h));

      if (r >= i) {
        dest[i]     = source[r];
        dest[r]     = source[i];

        dest[nm1-i] = source[nm1-r];
        dest[nm1-r] = source[nm1-i];
      }
      i++;
    } while (i < halfSize);
    dest[nm1] = source[nm1];
  };

  // don't use a lookup table to do the permute, use this instead
  // the inverse transform needs to do this in place so we have this
  this._reverseBinPermuteInPlace = function (buf) {
        var bufferSize  = this.bufferSize,
            halfSize    = bufferSize >>> 1,
            nm1         = bufferSize - 1,
            i = 1, r = 0, h, t;

        do {
            r += halfSize;
            t = buf[i];
            buf[i] = buf[r];
            buf[r] = t;

            i++;

            h = halfSize << 1;
            while (h = h >> 1, !((r ^= h) & h));

            if (r >= i) {
                t = buf[i];
                buf[i] = buf[r];
                buf[r] = t;
                t = buf[nm1-i];
                buf[nm1-i] = buf[nm1-r];
                buf[nm1-r] = t;
            }
            i++;
        } while (i < halfSize);
  };
}


// Ordering of output:
//
// trans[0]     = re[0] (==zero frequency, purely real)
// trans[1]     = re[1]
//             ...
// trans[n/2-1] = re[n/2-1]
// trans[n/2]   = re[n/2]    (==nyquist frequency, purely real)
//
// trans[n/2+1] = im[n/2-1]
// trans[n/2+2] = im[n/2-2]
//             ...
// trans[n-1]   = im[1]
//
// note before using RFFT.trans you need to scale it, however since [0-1]
// is frequently not the range you want, or you often want to work with it in
// some other way we leave it unscaled for speed (this way we make one fewer
// pass, if you're willing to remeber to scale it yourself.

RFFT.prototype.forward = function(buffer) {
  var n         = this.bufferSize,
      x         = this.trans,
      TWO_PI    = 2*Math.PI,
      sqrt      = Math.sqrt,
      i         = n >>> 1,
      bSi       = 2 / n,
      n2, n4, n8, nn,
      t1, t2, t3, t4,
      i1, i2, i3, i4, i5, i6, i7, i8,
      st1, cc1, ss1, cc3, ss3,
      e,
      a,
      rval, ival, mag;

  this._reverseBinPermute(x, buffer);

  for (var ix = 0, id = 4; ix < n; id *= 4) {
    for (var i0 = ix; i0 < n; i0 += id) {
      //sumdiff(x[i0], x[i0+1]); // {a, b}  <--| {a+b, a-b}
      st1 = x[i0] - x[i0+1];
      x[i0] += x[i0+1];
      x[i0+1] = st1;
    }
    ix = 2*(id-1);
  }

  n2 = 2;
  nn = n >>> 1;

  while((nn = nn >>> 1)) {
    ix = 0;
    n2 = n2 << 1;
    id = n2 << 1;
    n4 = n2 >>> 2;
    n8 = n2 >>> 3;
    do {
      if(n4 !== 1) {
        for(i0 = ix; i0 < n; i0 += id) {
          i1 = i0;
          i2 = i1 + n4;
          i3 = i2 + n4;
          i4 = i3 + n4;

          //diffsum3_r(x[i3], x[i4], t1); // {a, b, s} <--| {a, b-a, a+b}
          t1 = x[i3] + x[i4];
          x[i4] -= x[i3];
          //sumdiff3(x[i1], t1, x[i3]);   // {a, b, d} <--| {a+b, b, a-b}
          x[i3] = x[i1] - t1;
          x[i1] += t1;

          i1 += n8;
          i2 += n8;
          i3 += n8;
          i4 += n8;

          //sumdiff(x[i3], x[i4], t1, t2); // {s, d}  <--| {a+b, a-b}
          t1 = x[i3] + x[i4];
          t2 = x[i3] - x[i4];

          t1 = -t1 * Math.SQRT1_2;
          t2 *= Math.SQRT1_2;

          // sumdiff(t1, x[i2], x[i4], x[i3]); // {s, d}  <--| {a+b, a-b}
          st1 = x[i2];
          x[i4] = t1 + st1;
          x[i3] = t1 - st1;

          //sumdiff3(x[i1], t2, x[i2]); // {a, b, d} <--| {a+b, b, a-b}
          x[i2] = x[i1] - t2;
          x[i1] += t2;
        }
      } else {
        for(i0 = ix; i0 < n; i0 += id) {
          i1 = i0;
          i2 = i1 + n4;
          i3 = i2 + n4;
          i4 = i3 + n4;

          //diffsum3_r(x[i3], x[i4], t1); // {a, b, s} <--| {a, b-a, a+b}
          t1 = x[i3] + x[i4];
          x[i4] -= x[i3];

          //sumdiff3(x[i1], t1, x[i3]);   // {a, b, d} <--| {a+b, b, a-b}
          x[i3] = x[i1] - t1;
          x[i1] += t1;
        }
      }

      ix = (id << 1) - n2;
      id = id << 2;
    } while (ix < n);

    e = TWO_PI / n2;

    for (var j = 1; j < n8; j++) {
      a = j * e;
      ss1 = Math.sin(a);
      cc1 = Math.cos(a);

      //ss3 = sin(3*a); cc3 = cos(3*a);
      cc3 = 4*cc1*(cc1*cc1-0.75);
      ss3 = 4*ss1*(0.75-ss1*ss1);

      ix = 0; id = n2 << 1;
      do {
        for (i0 = ix; i0 < n; i0 += id) {
          i1 = i0 + j;
          i2 = i1 + n4;
          i3 = i2 + n4;
          i4 = i3 + n4;

          i5 = i0 + n4 - j;
          i6 = i5 + n4;
          i7 = i6 + n4;
          i8 = i7 + n4;

          //cmult(c, s, x, y, &u, &v)
          //cmult(cc1, ss1, x[i7], x[i3], t2, t1); // {u,v} <--| {x*c-y*s, x*s+y*c}
          t2 = x[i7]*cc1 - x[i3]*ss1;
          t1 = x[i7]*ss1 + x[i3]*cc1;

          //cmult(cc3, ss3, x[i8], x[i4], t4, t3);
          t4 = x[i8]*cc3 - x[i4]*ss3;
          t3 = x[i8]*ss3 + x[i4]*cc3;

          //sumdiff(t2, t4);   // {a, b} <--| {a+b, a-b}
          st1 = t2 - t4;
          t2 += t4;
          t4 = st1;

          //sumdiff(t2, x[i6], x[i8], x[i3]); // {s, d}  <--| {a+b, a-b}
          //st1 = x[i6]; x[i8] = t2 + st1; x[i3] = t2 - st1;
          x[i8] = t2 + x[i6];
          x[i3] = t2 - x[i6];

          //sumdiff_r(t1, t3); // {a, b} <--| {a+b, b-a}
          st1 = t3 - t1;
          t1 += t3;
          t3 = st1;

          //sumdiff(t3, x[i2], x[i4], x[i7]); // {s, d}  <--| {a+b, a-b}
          //st1 = x[i2]; x[i4] = t3 + st1; x[i7] = t3 - st1;
          x[i4] = t3 + x[i2];
          x[i7] = t3 - x[i2];

          //sumdiff3(x[i1], t1, x[i6]);   // {a, b, d} <--| {a+b, b, a-b}
          x[i6] = x[i1] - t1;
          x[i1] += t1;

          //diffsum3_r(t4, x[i5], x[i2]); // {a, b, s} <--| {a, b-a, a+b}
          x[i2] = t4 + x[i5];
          x[i5] -= t4;
        }

        ix = (id << 1) - n2;
        id = id << 2;

      } while (ix < n);
    }
  }

  return this.trans;
};


RFFT.prototype.scale_trans = function() {
    var i=0,bSi = 1.0/this.bufferSize, x = this.trans;
    while(i < x.length) { x[i] *= bSi; i++; }
};

// input must have ordering as in output of the forward version,
// you can just pass in this.trans and it should work (if you've scaled it)
RFFT.prototype.inverse = function(buffer) {
    var n         = this.bufferSize,
        x         = this.itrans,
        TWO_PI    = 2*Math.PI,
        n2, n4, n8, nn,
        t1, t2, t3, t4, t5,
        j, i0, i1, i2, i3, i4, i5, i6, i7, i8, ud, ix, id,
        st1, cc1, ss1, cc3, ss3,
        e,
        a;

    x.set(buffer);

    nn = n>>>1;
    n2 = n<<1;

    while ( nn >>>= 1 )
    {
        ix = 0;
        id = n2;
        n2 >>>= 1;
        n4 = n2>>>2;
        n8 = n4>>>1;

        do  // ix
        {
            for (i0=ix; i0<n; i0+=id)
            {
                i1 = i0;
                i2 = i1 + n4;
                i3 = i2 + n4;
                i4 = i3 + n4;

                //sumdiff3(x[i1], x[i3], t1);// {a, b, d} <--| {a+b, b, a-b}
                t1 = x[i1] - x[i3]; x[i1] += x[i3];

                x[i2] += x[i2];
                x[i4] += x[i4];

                //sumdiff3_r(x[i4], t1, x[i3]);// {a,b,d} <--| {a+b, b, b-a}
                x[i3] = t1 - x[i4]; x[i4] += t1;

                if ( n4!=1 )  // note: optimise (Note this comment from original C++)
                {
                    i1 += n8;
                    i2 += n8;
                    i3 += n8;
                    i4 += n8;

                    //sumdiff3(x[i1], x[i2], t1); // {a, b, d} <--| {a+b, b, a-b}
                    t1 = x[i1] - x[i2]; x[i1] += x[i2];

                    //sumdiff(a, b, &s, &d) {s, d}  <--| {a+b, a-b}
                    //sumdiff(x[i4], x[i3], t2, x[i2]);
                    t2 = x[i4] + x[i3]; x[i2] = x[i4] - x[i3];

                    t2 = -t2 * Math.SQRT2;
                    t1 *= Math.SQRT2;
                    //sumdiff(a, b, &s, &d) {s, d}  <--| {a+b, a-b}
                    //sumdiff(t2, t1, x[i3], x[i4]);
                    x[i3] = t2 + t1; x[i4] = t2 - t1;
                }
            }

            ix = (id<<1) - n2;
            id <<= 2;
        } while ( ix<n );

        e = TWO_PI/n2;
        for (j=1; j<n8; j++)
        {
            a = j*e;

            ss1 = Math.sin(a);
            cc1 = Math.cos(a);

            ss3 = Math.sin(3*a); cc3 = Math.cos(3*a);
            cc3 = 4*cc1*(cc1*cc1-0.75);
            ss3 = 4*ss1*(0.75-ss1*ss1);

            ix = 0;
            id = n2<<1;
            do  // ix-loop
            {
                for (i0=ix; i0<n; i0+=id)
                {
                    i1 = i0 + j;
                    i2 = i1 + n4;
                    i3 = i2 + n4;
                    i4 = i3 + n4;

                    i5 = i0 + n4 - j;
                    i6 = i5 + n4;
                    i7 = i6 + n4;
                    i8 = i7 + n4;

                    //sumdiff3(x[i1], x[i6], t1); // {a, b, d} <--| {a+b, b, a-b}
                    t1 = x[i1] - x[i6]; x[i1] += x[i6];
                    //sumdiff3(x[i5], x[i2], t2); // {a, b, d} <--| {a+b, b, a-b}
                    t2 = x[i5] - x[i2]; x[i5] += x[i2];
                    //t2 = x[i5] + x[i2]; x[i5] = x[i5] - x[i2];

                    //sumdiff(a, b, &s, &d) {s, d}  <--| {a+b, a-b}
                    //sumdiff(x[i8], x[i3], t3, x[i6]);
                    //sumdiff(x[i4], x[i7], t4, x[i2]);
                    t3 = x[i8] + x[i3]; x[i6] = x[i8] - x[i3];
                    t4 = x[i4] + x[i7]; x[i2] = x[i4] - x[i7];

                    //sumdiff3(t1, t4, t5); // {a, b, d} <--| {a+b, b, a-b}
                    t5 = t1 - t4; t1 += t4;
                    //sumdiff3(t2, t3, t4); // {a, b, d} <--| {a+b, b, a-b}
                    t4 = t2 - t3; t2 += t3;

                    //cmult(c, s, x, y, &u, &v) {u,v} <--| {x*c-y*s, x*s+y*c}
                    //cmult(ss1, cc1, t5, t4, x[i7], x[i3]);
                    //cmult(cc3, ss3, t1, t2, x[i4], x[i8]);
                    x[i7] = t5*ss1 - t4*cc1; x[i3] = t5*cc1 + t4*ss1;
                    x[i4] = t1*cc3 - t2*ss3; x[i8] = t1*ss3 + t2*cc3;
                }

                ix = (id<<1) - n2;
                id <<= 2;
            } while ( ix < n );
        }
    }

    for (ix=0, id=4;  ix<n;  id*=4)
    {
        for (i0=ix; i0<n; i0+=id) {
            // sumdiff(&a, &b) {a, b}  <--| {a+b, a-b}
            //sumdiff(x[i0], x[i0+1]);
            st1 = x[i0] - x[i0+1]; x[i0] += x[i0+1]; x[i0+1] = st1;
        }
        ix = 2*(id-1);
    }

    this._reverseBinPermuteInPlace(x);
    return x;
};
