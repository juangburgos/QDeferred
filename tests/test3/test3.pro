QT += core
QT -= gui

TARGET = test3
CONFIG += console
CONFIG -= app_bundle

include(./../../src/qdeferred.pri)

TEMPLATE = app

HEADERS  += threadworker.h

SOURCES += main.cpp \
           threadworker.cpp

include(./../add_qt_path.pri)