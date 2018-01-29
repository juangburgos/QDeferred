
QT += core
QT -= gui

TARGET  = test11
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include(./../../src/qlambdathreadworker.pri)
include(./../../src/qdeferred.pri)

SOURCES += main.cpp \

include(./../add_qt_path.pri)