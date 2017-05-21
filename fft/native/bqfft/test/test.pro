
TEMPLATE = app
TARGET = test-fft
win*: TARGET = "TestFFT"

QT += testlib
QT -= gui

DESTDIR = .
QMAKE_LIBDIR += . ..

LIBS += -lbqfft -lfftw3 -lfftw3f

INCLUDEPATH += . .. ../../bqvec
DEPENDPATH += . .. ../../bqvec

HEADERS += TestFFT.h
SOURCES += main.cpp

!win32 {
    !macx* {
        QMAKE_POST_LINK=valgrind $${DESTDIR}/$${TARGET}
    }
    macx* {
        QMAKE_POST_LINK=$${DESTDIR}/$${TARGET}.app/Contents/MacOS/$${TARGET}
    }
}

win32-g++:QMAKE_POST_LINK=$${DESTDIR}$${TARGET}.exe

