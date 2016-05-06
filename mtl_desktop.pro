#-------------------------------------------------
#
# Project created by QtCreator 2016-04-29T10:49:22
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mtl_desktop
TEMPLATE = app

INCLUDEPATH += \
            include

SOURCES += main.cpp\
        mainwindow.cpp \
    createpagedialog.cpp

HEADERS  += mainwindow.h \
    createpagedialog.h

FORMS    += mainwindow.ui \
    createpagedialog.ui

RESOURCES += \
    mtl_desktop.qrc

DISTFILES += \
    test_data.xml \
    format.xml

LIBS += \
    -lqjson
