TEMPLATE = app

TARGET = CQSandbox

QT += widgets

DEPENDPATH += .

QMAKE_CXXFLAGS += \
-std=c++17 \
-DNO_QT_APP=1 \
-DOS_UNIX=1 \
-DOS_LINUX=1

MOC_DIR = .moc

CONFIG += debug

SOURCES += \
CQSandboxMain.cpp \
CQSandboxApp.cpp \
\
CQSandboxCanvas2D.cpp \
CQSandboxObject2D.cpp \
\
CQSandboxCanvas3D.cpp \
CQSandboxObject3D.cpp \
CQSandboxParticleSystem.cpp \
CQSandboxLight3D.cpp \
\
CQSandboxArray2DObj.cpp \
CQSandboxArrowObj.cpp \
CQSandboxAStarObj.cpp \
CQSandboxAxisObj.cpp \
CQSandboxCsvObj.cpp \
CQSandboxGroupObj.cpp \
CQSandboxPalette2DObj.cpp \
CQSandboxPathObj.cpp \
CQSandboxPointListObj.cpp \
CQSandboxQuadTreeObj.cpp \
CQSandboxRenderer2DObj.cpp \
CQSandboxShlib2DObj.cpp \
CQSandboxText2DObj.cpp \
CQSandboxVector2DObj.cpp \
\
CQSandboxAnimReal3DObj.cpp \
CQSandboxArray3DObj.cpp \
CQSandboxAStar3DObj.cpp \
CQSandboxAxis3DObj.cpp \
CQSandboxBBox3DObj.cpp \
CQSandboxCsv3DObj.cpp \
CQSandboxCube3DObj.cpp \
CQSandboxDungeon3DObj.cpp \
CQSandboxFieldRunners3DObj.cpp \
CQSandboxGraph3DObj.cpp \
CQSandboxGrid3DObj.cpp \
CQSandboxGroup3DObj.cpp \
CQSandboxJson3DObj.cpp \
CQSandboxLineList3DObj.cpp \
CQSandboxModel3DObj.cpp \
CQSandboxOthello3DObj.cpp \
CQSandboxParticleList3DObj.cpp \
CQSandboxPath3DObj.cpp \
CQSandboxPlane3DObj.cpp \
CQSandboxPoint3DObj.cpp \
CQSandboxPointList3DObj.cpp \
CQSandboxQuadTree3DObj.cpp \
CQSandboxShader3DObj.cpp \
CQSandboxShaderShape3DObj.cpp \
CQSandboxShape3DObj.cpp \
CQSandboxShlib3DObj.cpp \
CQSandboxSkybox3DObj.cpp \
CQSandboxSprite3DObj.cpp \
CQSandboxSurface3DObj.cpp \
CQSandboxText3DObj.cpp \
CQSandboxVector3DObj.cpp \
CQSandboxXML3DObj.cpp \
\
CQSandboxGeomObject.cpp \
\
CQSandboxToolbar2D.cpp \
CQSandboxControl2D.cpp \
CQSandboxConsole2D.cpp \
\
CQSandboxControl3D.cpp \
CQSandboxToolbar3D.cpp \
CQSandboxOverview3D.cpp \
\
CQSandboxStatus.cpp \
\
CQSandboxShaderProgram.cpp \
CQSandboxShaderToyProgram.cpp \
CQSandboxShape3DData.cpp \
\
CQSandboxCamera.cpp \
CQSandboxFPCamera.cpp \
CQSandboxOrthoCamera.cpp \
\
CQGLTexture.cpp \
CQGLCubemap.cpp \
CQSVGUtil.cpp \
CGLCamera.cpp \
\
CCircleFactor.cpp \
CSVGUtil.cpp \
CQAxis.cpp \
CLorenzCalc.cpp \
CQGLUtil.cpp \
CGLTexture.cpp \
CQArrow.cpp \
CQPoint3DEdit.cpp \
CQBBox3DEdit.cpp \
CQTclUtil.cpp \
CTclUtil.cpp \
CQRubberBand.cpp \
CQTclIntegerSpin.cpp \
\
CForceDirected3D.cpp \
CFlag.cpp \
CDotParse.cpp \
CFireworks.cpp \
CFlocking.cpp \
CFlock.cpp \
CBoid.cpp \
CWaterSurface.cpp \
CProfile.cpp \
\
CPSysAttraction.cpp \
CPSysEulerIntegrator.cpp \
CPSysModifiedEulerIntegrator.cpp \
CPSysParticle.cpp \
CPSysRungeKuttaIntegrator.cpp \
CPSysSpring.cpp \
CPSysSystem.cpp \
\
CQTclCommand.cpp \
CQCommand.cpp \
CQTclCmd.cpp \
CTclParse.cpp \
\
CColorRange.cpp \
\
shlib/CShLibMgr.cpp \
shlib/CShLib.cpp \
shlib/CShLibSun.cpp

