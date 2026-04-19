QT       += testlib
QT       -= gui

TARGET = tst_freeparsectest
CONFIG += console
CONFIG -= app_bundle

CONFIG += c++1z

TEMPLATE = app


SOURCES += \
    tst_free_parsec.cpp \
    tst_samples.cpp \
    tests_runner.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    common.h \
    tst_free_parsec.h \
    tst_samples.h

# ps targets

win32:      CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../cpp_free_parsec/release/ -lcpp_free_parsec
else:win32: CONFIG(debug, debug|release):   LIBS += -L$$OUT_PWD/../cpp_free_parsec/debug/   -lcpp_free_parsec
else:unix:                                  LIBS += -L$$OUT_PWD/../cpp_free_parsec/         -lcpp_free_parsec

INCLUDEPATH += $$PWD/../cpp_free_parsec
DEPENDPATH  += $$PWD/../cpp_free_parsec

win32-g++:              CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cpp_free_parsec/release/libcpp_free_parsec.a
else:win32-g++:         CONFIG(debug, debug|release):   PRE_TARGETDEPS += $$OUT_PWD/../cpp_free_parsec/debug/libcpp_free_parsec.a
else:win32:!win32-g++:  CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../cpp_free_parsec/release/cpp_free_parsec.lib
else:win32:!win32-g++:  CONFIG(debug, debug|release):   PRE_TARGETDEPS += $$OUT_PWD/../cpp_free_parsec/debug/cpp_free_parsec.lib
else:unix:                                              PRE_TARGETDEPS += $$OUT_PWD/../cpp_free_parsec/libcpp_free_parsec.a
