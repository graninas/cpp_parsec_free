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
    ps/core/parser/adt.h \
    ps/core/parser/functor.h \
    ps/core/parser/bind.h \
    ps/core/conds.h \
    ps/core/interpreter.h \
    ps/core/raw_parsers.h \
    ps/core/parser.h \
    ps/core/merge.h \
    ps/core/choice.h \
    ps/free_parsec.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES +=
