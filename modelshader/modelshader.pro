# QT += core
QT -= gui
QT += opengl

CONFIG += c++11

TARGET = modelshader
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += /usr/local/include
INCLUDEPATH += /usr/include

# LIBS += "c:\Program Files (x86)\Windows Kits\8.0\Lib\win8\um\x86\OpenGL32.Lib"
# LIBS += "c:\Program Files (x86)\Windows Kits\8.0\Lib\win8\um\x86\GlU32.Lib"
# LIBS += "c:\sdks\freeglut\lib\freeglut.lib"
# LIBS += "c:\sdks\glew\lib\Release\Win32\glew32.lib"
# LIBS += "c:\sdks\glfw\lib\win32\glfw3.lib"

LIBS += /usr/lib/x86_64-linux-gnu/mesa/libGL.so
LIBS += -L/usr/local/lib -lGLEW -lglfw -lglut -lGLU -lSDL2 -lSDL2_image

SOURCES += main.cpp \
    ../glcommon/matrix4f.cpp \
    ../glcommon/glaux.cpp \
    ../glcommon/load_m.cpp \
    ../glcommon/loadbitmap.cpp

HEADERS += \
    ../glcommon/loadshaders.h \
    ../glcommon/matrix4f.h \
    ../glcommon/glaux.h \
    ../glcommon/load_m.h \
    ../glcommon/loadbitmap.h

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
