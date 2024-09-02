#ifndef CQSandboxCanvas3D_H
#define CQSandboxCanvas3D_H

#include <CTclUtil.h>
#include <CGLMatrix3D.h>
#include <CGLVector3D.h>
#include <CGLVector2D.h>
#include <CPoint3D.h>

#include <QFrame>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

class CImportBase;
class CQGLBuffer;
class CQGLTexture;
class CGLTexture;
class CGLCamera;
class CGeomObject3D;
class CForceDirected3D;

class QOpenGLShaderProgram;
class QTimer;

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
class OpenGLWindow : public QOpenGLWidget, public QOpenGLFunctions_3_3_Core {
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

struct ProgramData {
  QOpenGLShaderProgram *program           { nullptr };
  GLint                 projectionUniform { 0 };
  GLint                 viewUniform       { 0 };
  GLint                 modelUniform      { 0 };
};

//---

class Canvas3D;

class Object3D : public QObject {
  Q_OBJECT

 public:
  Object3D(Canvas3D *canvas);

  Canvas3D *canvas() const { return canvas_; }

  //---

  size_t ind() const { return ind_; }
  void setInd(size_t ind) { ind_ = ind; }

  const QString &id() const { return id_; }
  void setId(const QString &s) { id_ = s; }

  QString calcId() const;

  bool isVisible() const { return visible_; }
  void setVisible(bool b) { visible_ = b; }

  double xAngle() const { return xAngle_; }
  void setXAngle(double r) { xAngle_ = r; }

  double yAngle() const { return yAngle_; }
  void setYAngle(double r) { yAngle_ = r; }

  double zAngle() const { return zAngle_; }
  void setZAngle(double r) { zAngle_ = r; }

  const CPoint3D &position() const { return position_; }
  void setPosition(const CPoint3D &p);

  //---

  virtual void init() { }

  virtual QVariant getValue(const QString &name, const QStringList &args);
  virtual void setValue(const QString &name, const QString &value, const QStringList &args);

  virtual void tick() { }

  virtual void render();

  QString getCommandName() const;

 protected:
  Canvas3D* canvas_ { nullptr };
  size_t    ind_    { 0 };

  QString id_;
  bool    visible_ { true };

  double   xAngle_   { 0.0 };
  double   yAngle_   { 0.0 };
  double   zAngle_   { 0.0 };
  CPoint3D position_ { 0, 0, 0 };

  CGLMatrix3D modelMatrix_;

  bool needsUpdate_ { true };
};

//---

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

//---

class ShapeObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  ShapeObj(Canvas3D *canvas);

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void setTexture(const QString &filename);

  double scale() const { return scale_; }
  void setScale(double r) { scale_ = r; }

  void init() override;

  void updateGL();

  void render() override;

 protected:
  using Points    = std::vector<CGLVector3D>;
  using Indices   = std::vector<unsigned int>;
  using Colors    = std::vector<CGLVector3D>;
  using TexCoords = std::vector<CGLVector2D>;

  static ProgramData* s_program;

  Points    points_;
  Indices   indices_;
  Colors    colors_;
  TexCoords texCoords_;
  bool      wireframe_ { false };

  double scale_ { 1.0 };

  CQGLTexture *texture_ { nullptr };

  bool useTexture_  { false };

  unsigned int vertexBufferId_   { 0 };
  unsigned int colorBufferId_    { 0 };
  unsigned int texCoordBufferId_ { 0 };
  unsigned int vertexArrayId_    { 0 };
  unsigned int indBufferId_      { 0 };
};

//---

class CubeObj : public ShapeObj {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  CubeObj(Canvas3D *canvas);

  void init() override;
};

//---

class ShaderObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  ShaderObj(Canvas3D *canvas);

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void updateShaders();

  void tick() override;

  void render() override;

 private:
  QOpenGLShaderProgram* program_ { nullptr };

  QString fragmentShader_;
  QString vertexShader_;

  double elapsed_ { 0.0 };
  int    frame_   { 0 };

  bool shaderValid_ { false };
};

//---

class ParticleListObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  ParticleListObj(Canvas3D *canvas);

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void render() override;

 protected:
  void setNumPoints(int n);

 protected:
  struct ParticleListProgramData : public ProgramData {
    GLint positionAttr { 0 };
    GLint centerAttr   { 0 };
    GLint colorAttr    { 0 };
  };

  static size_t s_maxPoints;

  static ParticleListProgramData *s_program;

  using Points = std::vector<CGLVector3D>;
  using Colors = std::vector<Color>;

  Points points_;
  Colors colors_;

  GLuint particles_position_buffer_ { 0 };
  GLuint particles_color_buffer_    { 0 };
  GLuint billboard_vertex_buffer_   { 0 };
};

//---

class FontData;

class Text3DObj : public Object3D {
  Q_OBJECT

 public:
  struct GlyphInfo {
    CGLVector3D positions[4];
    CGLVector2D uvs[4];
    float       offsetX { 0 };
    float       offsetY { 0 };
  };

  struct TextProgramData : public ProgramData {
    GLint posAttr        { 0 };
    GLint colAttr        { 0 };
    GLint texPosAttr     { 0 };
    GLint textureUniform { 0 };
  };

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Text3DObj(Canvas3D *canvas);

  const QString &text() const { return text_; }
  void setText(const QString &s);

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void render() override;

 private:
  void initFont();

  std::vector<uint8_t> readFile(const char *path) const;

  void updateTextData();

  void initRotatingLabel();

  GlyphInfo makeGlyphInfo(uint32_t character, float offsetX, float offsetY) const;

 private:
  struct RotatingLabel {
    GLuint   vao = 0;
    GLuint   vertexBuffer = 0;
    GLuint   uvBuffer = 0;
    GLuint   colBuffer = 0;
    GLuint   indexBuffer = 0;
    uint16_t indexElementCount = 0;
    float    angle { 0.0f };
  };

  static TextProgramData* s_program;
  static FontData*        s_fontData;

  QString text_;
  Color   color_ { 1.0, 1.0, 1.0 };
  double  size_ { 0.05 };
  bool    textDataValid_ { false };

  std::vector<CGLVector3D> vertices_;
  std::vector<CGLVector2D> uvs_;
  std::vector<Color>       colors_;
  std::vector<uint16_t>    indexes_;

  RotatingLabel rotatingLabel_;
};

//---

class GraphObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  GraphObj(Canvas3D *canvas);

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void tick() override;

  void render() override;

 private:
  void updatePoints();

  void updateTextObjs();

 private:
  using Points   = std::vector<CGLVector3D>;
  using TextObjs = std::vector<Text3DObj *>;

  static ProgramData* s_program;

  CForceDirected3D *forceDirected_ { nullptr };

  double stepSize_ { 0.01 };

  Points points_;
  Points linePoints_;

  TextObjs textObjs_;

  unsigned int pointsBufferId_ { 0 };
  unsigned int linesBufferId_  { 0 };
  unsigned int pointsArrayId_  { 0 };
  unsigned int linesArrayId_   { 0 };
};

//---

class Canvas3D : public OpenGLWindow {
  Q_OBJECT

 public:
  Canvas3D(App *app);

  App *app() const { return app_; }

  //---

  int redrawTimeOut() const { return redrawTimeOut_; }

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

  void removeObject(Object3D *obj);

  //---

  void initialize() override;

  void render() override;

  //---

  void mousePressEvent  (QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent   (QMouseEvent *event) override;

  void wheelEvent(QWheelEvent *) override;

  void keyPressEvent(QKeyEvent *event) override;

  //---

  void checkShaderErr(int shader);
  void checkProgramErr(int program);

 private:
  static int objectCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int loadModelProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

 protected Q_SLOTS:
  void timerSlot();

 Q_SIGNALS:
  void cameraChanged();

 private:
  using Objects = std::vector<Object3D *>;

  static QOpenGLShaderProgram* s_lightShaderProgram;

  App* app_ { nullptr };

  QTimer *timer_ { nullptr };
  int     redrawTimeOut_ { 100 };

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
