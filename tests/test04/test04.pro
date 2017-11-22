QT += core
QT -= gui

TARGET = test04
CONFIG += console
CONFIG -= app_bundle

include(./../../src/qdeferred.pri)
include(./../../src/qlambdathreadworker.pri)

TEMPLATE = app

SOURCES  += main.cpp \

include(./../add_qt_path.pri)