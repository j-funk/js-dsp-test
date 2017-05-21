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

#ifndef BQVEC_ALLOCATORS_H
#define BQVEC_ALLOCATORS_H

/*
 * Aligned and per-channel allocators and deallocators for raw C array
 * buffers.
 */

#include <new> // for std::bad_alloc
#include <stdlib.h>

#ifndef HAVE_POSIX_MEMALIGN
#ifndef _WIN32
#ifndef __APPLE__
#ifndef LACK_POSIX_MEMALIGN
#define HAVE_POSIX_MEMALIGN
#endif
#endif
#endif
#endif

#ifndef MALLOC_IS_NOT_ALIGNED
#ifdef __APPLE_
#ifndef MALLOC_IS_ALIGNED
#define MALLOC_IS_ALIGNED
#endif
#endif
#endif

#ifdef HAVE_POSIX_MEMALIGN
#include <sys/mman.h>
#endif

#ifdef LACK_BAD_ALLOC
namespace std { struct bad_alloc { }; }
#endif

namespace breakfastquay {

template <typename T>
T *allocate(size_t count)
{
    void *ptr = 0;
    // 32-byte alignment is required for at least OpenMAX
    static const int alignment = 32;
#ifdef USE_OWN_ALIGNED_MALLOC
    // Alignment must be a power of two, bigger than the pointer
    // size. Stuff the actual malloc'd pointer in just before the
    // returned value.  This is the least desirable way to do this --
    // the other options below are all better
    size_t allocd = count * sizeof(T) + alignment;
    void *buf = malloc(allocd);
    if (buf) {
        char *adj = (char *)buf;
        while ((unsigned long long)adj & (alignment-1)) --adj;
        ptr = ((char *)adj) + alignment;
        ((void **)ptr)[-1] = buf;
    }
#else /* !USE_OWN_ALIGNED_MALLOC */
#ifdef HAVE_POSIX_MEMALIGN
    if (posix_memalign(&ptr, alignment, count * sizeof(T))) {
        ptr = malloc(count * sizeof(T));
    }
#else /* !HAVE_POSIX_MEMALIGN */
#ifdef __MSVC__
    ptr = _aligned_malloc(count * sizeof(T), alignment);
#else /* !__MSVC__ */
#ifndef MALLOC_IS_ALIGNED
#error "No aligned malloc available: define MALLOC_IS_ALIGNED to stick with system malloc, HAVE_POSIX_MEMALIGN if posix_memalign is available, or USE_OWN_ALIGNED_MALLOC to roll our own"
#endif
    // Note that malloc always aligns to 16 byte boundaries on OS/X
    ptr = malloc(count * sizeof(T));
    (void)alignment; // avoid compiler warning for unused 
#endif /* !__MSVC__ */
#endif /* !HAVE_POSIX_MEMALIGN */
#endif /* !USE_OWN_ALIGNED_MALLOC */
    if (!ptr) {
#ifndef NO_EXCEPTIONS
        throw(std::bad_alloc());
#else
        abort();
#endif
    }
    return (T *)ptr;
}

#ifdef HAVE_IPP

template <>
float *allocate(size_t count);

template <>
double *allocate(size_t count);

#endif
	
template <typename T>
T *allocate_and_zero(size_t count)
{
    T *ptr = allocate<T>(count);
    for (size_t i = 0; i < count; ++i) {
        ptr[i] = T();
    }
    return ptr;
}

template <typename T>
void deallocate(T *ptr)
{
#ifdef USE_OWN_ALIGNED_MALLOC
    if (ptr) free(((void **)ptr)[-1]);
#else /* !USE_OWN_ALIGNED_MALLOC */
#ifdef __MSVC__
    if (ptr) _aligned_free((void *)ptr);
#else /* !__MSVC__ */
    if (ptr) free((void *)ptr);
#endif /* !__MSVC__ */
#endif /* !USE_OWN_ALIGNED_MALLOC */
}

#ifdef HAVE_IPP

template <>
void deallocate(float *);

template <>
void deallocate(double *);

#endif

/// Reallocate preserving contents but leaving additional memory uninitialised	
template <typename T>
T *reallocate(T *ptr, size_t oldcount, size_t count)
{
    T *newptr = allocate<T>(count);
    if (oldcount && ptr) {
        for (size_t i = 0; i < oldcount && i < count; ++i) {
            newptr[i] = ptr[i];
        }
    }
    if (ptr) deallocate<T>(ptr);
    return newptr;
}

/// Reallocate, zeroing all contents
template <typename T>
T *reallocate_and_zero(T *ptr, size_t oldcount, size_t count)
{
    ptr = reallocate(ptr, oldcount, count);
    for (size_t i = 0; i < count; ++i) {
        ptr[i] = T();
    }
    return ptr;
}
	
/// Reallocate preserving contents and zeroing any additional memory	
template <typename T>
T *reallocate_and_zero_extension(T *ptr, size_t oldcount, size_t count)
{
    ptr = reallocate(ptr, oldcount, count);
    if (count > oldcount) {
        for (size_t i = oldcount; i < count; ++i) {
            ptr[i] = T();
        }
    }
    return ptr;
}

template <typename T>
T **allocate_channels(size_t channels, size_t count)
{
    // We don't want to use the aligned allocate for the channel
    // pointers, it might even make things slower
    T **ptr = new T *[channels];
    for (size_t c = 0; c < channels; ++c) {
        ptr[c] = allocate<T>(count);
    }
    return ptr;
}
	
template <typename T>
T **allocate_and_zero_channels(size_t channels, size_t count)
{
    // We don't want to use the aligned allocate for the channel
    // pointers, it might even make things slower
    T **ptr = new T *[channels];
    for (size_t c = 0; c < channels; ++c) {
        ptr[c] = allocate_and_zero<T>(count);
    }
    return ptr;
}

template <typename T>
void deallocate_channels(T **ptr, size_t channels)
{
    if (!ptr) return;
    for (size_t c = 0; c < channels; ++c) {
        deallocate<T>(ptr[c]);
    }
    delete[] ptr;
}
	
template <typename T>
T **reallocate_channels(T **ptr,
                        size_t oldchannels, size_t oldcount,
                        size_t channels, size_t count)
{
    T **newptr = allocate_channels<T>(channels, count);
    if (oldcount && ptr) {
        for (size_t c = 0; c < channels; ++c) {
            for (size_t i = 0; i < oldcount && i < count; ++i) {
                newptr[c][i] = ptr[c][i];
            }
        }
    } 
    if (ptr) deallocate_channels<T>(ptr, oldchannels);
    return newptr;
}
	
template <typename T>
T **reallocate_and_zero_extend_channels(T **ptr,
                                        size_t oldchannels, size_t oldcount,
                                        size_t channels, size_t count)
{
    T **newptr = allocate_and_zero_channels<T>(channels, count);
    if (oldcount && ptr) {
        for (size_t c = 0; c < channels; ++c) {
            for (size_t i = 0; i < oldcount && i < count; ++i) {
                newptr[c][i] = ptr[c][i];
            }
        }
    } 
    if (ptr) deallocate_channels<T>(ptr, oldchannels);
    return newptr;
}

/// RAII class to call deallocate() on destruction
template <typename T>
class Deallocator
{
public:
    Deallocator(T *t) : m_t(t) { }
    ~Deallocator() { deallocate<T>(m_t); }
private:
    T *m_t;
};

}

#endif

