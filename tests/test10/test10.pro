
QT += core
QT -= gui

TARGET  = test10
CONFIG += console
CONFIG -= app_bundle

include(./../../src/qeventer.pri)

TEMPLATE = app

SOURCES += main.cpp \

include(./../add_qt_path.pri)