CONFIG += c++11
CONFIG -= flat

INCLUDEPATH += $$PWD/

!contains( DEFINES, QDEFERRED_USED ) {
    include($$PWD/qdeferred.pri)
}

HEADERS  += $$PWD/qlambdathreadworkerdata.h \
            $$PWD/qlambdathreadworker.h

SOURCES  += $$PWD/qlambdathreadworkerdata.cpp \
            $$PWD/qlambdathreadworker.cpp