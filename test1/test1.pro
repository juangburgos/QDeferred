QT += core
QT -= gui

CONFIG += c++11

TARGET = test1
CONFIG += console
CONFIG -= app_bundle

include(./../src/qdeferred.pri)

TEMPLATE = app

SOURCES += main.cpp
