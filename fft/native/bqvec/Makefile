
# Add to VECTOR_DEFINES the relevant options for your desired
# third-party library support.
#
# Available options are
#
#  -DHAVE_IPP    Intel's Integrated Performance Primitives are available
#  -DHAVE_VDSP   Apple's Accelerate framework is available
#
# These are optional (they affect performance, not function) and you
# may define more than one of them.
# 
# Add any relevant -I flags for include paths as well.
#
# Note that you must supply the same flags when including bqvec
# headers later as you are using now when compiling the library. (You
# may find it simplest to just add the bqvec source files to your
# application's build system and not build a bqvec library at all.)

VECTOR_DEFINES	:= -DHAVE_VDSP


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
# Note that you must supply the same flags when including bqvec
# headers later as you are using now when compiling the library. (You
# may find it simplest to just add the bqvec source files to your
# application's build system and not build a bqvec library at all.)

ALLOCATOR_DEFINES := -DMALLOC_IS_ALIGNED


SRC_DIR	:= src
HEADER_DIR := bqvec

SOURCES	:= $(wildcard $(SRC_DIR)/*.cpp)
HEADERS	:= $(wildcard $(HEADER_DIR)/*.h) $(wildcard $(SRC_DIR)/*.h)

OBJECTS	:= $(SOURCES:.cpp=.o)
OBJECTS	:= $(OBJECTS:.c=.o)

CXXFLAGS := $(VECTOR_DEFINES) $(ALLOCATOR_DEFINES) -I$(HEADER_DIR) -O3 -ffast-math -Wall -Werror -fpic

LIBRARY	:= libbqvec.a

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

src/VectorOpsComplex.o: bqvec/VectorOpsComplex.h bqvec/VectorOps.h
src/VectorOpsComplex.o: bqvec/Restrict.h bqvec/ComplexTypes.h
src/Allocators.o: bqvec/Allocators.h bqvec/VectorOps.h bqvec/Restrict.h
bqvec/RingBuffer.o: bqvec/Barrier.h bqvec/Allocators.h bqvec/VectorOps.h
bqvec/RingBuffer.o: bqvec/Restrict.h
bqvec/VectorOpsComplex.o: bqvec/VectorOps.h bqvec/Restrict.h
bqvec/VectorOpsComplex.o: bqvec/ComplexTypes.h
bqvec/VectorOps.o: bqvec/Restrict.h
bqvec/Allocators.o: bqvec/VectorOps.h bqvec/Restrict.h
