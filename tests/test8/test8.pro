
QT += core
QT -= gui

TARGET  = test8
CONFIG += console
CONFIG -= app_bundle

include(./../../src/qdynamicevents.pri)
include(./../../src/qlambdathreadworker.pri)

TEMPLATE = app

HEADERS += derived.h \

SOURCES += main.cpp \
           derived.cpp
