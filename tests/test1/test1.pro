QT += core
QT -= gui

TARGET = test1
CONFIG += console
CONFIG -= app_bundle

include(./../../src/qdeferred.pri)

TEMPLATE = app

SOURCES += main.cpp
