TEMPLATE = app

TARGET = CQSandbox

QT += widgets

DEPENDPATH += .

QMAKE_CXXFLAGS += -std=c++17

MOC_DIR = .moc

CONFIG += debug

SOURCES += \
CQSandboxMain.cpp \
CQSandboxApp.cpp \
CQSandbox.cpp \
CQSandboxEditor.cpp \
CQSandboxCanvas3D.cpp \
\
CQGLTexture.cpp \
CQSVGUtil.cpp \
CQAxis.cpp \
CLorenzCalc.cpp \

HEADERS += \
CQSandboxApp.h \
CQSandbox.h \
CQSandboxEditor.h \
CQSandboxCanvas3D.h \
CQSandboxUtil.h \
\
CQGLTexture.h \
CQSVGUtil.h \
CGLCamera.h \
CQGLBuffer.h \
CQAxis.h \

DESTDIR     = ../bin
OBJECTS_DIR = ../obj
LIB_DIR     = ../lib

INCLUDEPATH += \
. \
../include \
../../CQUtil/include \
../../CSVG/include \
../../CGeometry3D/include \
../../CImportModel/include \
../../CQBaseModel/include \
../../CImageLib/include \
../../CFont/include \
../../CCsv/qinclude \
../../CUtil/include \
../../CFile/include \
../../CMath/include \
../../CStrUtil/include \
../../COS/include \
/usr/include/tcl \

unix:LIBS += \
-L$$LIB_DIR \
