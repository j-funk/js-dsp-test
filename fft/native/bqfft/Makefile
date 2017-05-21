
# Add to FFT_DEFINES the relevant options for your desired third-party
# library support.
#
# Available options are
#
#  -DHAVE_IPP         Intel's Integrated Performance Primitives are available
#  -DHAVE_VDSP        Apple's Accelerate framework is available
#  -DHAVE_FFTW3       The FFTW library is available
#  -DHAVE_KISSFFT     The KissFFT library is available
#  -DHAVE_MEDIALIB    The Medialib library (from Sun) is available
#  -DHAVE_OPENMAX     The OpenMAX signal processing library is available
#  -DUSE_BUILTIN_FFT  Compile the built-in FFT code (which is very slow)
#
# You may define more than one of these. If you define
# USE_BUILTIN_FFT, the code will be compiled in but will only be used
# if no other option is available. The default, if no flags are
# supplied, is for the code to refuse to compile.
# 
# Add any relevant -I flags for include paths as well.
#
# Note that you must supply the same flags when including bqfft
# headers later as you are using now when compiling the library. (You
# may find it simplest to just add the bqfft source files to your
# application's build system and not build a bqfft library at all.)

# WARNING! The default option here is VERY SLOW! Read above for better
# alternatives!
FFT_DEFINES	:= -DHAVE_VDSP


# Add to ALLOCATOR_DEFINES options relating to aligned malloc.
#
# Available options are
#
#  -DHAVE_POSIX_MEMALIGN     The posix_memalign call is available in sys/mman.h
#  -DLACK_POSIX_MEMALIGN     The posix_memalign call is not available
#
#  -DMALLOC_IS_ALIGNED       The malloc call already returns aligned memory
#  -DMALLOC_IS_NOT_ALIGNED   The malloc call does not return aligned memory
#
#  -DUSE_OWN_ALIGNED_MALLOC  No aligned malloc is available, roll your own
#
#  -DLACK_BAD_ALLOC          The C++ library lacks the std::bad_alloc exception
#
# Here "aligned" is assumed to mean "aligned enough for whatever
# vector stuff the space will be used for" which most likely means
# 16-byte alignment.
#
# The default is to use _aligned_malloc when building with Visual C++,
# system malloc when building on OS/X, and posix_memalign otherwise.
#
# Note that you must supply the same flags when including bqfft
# headers later as you are using now when compiling the library. (You
# may find it simplest to just add the bqfft source files to your
# application's build system and not build a bqfft library at all.)

ALLOCATOR_DEFINES := -DMALLOC_IS_ALIGNED

SRC_DIR	:= src
HEADER_DIR := bqfft

SOURCES	:= $(wildcard $(SRC_DIR)/*.cpp)
HEADERS	:= $(wildcard $(HEADER_DIR)/*.h) $(wildcard $(SRC_DIR)/*.h)

OBJECTS	:= $(SOURCES:.cpp=.o)
OBJECTS	:= $(OBJECTS:.c=.o)

CXXFLAGS := $(FFT_DEFINES) $(ALLOCATOR_DEFINES) -O3 -ffast-math -I. -I../bqvec -fpic

LIBRARY	:= libbqfft.a

all:	$(LIBRARY)

$(LIBRARY):	$(OBJECTS)
	$(AR) rc $@ $^

clean:		
	rm -f $(OBJECTS)

distclean:	clean
	rm -f $(LIBRARY)

depend:
	makedepend -Y -fMakefile $(SOURCES) $(HEADERS)


# DO NOT DELETE

src/FFT.o: bqfft/FFT.h
