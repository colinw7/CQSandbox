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
CQHtmlTextPainter.cpp \
CCircleFactor.cpp \
CQGLTexture.cpp \
CQSVGUtil.cpp \
CSVGUtil.cpp \
CQAxis.cpp \
CLorenzCalc.cpp \
CQGLUtil.cpp \
CGLTexture.cpp \
CQArrow.cpp \
CQTclUtil.cpp \
CTclUtil.cpp \
\
CForceDirected3D.cpp \

HEADERS += \
CQSandboxApp.h \
CQSandbox.h \
CQSandboxEditor.h \
CQSandboxCanvas3D.h \
CQSandboxUtil.h \
\
CQHtmlTextPainter.h \
CQTclUtil.h \
CQArrow.h \
CQGLTexture.h \
CQSVGUtil.h \
CSVGUtil.h \
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
-L../../CCsv/lib \
-L../../CQBaseModel/lib \
-L../../CQUtil/lib \
-L../../CImportModel/lib \
-L../../CGeometry3D/lib \
-L../../CImageLib/lib \
-L../../CRGBName/lib \
-L../../CUtil/lib \
-L../../CFileUtil/lib \
-L../../CFile/lib \
-L../../CMath/lib \
-L../../CRegExp/lib \
-L../../CStrUtil/lib \
-L../../COS/lib \
-lCQCsv \
-lCQBaseModel \
-lCCsv \
-lCQUtil \
-lCImportModel \
-lCGeometry3D \
-lCImageLib \
-lCRGBName \
-lCUtil \
-lCFileUtil \
-lCFile \
-lCMath \
-lCRegExp \
-lCStrUtil \
-lCOS \
-ltk -ltcl \
-lglut -lGLU \
-ltre -lpng -ljpeg
