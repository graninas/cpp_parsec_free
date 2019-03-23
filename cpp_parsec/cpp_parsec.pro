QT -= core gui

TARGET = cpp_parsec
TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++1z

SOURCES += \
    ps/context.cpp

HEADERS += \
    ps/tvar.h \
    ps/context.h \
    ps/psf/functor.h \
    ps/psf/psf.h \
    ps/impl/runtime.h \
    ps/types.h \
    ps/church/psl.h \
    ps/church/interpreter.h \
    ps/church/functor.h \
    ps/church/ps.h \
    ps/ps.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES +=
