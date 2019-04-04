QT -= core gui

TARGET = cpp_parsec
TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++1z

SOURCES += \
    ps/context.cpp

HEADERS += \
    ps/context.h \
    ps/impl/runtime.h \
    ps/types.h \
    ps/church/interpreter.h \
    ps/church/functor.h \
    ps/ps.h \
    ps2/tvar.h \
    ps2/context.h \
    ps2/psf/visitor.h \
    ps2/psf/psf.h \
    ps2/impl/runtime.h \
    ps2/types.h \
    ps2/church/psl.h \
    ps2/church/interpreter.h \
    ps2/church/functor.h \
    ps2/church/ps.h \
    ps2/ps.h \
    ps/church/parserl.h \
    ps/church/parser.h \
    ps/psf/parserf.h \
    ps/psf/visitor.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES +=
