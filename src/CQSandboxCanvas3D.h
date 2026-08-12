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
#include <CRGBA.h>

#include <QFrame>
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>

class CGeomScene3D;
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

  Q_PROPERTY(double ambientStrength  READ ambientStrength  WRITE setAmbientStrength)
  Q_PROPERTY(double diffuseStrength  READ diffuseStrength  WRITE setDiffuseStrength)
  Q_PROPERTY(double specularStrength READ specularStrength WRITE setSpecularStrength)
  Q_PROPERTY(double shininess        READ shininess        WRITE setShininess)

  Q_PROPERTY(bool polygonLine READ isPolygonLine WRITE setPolygonLine)
  Q_PROPERTY(bool wireframe   READ isWireframe   WRITE setWireframe)
  Q_PROPERTY(bool solid       READ isSolid       WRITE setSolid)
  Q_PROPERTY(bool textured    READ isTextured    WRITE setTextured)
  Q_PROPERTY(bool showBBox    READ isShowBBox    WRITE setShowBBox)

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

  const CRGBA &ambientColor() const { return ambientColor_; }
  void setAmbientColor(const CRGBA &v) { ambientColor_ = v; }

  double ambientStrength() const { return ambientStrength_; }
  void setAmbientStrength(double r) { ambientStrength_ = r; }

  double diffuseStrength() const { return diffuseStrength_; }
  void setDiffuseStrength(double r) { diffuseStrength_ = r; }

  const CRGBA &specularColor() const { return specularColor_; }
  void setSpecularColor(const CRGBA &v) { specularColor_ = v; }

  double specularStrength() const { return specularStrength_; }
  void setSpecularStrength(double r) { specularStrength_ = r; }

  const CRGBA &emissiveColor() const { return emissiveColor_; }
  void setEmissiveColor(const CRGBA &v) { emissiveColor_ = v; }

  double emissiveStrength() const { return emissiveStrength_; }
  void setEmissiveStrength(double r) { emissiveStrength_ = r; }

  double shininess() const { return shininess_; }
  void setShininess(double r) { shininess_ = r; }

  //---

  bool isPolygonLine() const { return polygonLine_; }
  void setPolygonLine(bool b) { polygonLine_ = b; }

  bool isWireframe() const { return wireframe_; }
  void setWireframe(bool b) { wireframe_ = b; }

  bool isSolid() const { return solid_; }
  void setSolid(bool b) { solid_ = b; }

  bool isTextured() const { return textured_; }
  void setTextured(bool b) { textured_ = b; }

  bool isShowBBox() const { return showBBox_; }
  void setShowBBox(bool b) { showBBox_ = b; }

  //---

  CGeomScene3D *scene() const { return scene_; }

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
  void mouseMoveEvent   (QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

  void setMousePos(float xpos, float ypos);

  void wheelEvent(QWheelEvent *) override;

  void keyPressEvent(QKeyEvent *event) override;

  //---

  void checkShaderErr(int shader);
  void checkProgramErr(int program);

  //---

  const QStringList &modelDirs() const { return modelDirs_; }

 private:
  static int objectCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int canvasProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
  static int cameraProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int customFormProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

#if 0
  static int loadModelProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
#endif

  bool getValue(const QString &name, const QStringList &args, QVariant &value);
  bool setValue(const QString &name, const QString &value, const QStringList &args);

  bool getCameraValue(const QString &name, const QStringList &args, QVariant &value);
  bool setCameraValue(const QString &name, const QString &value, const QStringList &args);

 protected Q_SLOTS:
  void timerSlot();

 Q_SIGNALS:
  void cameraChanged();

  void objectsChanged();

 private:
  struct MouseData {
    bool            pressed   { false };
    bool            isShift   { false };
    bool            isControl { false };
    Qt::MouseButton button    { Qt::NoButton };
    CPoint2D        press     { 0.0, 0.0 };
    CPoint2D        move      { 0.0, 0.0 };
  };

  //---

  static QString s_buildDir;

  using Points = std::vector<CGLVector3D>;

  App* app_ { nullptr };

  QTimer *timer_ { nullptr };
  int     redrawTimeOut_ { 100 };

  size_t lastInd_ { 0 };

  // lighting
  CRGBA  ambientColor_     { CRGBA::white() };
  double ambientStrength_  { 0.2 };
  double diffuseStrength_  { 1.0 };
  CRGBA  emissiveColor_    { CRGBA::white() };
  double emissiveStrength_ { 0.0 };
  CRGBA  specularColor_    { CRGBA::white() };
  double specularStrength_ { 0.2 };

  double shininess_ { 32.0 };

  bool polygonLine_ { false };
  bool wireframe_   { false };
  bool solid_       { false };
  bool textured_    { true };
  bool showBBox_    { false };

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

  // interaction
  MouseData mouseData_;

  //---

  CGeomScene3D* scene_ { nullptr };

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

  QStringList modelDirs_;
};

}

#endif
