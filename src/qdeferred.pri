CONFIG += c++11
CONFIG -= flat

INCLUDEPATH += $$PWD/

OTHER_FILES  = QDeferred.natvis

HEADERS     += $$PWD/qdeferred.hpp \
               $$PWD/qdeferreddata.hpp

SOURCES     += $$PWD/qdeferreddata.cpp
