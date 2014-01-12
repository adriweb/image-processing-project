#-------------------------------------------------
#
# Project created by QtCreator 2013-12-10T15:22:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Tdi-GUI
TEMPLATE = app

SOURCES += main.cpp \
    TD2-TDI.cpp \
    mainwindow.cpp \
    filtering.cpp \
    BmpLib.cpp \
    OutilsLib.cpp \
    wrappercpp.cpp \
    previewwindow.cpp \
    utils.cpp \
    watershed.cpp

HEADERS  += mainwindow.h \
    filtering.h \
    common.h \
    BmpLib.h \
    TD2-TDI.h \
    OutilsLib.h \
    wrappercpp.h \
    previewwindow.h \
    utils.h \
    watershed.h

FORMS    += mainwindow.ui

