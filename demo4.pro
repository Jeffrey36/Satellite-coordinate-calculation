QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyFirstWindow
TEMPLATE = app
QT += core gui openglwidgets opengl
CONFIG += c++11

SOURCES += main.cpp \
           mainwindow.cpp \
           calculate.cpp \
           map.cpp

HEADERS += mainwindow.h \
           calculate.h \
           map.h

INCLUDEPATH += D:\APP\eigen-3.4.0

FORMS += mainwindow.ui
LIBS += -lopengl32
RC_FILE = icon.txt
