#-------------------------------------------------
#
# Project created by QtCreator 2016-12-18T14:33:03
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FileSystem
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    fsmanager.cpp \
    disk.cpp \
    utility.cpp \
    filesystem.cpp \
    squarifiedalgorithm.cpp \
    mainwindowfiletree.cpp \
    mainwindowfolderview.cpp \
    mainwindowui.cpp

HEADERS  += mainwindow.h \
    filecontrolblock.h \
    disk.h \
    fsmanager.h \
    user.h \
    utility.h \
    squarifiedalgorithm.h

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc
