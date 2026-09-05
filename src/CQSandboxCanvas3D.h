#ifndef CQSandboxCanvas3D_H
#define CQSandboxCanvas3D_H

#include <CQSandboxObject3D.h>
#include <CQSandboxGeom.h>

#include <CTclUtil.h>
#include <CGLMatrix3D.h>
#include <CGLPath3D.h>
#include <CGLVector2D.h>
#include <CGLColor.h>
#include <CPoint3D.h>
#include <CBBox3D.h>
#include <CMinMax.h>
#include <CRGBA.h>
#include <CPlane3D.h>

#include <QFrame>
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>

class CQRubberBand;

class CGeomScene3D;
class CGeomObject3D;
class CGeomFace3D;
class CGeomVertex3D;
class CQGLBuffer;
class CQTcl;

class QTimer;

//---

#include <CQSandboxShaderProgram.h>

namespace CQSandbox {

class App;
class ShaderToyProgram;
class Light3D;
class Path3DObj;
class ParticleList3DObj;

class Camera;
class FPCamera;
class OrthoCamera;
class CameraIFace;

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

  Q_PROPERTY(bool looping       READ isLooping     WRITE setLooping)
  Q_PROPERTY(int  redrawTimeOut READ redrawTimeOut WRITE setRedrawTimeOut)

 public:
  enum class Type {
    CAMERA = 0,
    LIGHT  = 1,
    MODEL  = 2,
    GAME   = 3
  };

  enum class EditMode {
    MOVE,
    SCALE,
    ROTATE
  };

  enum class EditType {
    POINT,
    LINE,
    FACE,
    OBJECT
  };

  enum class CameraType {
    MODEL,
    FIRST_PERSON,
    ORTHO
  };

  using Mgrs    = std::map<QString, ObjectMgr3D *>;
  using Objects = std::vector<Object3D *>;

  //---

  enum { NUM_NODE_MATRICES = 128 };

  using NodeMatrices       = std::map<int, CMatrix3D>;
  using ObjectNodeMatrices = std::map<uint, NodeMatrices>;

  using FrameMatrix = std::map<int, CMatrix3DH>;

  struct ObjectMeshData {
    double tmin { 0.0 };
    double tmax { 1.0 };
    int    nt   { 10 };
    double dt   { 0.1 };

    FrameMatrix frameMatrix;
  };

  using Cameras = std::vector<CameraIFace *>;

 public:
  Canvas3D(App *app);

  App *app() const { return app_; }

  CQTcl *tcl() const;

  int ind() const { return 0; }

  //---

  bool isLooping() const { return looping_; }
  void setLooping(bool b);

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

  bool isEyeLineVisible() const { return eyeLineVisible_; }
  void setEyeLineVisible(bool b) { eyeLineVisible_ = b; }

  bool isAnimEnabled() const { return animEnabled_; }
  void setAnimEnabled(bool b) { animEnabled_ = b; }

  bool isLightsVisible() const { return lightsVisible_; }
  void setLightsVisible(bool b) { lightsVisible_ = b; }

  bool isSelectionSolid() const { return selectionSolid_; }
  void setSelectionSolid(bool b) { selectionSolid_ = b; }

  //---

  CGeomScene3D *scene() const { return scene_; }

  //---

  void setProgramMatrices(ShaderProgram *program);

  //---

  Camera      *camera     () const { return camera_  ; }
  FPCamera    *fpCamera   () const { return fpCamera_; }
  OrthoCamera *orthoCamera() const { return orthoCamera_; }

  const Cameras &cameras() const { return cameras_; }

  CameraIFace *currentCamera() const;

  const CameraType &cameraType() const { return cameraType_; }
  void setCameraType(const CameraType &cameraType);

  //---

  double modelXAngle() const { return modelXAngle_; }
  double modelYAngle() const { return modelYAngle_; }
  double modelZAngle() const { return modelZAngle_; }

  //---

  Light3D *currentLight() const;

  Light3D *getLight(uint i) const;

  void updateLights();

  void resetLight(Light3D *);

  void setProgramLightGlobals(ShaderProgram *program);
  void setProgramSimpleLight(ShaderProgram *program);
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

  const EditMode &editMode() const { return editMode_; }
  void setEditMode(const EditMode &mode);

  const EditType &editType() const { return editType_; }
  void setEditType(const EditType &type);

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

  const CMatrix3DH &projectionMatrix() const { return projectionMatrix_; }

  const CMatrix3DH &viewMatrix() const { return viewMatrix_; }

  const CVector3D &viewPos() const { return viewPos_; }

