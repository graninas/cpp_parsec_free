QT       += testlib
QT       -= gui

TARGET = tst_parsectest
CONFIG += console
CONFIG -= app_bundle

CONFIG += c++1z

TEMPLATE = app


SOURCES += \
    tst_parsec.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    common.h

# ps targets

win32:      CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../cpp_parsec/release/ -lcpp_parsec
else:win32: CONFIG(debug, debug|release):   LIBS += -L$$OUT_PWD/../cpp_parsec/debug/   -lcpp_parsec
else:unix:                                  LIBS += -L$$OUT_PWD/../cpp_parsec/         -lcpp_parsec

INCLUDEPATH += $$PWD/../cpp_parsec
DEPENDPATH  += $$PWD/../cpp_parsec

win32-g++:              CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cpp_parsec/release/libcpp_parsec.a
else:win32-g++:         CONFIG(debug, debug|release):   PRE_TARGETDEPS += $$OUT_PWD/../cpp_parsec/debug/libcpp_parsec.a
else:win32:!win32-g++:  CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cpp_parsec/release/cpp_parsec.lib
else:win32:!win32-g++:  CONFIG(debug, debug|release):   PRE_TARGETDEPS += $$OUT_PWD/../cpp_parsec/debug/cpp_parsec.lib
else:unix:                                              PRE_TARGETDEPS += $$OUT_PWD/../cpp_parsec/libcpp_parsec.a
