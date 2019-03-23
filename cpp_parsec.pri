# parsec targets

win32:       CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../lib/cpp_parsec/cpp_parsec/release/ -lcpp_parsec
else:win32:  CONFIG(debug, debug|release):   LIBS += -L$$OUT_PWD/../../lib/cpp_parsec/cpp_parsec/debug/   -lcpp_parsec
else:unix:                                   LIBS += -L$$OUT_PWD/../../lib/cpp_parsec/cpp_parsec/         -lcpp_parsec

INCLUDEPATH += $$PWD/../../lib/cpp_parsec/cpp_parsec
DEPENDPATH  += $$PWD/../../lib/cpp_parsec/cpp_parsec

win32-g++:             CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../lib/cpp_parsec/cpp_parsec/release/libcpp_parsec.a
else:win32-g++:        CONFIG(debug, debug|release):   PRE_TARGETDEPS += $$OUT_PWD/../../lib/cpp_parsec/cpp_parsec/debug/libcpp_parsec.a
else:win32:!win32-g++: CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../lib/cpp_parsec/cpp_parsec/release/cpp_parsec.lib
else:win32:!win32-g++: CONFIG(debug, debug|release):   PRE_TARGETDEPS += $$OUT_PWD/../../lib/cpp_parsec/cpp_parsec/debug/cpp_parsec.lib
else:unix:                                             PRE_TARGETDEPS += $$OUT_PWD/../../lib/cpp_parsec/cpp_parsec/libcpp_parsec.a