  const CBBox3D &bbox() const { return bbox_; }

  //---

  void init();

  void initCamera();

  void addCommands();

  void createObjCommand(Object3D *obj);

  //--

  void addObjectMgr(ObjectMgr3D *mgr);

  QString addNewObject(Object3D *obj);

  void addObject(Object3D *obj);

  void removeObject(Object3D *obj);

  //---

  Object3D *getObjectByName(const QString &name) const;

  //---

  void initialize() override;

  void render() override;

  void bindBuffer (CQGLBuffer *buffer);
  void bindProgram(ShaderProgram *program);

  void initSelectionProgram();

  void drawLights();

  //---

  void clearObjectMeshData();

  bool addObjectMeshData(CGeomObject3D *object, CMatrix3DH &meshMatrix);

  ObjectMeshData &getObjectMeshData(CGeomObject3D *object);

  //---

  void mousePressEvent  (QMouseEvent *e) override;
  void mouseMoveEvent   (QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;

  void wheelEvent(QWheelEvent *e) override;

  bool event(QEvent *e) override;

  void keyPressEvent  (QKeyEvent *e) override;
  void keyReleaseEvent(QKeyEvent *e) override;

  void mouseMoveCamera();

  //---

  void selectNearestPoint(const CPoint2D &p);
  void selectNearestLine (const CPoint2D &p);
  void selectNearestFace (const CPoint2D &p);

  void selectPointsInside(const CBBox2D &r);
  void selectLinesInside (const CBBox2D &r);
  void selectFacesInside (const CBBox2D &r);

  //---

  void gameKeyPress();
  void cameraKeyPress();
  void lightKeyPress();
  void modelKeyPress();

  //---

  bool getKeyPressed(const QString &key) const;

  QString getKeyString(QKeyEvent *e) const;

  //---

  void selectObjects(const std::vector<Object3D *> &objs, bool clear=false, bool update=true);
  void selectObject(Object3D *obj, bool clear=false, bool update=true);
  void selectFace(CGeomFace3D *face, bool clear=false, bool update=true);
  void deselectAll();

  void setMousePos(double xpos, double ypos);

  //---

  void checkShaderErr(int shader);
  void checkProgramErr(int program);

  //---

  const QStringList &modelDirs() const { return modelDirs_; }

  //---

  void resetCamera();

  //---

  void updateNodeMatrices(CGeomObject3D *object);

  const NodeMatrices &getObjectNodeMatrices(CGeomObject3D *object) const;

  const ObjectNodeMatrices &getNodeMatrices() const;

  ObjectNodeMatrices calcNodeMatrices() const;

  void invalidateNodeMatrices() { objectNodeMatricesValid_ = false; }

  QMatrix4x4 *nodeQMatrices() const;
  int numNodeQMatrices() const { return NUM_NODE_MATRICES; }

  CPoint3D adjustAnimPoint(const CGeomVertex3D &vertex, const CPoint3D &p,
                           const NodeMatrices &nodeMatrices) const;

  bool getNodeMatrix(const NodeMatrices &nodeMatrices, int nodeId, CMatrix3D &m) const;

  //---

  std::vector<CGeomObject3D *> getAnimObjects() const;

  //---

  void addClip(const CPlane3D &clip);

  void enableClips(bool b);

  void setProgramClips(ShaderProgram *program);

  const std::vector<CPlane3D> &clips() const { return clips_; }

  //---

  bool runTclCmd(const QString &cmd);

 protected:
  void addEyeLine();
  void addIntersectParticles();

 private:
  static int objectCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int canvasProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
  static int cameraProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
  static int lightProc (void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int uiProc        (void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
  static int customFormProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

#if 0
  static int loadModelProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
#endif

  bool getValue(const QString &name, const QStringList &args, QVariant &value);
  bool setValue(const QString &name, const QString &value, const QStringList &args);

  bool getCameraValue(const QString &name, const QStringList &args, QVariant &value);
  bool setCameraValue(const QString &name, const QString &value, const QStringList &args);
  bool execCamera(const QString &op, const QStringList &args, QVariant &res);

  bool getLightValue(const QString &name, const QStringList &args, QVariant &value);
  bool setLightValue(const QString &name, const QString &value, const QStringList &args);
  bool execLight(const QString &op, const QStringList &args, QVariant &res);

 protected Q_SLOTS:
  void timerSlot();
  void uiTimerSlot();

  void cameraChangeSlot();
  void lightChangeSlot();

 Q_SIGNALS:
  void bboxChanged();

  void objectsChanged();

  void uiUpdateSignal();

  void cameraChangedSignal();

  void lightAdded();

 private:
  struct PaintData {
    CGeomObject3D*          animObject { nullptr };
    std::vector<CMatrix3D>  nodeMatrices;
    std::vector<QMatrix4x4> nodeQMatrices;

    void reset() {
      animObject = nullptr;

      nodeMatrices .clear();
      nodeQMatrices.clear();
    }
  };

  //---

  struct MouseData {
    bool            pressed   { false };
    bool            isShift   { false };
    bool            isControl { false };
    Qt::MouseButton button    { Qt::NoButton };
    CPoint2D        press     { 0.0, 0.0 };
    CPoint2D        move1     { 0.0, 0.0 };
    CPoint2D        move2     { 0.0, 0.0 };
    int             key       { 0 };
    QString         keyStr;
  };

  //---

  using ObjectMeshDataMap = std::map<CGeomObject3D *, ObjectMeshData>;

  using Points = std::vector<CVector3D>;

  ///---

  App* app_ { nullptr };

  CQTcl* tcl_ { nullptr };

  bool    looping_       { false };
  QTimer *timer_         { nullptr };
  QTimer *uiTimer_       { nullptr };
  int     redrawTimeOut_ { 100 };

  size_t lastInd_ { 0 };

  // lighting
  CRGBA  ambientColor_     { CRGBA::white() };
  double ambientStrength_  { 0.1 };
  double diffuseStrength_  { 1.0 };
  CRGBA  specularColor_    { CRGBA::white() };
  double specularStrength_ { 0.2 };
  CRGBA  emissiveColor_    { CRGBA::white() };
  double emissiveStrength_ { 0.1 };
  double shininess_        { 32.0 };

  bool polygonLine_    { false };
  bool wireframe_      { false };
  bool solid_          { false };
  bool textured_       { true };
  bool showBBox_       { false };
  bool eyeLineVisible_ { false };
  bool animEnabled_    { true };
  bool lightsVisible_  { false };
  bool selectionSolid_ { false };

  Type     type_     { Type::CAMERA };
  EditMode editMode_ { EditMode::MOVE };
  EditType editType_ { EditType::POINT };

  CameraType cameraType_ { CameraType::MODEL };
  Cameras    cameras_;

  bool depthTest_   { true };
  bool cullFace_    { true };
  bool lighting_    { true };
  bool frontFace_   { false };
  bool smoothShade_ { true };
  bool outline_     { false };

  CRMinMax xrange_ { -1.0, 1.0 };
  CRMinMax yrange_ { -1.0, 1.0 };
  CRMinMax zrange_ { -1.0, 1.0 };

  CMatrix3DH projectionMatrix_;
  CMatrix3DH viewMatrix_;
  CVector3D  viewPos_;

  CBBox3D bbox_;

  // interaction
  MouseData mouseData_;

  CQGLBuffer*    currentBuffer_  { nullptr };
  ShaderProgram* currentProgram_ { nullptr };

  //---

  CGeomScene3D* scene_ { nullptr };

  Camera*      camera_      { nullptr };
  FPCamera*    fpCamera_    { nullptr };
  OrthoCamera* orthoCamera_ { nullptr };

  Path3DObj* eyeLine_ { nullptr };

  uint numDirectionalLights_ { 1 };
  uint numPointLights_ { 2 };
  uint numSpotLights_ { 2 };

  std::vector<Light3D *> lights_;
  uint                   lightNum_ { 1 };
  bool                   simpleLights_ { false };

  ParticleList3DObj* intersectParticles_ { nullptr };

  double modelXAngle_ { 0.0 };
  double modelYAngle_ { 0.0 };
  double modelZAngle_ { 0.0 };

  Mgrs mgrs_;

  Objects objects_;
  Objects allObjects_;
  bool    objectsValid_ { false };

  Points intersectPoints_;

  QStringList modelDirs_;

  bool ignoreChange_ { false };

  //---

  PaintData paintData_;

  ObjectMeshDataMap objectMeshData_;

  ObjectNodeMatrices objectNodeMatrices_;
  bool               objectNodeMatricesValid_ { false };

  //---

  using KeyPressed = std::map<QString, bool>;

  KeyPressed keyPressed_;

  //---

  std::vector<CPlane3D> clips_;

  //---

  CQRubberBand* rubberBand_ { nullptr };

  ShaderProgram* selectionProgram_ { nullptr };
  CQGLBuffer*    selectionBuffer_  { nullptr };
};

}

#endif
