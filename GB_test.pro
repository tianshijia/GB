#-------------------------------------------------
#
# Project created by QtCreator 2022-05-01T22:47:58
#
#-------------------------------------------------

QT       += core gui
QT       +=serialport
QT       += axcontainer
QT       += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = GB_test
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        bgtest.cpp \
    gsv.cpp \
    mythread.cpp \
    qcustomplot.cpp

HEADERS += \
        bgtest.h \
    gsv.h \
    mythread.h \
    qcustomplot.h

FORMS += \
        bgtest.ui

RC_ICONS += satellite.ico

RESOURCES += \
    file.qrc

DISTFILES +=

#版本信息
VERSION = 2.1.3
#作者名称
QMAKE_TARGET_COMPANY = "Tianshijia"
#产品名称
QMAKE_TARGET_PRODUCT = "GB_NMEA0183"
#文件说明
QMAKE_TARGET_DESCRIPTION = "Qt 5.14.2 (MinGW 64 bit)"
#版权信息
QMAKE_TARGET_COPYRIGHT = "Public"
#中文（简体）
RC_LANG = 0x0004
