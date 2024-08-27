TEMPLATE = app

TARGET = CQSandbox

QT += widgets

DEPENDPATH += .

QMAKE_CXXFLAGS += -std=c++17

MOC_DIR = .moc

CONFIG += debug

SOURCES += \
CQSandboxApp.cpp 
CQSandboxCanvas3D.cpp 
CQSandbox.cpp 
CQSandboxEditor.cpp 
CQSandboxMain.cpp 
CQSVGUtil.cpp 

HEADERS += \
CGLCamera.h \
CQGLBuffer.h \
CQSandboxApp.h \
CQSandboxCanvas3D.h \
CQSandboxEditor.h \
CQSandbox.h \
CQSandboxUtil.h \
CQSVGUtil.h \

DESTDIR     = ../bin
OBJECTS_DIR = ../obj
LIB_DIR     = ../lib

INCLUDEPATH += \
. \
../include \
../../CQUtil/include \
../../CUtil/include \
../../CMath/include \
../../COS/include \
/usr/include/tcl \

unix:LIBS += \
-L$$LIB_DIR \
