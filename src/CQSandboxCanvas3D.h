#ifndef CQSandboxCanvas3D_H
#define CQSandboxCanvas3D_H

#include <CQSandboxObject3D.h>

#include <CTclUtil.h>
#include <CGLMatrix3D.h>
#include <CGLPath3D.h>
#include <CGLVector3D.h>
#include <CGLVector2D.h>
#include <CGLColor.h>
#include <CPoint3D.h>
#include <CBBox3D.h>
#include <CMinMax.h>

#include <QFrame>
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>

class CGLCamera;

class QTimer;

//---

#include <CQSandboxShaderProgram.h>

namespace CQSandbox {

class App;
//class ShaderProgram;
class ShaderToyProgram;
class Light3D;
class Path3DObj;
class ParticleList3DObj;

//---

// QOpenGLFunctions
class OpenGLWindow : public QOpenGLWidget, public QOpenGLExtraFunctions {
  Q_OBJECT

  Q_PROPERTY(QColor bgColor   READ bgColor     WRITE setBgColor)
  Q_PROPERTY(bool   animating READ isAnimating WRITE setAnimating)

 public:
  explicit OpenGLWindow(QWidget *parent=nullptr);
 ~OpenGLWindow();

  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  //---

  const QColor &bgColor() const { return bgColor_; }
  void setBgColor(const QColor &c) { bgColor_ = c; }

  double aspect() const { return aspect_; }

  //---

  virtual void initialize();

  virtual void render();

  bool isAnimating() const { return animating_; }
  void setAnimating(bool animating);

 protected:
  bool event(QEvent *event) override;

 Q_SIGNALS:
  void typeChanged();

 protected:
  QColor bgColor_ { 0, 0, 0 };

  bool animating_ { false };

  double pixelWidth_  { 100.0 };
  double pixelHeight_ { 100.0 };

  double aspect_ { 1.0 };
};

//---

class Canvas3D : public OpenGLWindow {
  Q_OBJECT

  Q_PROPERTY(double ambient   READ ambient   WRITE setAmbient)
  Q_PROPERTY(double diffuse   READ diffuse   WRITE setDiffuse)
  Q_PROPERTY(double specular  READ specular  WRITE setSpecular)
  Q_PROPERTY(double shininess READ shininess WRITE setShininess)

  Q_PROPERTY(bool polygonLine READ isPolygonLine WRITE setPolygonLine)
  Q_PROPERTY(bool wireframe   READ isWireframe   WRITE setWireframe)
  Q_PROPERTY(bool showBBox    READ isBBox        WRITE setBBox)

  Q_PROPERTY(bool simpleLights READ isSimpleLights WRITE setSimpleLights)

  Q_PROPERTY(bool depthTest   READ isDepthTest   WRITE setDepthTest)
  Q_PROPERTY(bool cullFace    READ isCullFace    WRITE setCullFace)
  Q_PROPERTY(bool lighting    READ isLighting    WRITE setLighting)
  Q_PROPERTY(bool frontFace   READ isFrontFace   WRITE setFrontFace)
  Q_PROPERTY(bool smoothShade READ isSmoothShade WRITE setSmoothShade)

  Q_PROPERTY(int redrawTimeOut READ redrawTimeOut WRITE setRedrawTimeOut)

 public:
  enum class Type {
    CAMERA = 0,
    LIGHT  = 1,
    MODEL  = 2,
    GAME   = 3
  };

  using Objects = std::vector<Object3D *>;

 public:
  static QString buildDir() { return s_buildDir; }

  //---

  Canvas3D(App *app);

  App *app() const { return app_; }

  //---

  int redrawTimeOut() const { return redrawTimeOut_; }
  void setRedrawTimeOut(int t);

  //---

  double ambient() const { return ambient_; }
  void setAmbient(double r) { ambient_ = r; }

  double diffuse() const { return diffuse_; }
  void setDiffuse(double r) { diffuse_ = r; }

  double specular() const { return specular_; }
  void setSpecular(double r) { specular_ = r; }

  double shininess() const { return shininess_; }
  void setShininess(double r) { shininess_ = r; }

  //---

  bool isPolygonLine() const { return polygonLine_; }
  void setPolygonLine(bool b) { polygonLine_ = b; }

  bool isWireframe() const { return wireframe_; }
  void setWireframe(bool b) { wireframe_ = b; }

  bool isBBox() const { return bbox_; }
  void setBBox(bool b) { bbox_ = b; }

  //---

  CGLCamera *camera() const { return camera_; }

  float modelXAngle() const { return modelXAngle_; }
  float modelYAngle() const { return modelYAngle_; }
  float modelZAngle() const { return modelZAngle_; }

