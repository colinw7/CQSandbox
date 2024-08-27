#ifndef CQSandboxCanvas3D_H
#define CQSandboxCanvas3D_H

#include <CGLVector3D.h>
#include <CTclUtil.h>
#include <CGLMatrix3D.h>
#include <CGLVector3D.h>
#include <CPoint3D.h>

#include <QFrame>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

class CImportBase;
class CQGLBuffer;
class CGLTexture;
class CGLCamera;
class CGeomObject3D;

class QOpenGLShaderProgram;

namespace CQSandbox {

struct Color {
  Color() = default;

  Color(float r_, float g_, float b_) :
   r(r_), g(g_), b(b_) {
  }

  float r { 0.0f };
  float g { 0.0f };
  float b { 0.0f };
};

//---

class App;

// QOpenGLFunctions
class OpenGLWindow : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
  Q_OBJECT

 public:
  enum class Type {
    CAMERA = 0,
    LIGHT  = 1,
    MODEL  = 2
  };

 public:
  explicit OpenGLWindow(QWidget *parent=nullptr);
 ~OpenGLWindow();

  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  virtual void initialize();

  virtual void render();

  void setAnimating(bool animating);

  const Type &type() const { return type_; }
  void setType(const Type &type);

  void genBufferId(GLuint *id);
  void bindArrayBuffer(GLuint id);
  void bindArrayBufferData(size_t size, const void *data, GLuint type);
  void enableVertexAttribArray(GLuint ind);
  void setBufferSubData(size_t size, const void *data);
  void setVertexAttribPointer(GLuint attrId, size_t size, GLuint type, GLuint normalized);
  void setVertexAttribDivisor(GLuint attrId, size_t n);
  void drawInstancesTriangles(size_t n);

 protected:
  bool event(QEvent *event) override;

 Q_SIGNALS:
  void typeChanged();

 protected:
  bool animating_ { false };

  Type type_ { Type::CAMERA };

  CGLVector3D lightPos_ { 0.4f, 0.4f, 0.4f };
};

//---

class Canvas3D;
class ProgramData;

class Object3D : public QObject {
  Q_OBJECT

 public:
  Object3D(Canvas3D *canvas);

  Canvas3D *canvas() const { return canvas_; }

  size_t ind() const { return ind_; }
  void setInd(size_t ind) { ind_ = ind; }

  const QString &id() const { return id_; }
  void setId(const QString &s) { id_ = s; }

  QString calcId() const;

  bool isVisible() const { return visible_; }
  void setVisible(bool b) { visible_ = b; }

  virtual QVariant getValue(const QString &name, const QStringList &args);
  virtual void setValue(const QString &name, const QString &value, const QStringList &args);

  virtual void render();

  QString getCommandName() const;

 protected:
  Canvas3D* canvas_ { nullptr };
  size_t    ind_    { 0 };

  QString id_;
  bool    visible_ { true };
};

class Model3D : public Object3D {
  Q_OBJECT

 public:
  Model3D(Canvas3D *canvas);

  bool load(const QString &filename);

  void render() override;

  void updateObjectData();

 protected:
  struct FaceData {
    int         pos             { 0 };
    int         len             { 0 };
    CGLTexture *diffuseTexture  { nullptr };
    CGLTexture *specularTexture { nullptr };
    CGLTexture *normalTexture   { nullptr };
  };

  using FaceDatas = std::vector<FaceData>;

  FaceDatas faceDatas;

  struct ObjectData {
    CQGLBuffer* buffer { nullptr };
    FaceDatas   faceDatas;
  };

  using ObjectDatas = std::map<CGeomObject3D*, ObjectData *>;
  using GLTextures  = std::map<int, CGLTexture *>;

  //---

  static QOpenGLShaderProgram* s_modelShaderProgram;

  QString filename_;

  CImportBase* import_ { nullptr };

  float       sceneScale_  { 1.0 };
  CPoint3D    sceneCenter_ { 0 , 0, 0 };
  ObjectDatas objectDatas_;

  bool flipYZ_ { false };

  GLTextures glTextures_;
};

class ParticleListObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  ParticleListObj(Canvas3D *canvas);

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void render() override;

 protected:
  void setNumPoints(int n);

  void updateGL();

 protected:
  static size_t s_maxPoints;

  static ProgramData *s_program;

  using Points = std::vector<CGLVector3D>;
  using Colors = std::vector<Color>;

  Points points_;
  Colors colors_;

  GLuint particles_position_buffer_ { 0 };
  GLuint particles_color_buffer_    { 0 };
  GLuint billboard_vertex_buffer_   { 0 };
};

//---

class Canvas3D : public OpenGLWindow {
  Q_OBJECT

 public:
  Canvas3D(App *app);

  App *app() const { return app_; }

  //---

  bool isPolygonLine() const { return polygonLine_; }
  void setPolygonLine(bool b) { polygonLine_ = b; }

  //---

  const CGLVector3D &lightPos() const { return lightPos_; }

  double ambient() const { return ambient_; }
  void setAmbient(double r) { ambient_ = r; }

  double diffuse() const { return diffuse_; }
  void setDiffuse(double r) { diffuse_ = r; }

  double specular() const { return specular_; }
  void setSpecular(double r) { specular_ = r; }

  double shininess() const { return shininess_; }
  void setShininess(double r) { shininess_ = r; }

  //---

  CGLCamera *camera() const { return camera_; }

  float modelXAngle() const { return modelXAngle_; }
  float modelYAngle() const { return modelYAngle_; }
  float modelZAngle() const { return modelZAngle_; }

  //---

  double aspect() const { return aspect_; }

  const CGLMatrix3D &projectionMatrix() const { return projectionMatrix_; }

  const CGLMatrix3D &viewMatrix() const { return viewMatrix_; }

  const CGLVector3D &viewPos() const { return viewPos_; }

  //---

  void init();
  void addCommands();

  void createObjCommand(Object3D *obj);

  QString addNewObject(Object3D *obj);

  void addObject(Object3D *obj);

  //---

  void initialize() override;

  void render() override;

  //---

  void mousePressEvent  (QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent   (QMouseEvent *event) override;

  void wheelEvent(QWheelEvent *) override;

  void keyPressEvent(QKeyEvent *event) override;

 private:
  static int objectCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int loadModelProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

 private:
  using Objects = std::vector<Object3D *>;

  static QOpenGLShaderProgram* s_lightShaderProgram;

  App* app_ { nullptr };

  size_t lastInd_ { 0 };

  bool polygonLine_ { false };

  QColor bgColor_ { 0, 0, 0 };

  CGLVector3D lightPos_ { 0.4f, 0.4f, 0.4f };

  double ambient_   { 0.5 };
  double diffuse_   { 0.5 };
  double specular_  { 1.0 };
  double shininess_ { 32.0 };

  double aspect_ { 1.0 };

  CGLMatrix3D projectionMatrix_;
  CGLMatrix3D viewMatrix_;
  CGLVector3D viewPos_;

  CGLCamera* camera_ { nullptr };

  CQGLBuffer* lightBuffer_ { nullptr };

  float modelXAngle_ { 0.0f };
  float modelYAngle_ { 0.0f };
  float modelZAngle_ { 0.0f };

  Objects objects_;

  bool pressed_ { false };
};

}

#endif
