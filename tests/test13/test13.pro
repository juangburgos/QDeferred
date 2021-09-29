QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(./../../src/qdeferred.pri)
include(./../../src/qlambdathreadworker.pri)

TARGET = test13
TEMPLATE = app

SOURCES += main.cpp\
        awaitwidget.cpp

HEADERS  += awaitwidget.h

FORMS    += awaitwidget.ui

include(./../add_qt_path.pri)
