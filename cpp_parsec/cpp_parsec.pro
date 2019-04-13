QT -= core gui

TARGET = cpp_parsec
TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++1z

SOURCES += \
    ps/context.cpp

HEADERS += \
    ps/context.h \
    ps/types.h \
    ps/ps.h \
    ps/free/interpreter.h \
    ps/free/bind.h \
    ps/free/parser.h \
    ps/free/parserl.h \
    ps/psf/parserf.h \
    ps/psf/visitor.h \
    ps/psf/parserfst.h \
    ps/psf/visitorst.h \
    ps/free/parserlst.h \
    ps/free/interpreterst.h \
    ps/free/bindst.h \
    ps/free/parserl_functor.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES +=
