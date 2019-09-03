#-------------------------------------------------
#
# Project created by QtCreator 2018-04-22T15:35:37
#
#-------------------------------------------------

QT       += core gui
QT       += charts
QT       += network
QT       += serialport
QT       += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Sirius
TEMPLATE = app
RC_ICONS = myico.ico

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#DEFINES += QCUSTOMPLOT_USE_OPENGL

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        axistag.cpp \
        configuration.cpp \
        main.cpp \
        mainwindow.cpp \
        onlinevarian.cpp \
        qcustomplot.cpp \
        serialport.cpp \
        status.cpp

HEADERS += \
        axistag.h \
        configuration.h \
        mainwindow.h \
        onlinevarian.h \
        qcustomplot.h \
        serialport.h \
        status.h

FORMS += \
        mainwindow.ui