HEADERS += \
CQSandboxApp.h \
\
CQSandboxCanvas2D.h \
CQSandboxObject2D.h \
\
CQSandboxCanvas3D.h \
CQSandboxObject3D.h \
CQSandboxParticleSystem.h \
CQSandboxLight3D.h \
\
CQSandboxArray2DObj.h \
CQSandboxArrowObj.h \
CQSandboxAStarObj.h \
CQSandboxAxisObj.h \
CQSandboxCsvObj.h \
CQSandboxGroupObj.h \
CQSandboxPalette2DObj.h \
CQSandboxPathObj.h \
CQSandboxPointListObj.h \
CQSandboxQuadTreeObj.h \
CQSandboxRenderer2DObj.h \
CQSandboxShlib2DObj.h \
CQSandboxText2DObj.h \
CQSandboxVector2DObj.h \
\
CQSandboxAnimReal3DObj.h \
CQSandboxArray3DObj.h \
CQSandboxAStar3DObj.h \
CQSandboxAxis3DObj.h \
CQSandboxBBox3DObj.h \
CQSandboxCsv3DObj.h \
CQSandboxCube3DObj.h \
CQSandboxDungeon3DObj.h \
CQSandboxFieldRunners3DObj.h \
CQSandboxGraph3DObj.h \
CQSandboxGrid3DObj.h \
CQSandboxGroup3DObj.h \
CQSandboxJson3DObj.h \
CQSandboxLineList3DObj.h \
CQSandboxModel3DObj.h \
CQSandboxOthello3DObj.h \
CQSandboxParticleList3DObj.h \
CQSandboxPath3DObj.h \
CQSandboxPlane3DObj.h \
CQSandboxPoint3DObj.h \
CQSandboxPointList3DObj.h \
CQSandboxQuadTree3DObj.h \
CQSandboxShader3DObj.h \
CQSandboxShaderShape3DObj.h \
CQSandboxShape3DObj.h \
CQSandboxShlib3DObj.h \
CQSandboxSkybox3DObj.h \
CQSandboxSprite3DObj.h \
CQSandboxSurface3DObj.h \
CQSandboxText3DObj.h \
CQSandboxVector3DObj.h \
CQSandboxXML3DObj.h \
\
CQSandboxGeomObject.h \
\
CQSandboxControl2D.h \
CQSandboxToolbar2D.h \
CQSandboxConsole2D.h \
\
CQSandboxControl3D.h \
CQSandboxToolbar3D.h \
CQSandboxOverview3D.h \
\
CQSandboxStatus.h \
\
CQSandboxShaderProgram.h \
CQSandboxShaderToyProgram.h \
CQSandboxShape3DData.h \
\
CQSandboxUtil.h \
CQTclIntegerSpin.h \
\
CQSandboxCamera.h \
CQSandboxFPCamera.h \
CQSandboxOrthoCamera.h \
\
CQGLTexture.h \
CQGLCubemap.h \
CQSVGUtil.h \
CGLCamera.h \
\
CQTclUtil.h \
CQArrow.h \
CQPoint3DEdit.h \
CQBBox3DEdit.h \
CQSVGUtil.h \
CQGLBuffer.h \
CQAxis.h \
CQRubberBand.h \
\
CQTclCommand.h \
CQCommand.h \
CQTclCmd.h \

DESTDIR     = ../bin
OBJECTS_DIR = ../obj
LIB_DIR     = ../lib

INCLUDEPATH += \
. \
./shlib \
../include \
../../CQPropertyView/include \
../../CQUtil/include \
../../CQXml/include \
../../CXML/include \
../../CJson/include \
../../CGeometry3D/include \
../../CImportModel/include \
../../CQBaseModel/include \
../../CImageLib/include \
../../CFont/include \
../../CCsv/qinclude \
../../CConfig/include \
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
-L../../CQXml/lib \
-L../../CQPropertyView/lib \
-L../../CQPropertyTree/lib \
-L../../CQStyleWidget/lib \
-L../../CQColorPalette/lib \
-L../../CQUtil/lib \
-L../../CImportModel/lib \
-L../../CXML/lib \
-L../../CJson/lib \
-L../../CVoxel/lib \
-L../../CGeometry3D/lib \
-L../../CImageLib/lib \
-L../../CRGBName/lib \
-L../../CConfig/lib \
-L../../CJson/lib \
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
-lCQXml \
-lCQPropertyView \
-lCQPropertyTree \
-lCQStyleWidget \
-lCQColorPalette \
-lCQUtil \
-lCImportModel \
-lCXML \
-lCJson \
-lCVoxel \
-lCGeometry3D \
-lCImageLib \
-lCRGBName \
-lCConfig \
-lCJson \
-lCUtil \
-lCFileUtil \
-lCFile \
-lCMath \
-lCRegExp \
-lCStrUtil \
-lCOS \
-ltk -ltcl \
-lglut -lGLU \
-ltre -lpng -ljpeg -lz