  //---

  Light3D *currentLight() const;

  void updateLights();

  void setProgramLights(ShaderProgram *program);

  const std::vector<Light3D *> lights() const { return lights_; }

  int lightNum() const { return lightNum_; }
  void setLightNum(int i) { lightNum_ = i; }

  bool isSimpleLights() const { return simpleLights_; }
  void setSimpleLights(bool b) { simpleLights_ = b; }

  //---

  const Objects objects() const { return objects_; }

  Object3D *objectFromInd(uint ind) const;

  Object3D *getCurrentObject() const;

  //---

  const Type &type() const { return type_; }
  void setType(const Type &type);

  //---

  bool isDepthTest() { return depthTest_; }
  void setDepthTest(bool b) { depthTest_ = b; }

  bool isCullFace() { return cullFace_; }
  void setCullFace(bool b) { cullFace_ = b; }

  bool isLighting() { return lighting_; }
  void setLighting(bool b) { lighting_ = b; }

  bool isFrontFace() { return frontFace_; }
  void setFrontFace(bool b) { frontFace_ = b; }

  bool isSmoothShade() { return smoothShade_; }
  void setSmoothShade(bool b) { smoothShade_ = b; }

  bool isOutline() { return outline_; }

  //---

  const CRMinMax &xrange() const { return xrange_; }
  const CRMinMax &yrange() const { return yrange_; }
  const CRMinMax &zrange() const { return zrange_; }

  //---

  const CGLMatrix3D &projectionMatrix() const { return projectionMatrix_; }

  const CGLMatrix3D &viewMatrix() const { return viewMatrix_; }

  const CGLVector3D &viewPos() const { return viewPos_; }

  //---

  void init();
  void addCommands();

  void createObjCommand(Object3D *obj);

  QString addNewObject(Object3D *obj);

  void addObject(Object3D *obj);

  void removeObject(Object3D *obj);

  Object3D *getObjectByName(const QString &name) const;

  //---

  void initialize() override;

  void render() override;

  //---

  void mousePressEvent  (QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent   (QMouseEvent *event) override;

  void setMousePos(float xpos, float ypos);

  void wheelEvent(QWheelEvent *) override;

  void keyPressEvent(QKeyEvent *event) override;

  //---

  void checkShaderErr(int shader);
  void checkProgramErr(int program);

 private:
  static int objectCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int canvasProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
  static int cameraProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int customFormProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

#if 0
  static int loadModelProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
#endif

  QVariant getValue(const QString &name, const QStringList &args);
  bool setValue(const QString &name, const QString &value, const QStringList &args);

  QVariant getCameraValue(const QString &name, const QStringList &args);
  void setCameraValue(const QString &name, const QString &value, const QStringList &args);

 protected Q_SLOTS:
  void timerSlot();

 Q_SIGNALS:
  void cameraChanged();

  void objectsChanged();

 private:
  static QString s_buildDir;

  using Points = std::vector<CGLVector3D>;

  App* app_ { nullptr };

  QTimer *timer_ { nullptr };
  int     redrawTimeOut_ { 100 };

  size_t lastInd_ { 0 };

  double ambient_   { 0.5 };
  double diffuse_   { 0.5 };
  double specular_  { 1.0 };
  double shininess_ { 32.0 };

  bool polygonLine_ { false };
  bool wireframe_   { false };
  bool bbox_        { false };

  Type type_ { Type::CAMERA };

  bool depthTest_   { true };
  bool cullFace_    { true };
  bool lighting_    { true };
  bool frontFace_   { false };
  bool smoothShade_ { true };
  bool outline_     { false };

  CRMinMax xrange_ { -1.0, 1.0 };
  CRMinMax yrange_ { -1.0, 1.0 };
  CRMinMax zrange_ { -1.0, 1.0 };

  CGLMatrix3D projectionMatrix_;
  CGLMatrix3D viewMatrix_;
  CGLVector3D viewPos_;

  CGLCamera* camera_ { nullptr };

  Path3DObj* eyeLine_ { nullptr };

  uint numDirectionalLights_ { 1 };
  uint numPointLights_ { 2 };
  uint numSpotLights_ { 2 };

  std::vector<Light3D *> lights_;
  uint                   lightNum_ { 0 };
  bool                   simpleLights_ { false };

  ParticleList3DObj* intersectParticles_ { nullptr };

  float modelXAngle_ { 0.0f };
  float modelYAngle_ { 0.0f };
  float modelZAngle_ { 0.0f };

  Objects objects_;
  Objects allObjects_;

  Points intersectPoints_;

  bool pressed_ { false };
};

}

#endif
