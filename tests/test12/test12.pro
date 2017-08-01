QT += core
QT -= gui

TARGET = test12
CONFIG += console
CONFIG -= app_bundle

include(./../../src/qdeferred.pri)

TEMPLATE = app

SOURCES += main.cpp

include(./../add_qt_path.pri)
