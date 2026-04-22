QT -= core gui

TARGET = cpp_free_parsec
TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++1z

SOURCES += \
    ps/core/runtime.cpp

HEADERS += \
    ps/core/types.h \
    ps/core/runtime.h \
    ps/core/internal/raw_parsers.h \
    ps/core/free/adt.h \
    ps/core/free/functor.h \
    ps/core/free/bind.h \
    ps/core/free/interpreter.h \
    ps/core/church/adt.h \
    ps/core/church/functor.h \
    ps/core/church/bind.h \
    ps/core/conds.h \
    ps/core/engine.h \
    ps/core/parser.h \
    ps/free_parsec.h \
    ps/core/runner.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES +=
