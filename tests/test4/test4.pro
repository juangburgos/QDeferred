QT += core
QT -= gui

TARGET = test4
CONFIG += console
CONFIG -= app_bundle

include(./../../src/qdeferred.pri)
include(./../../src/qdefthread.pri)

TEMPLATE = app

SOURCES  += main.cpp \
