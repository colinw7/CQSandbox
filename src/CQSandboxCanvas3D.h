#ifndef CQSandboxCanvas3D_H
#define CQSandboxCanvas3D_H

#include <CTclUtil.h>
#include <CGLMatrix3D.h>
#include <CGLPath3D.h>
#include <CGLVector3D.h>
#include <CGLVector2D.h>
#include <CPoint3D.h>
#include <CMinMax.h>

#include <QFrame>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

class CQCsvModel;
class CQGLBuffer;
class CQGLTexture;
class CImportBase;
class CGLTexture;
class CGLCamera;
class CGeomObject3D;
class CGeomTexture;
class CForceDirected3D;

#ifdef CQSANDBOX_WATER_SURFACE
class CWaterSurface;
#endif

#ifdef CQSANDBOX_OTHELLO
class COthelloBoard;
#endif

#ifdef CQSANDBOX_FIELD_RUNNERS
class CFieldRunners;
#endif

#ifdef CQSANDBOX_FLAG
class CFlag;
#endif

#ifdef CQSANDBOX_FLOCKING
class CFlocking;
#endif

#ifdef CQSANDBOX_FIREWORKS
class CFireworks;
#endif

class CShape3D;

class QOpenGLShaderProgram;
class QTimer;

namespace CDotParse {
class Parse;
}

namespace CQSandbox {

struct Color {
  Color() = default;

  Color(float r_, float g_, float b_, float a_=1.0) :
   r(r_), g(g_), b(b_), a(a_) {
  }

  float r { 0.0f };
  float g { 0.0f };
  float b { 0.0f };
  float a { 1.0f };
};

//---

class App;

// QOpenGLFunctions
class OpenGLWindow : public QOpenGLWidget, public QOpenGLFunctions_3_3_Core {
  Q_OBJECT

 public:
  explicit OpenGLWindow(QWidget *parent=nullptr);
 ~OpenGLWindow();

  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  virtual void initialize();

  virtual void render();

  void setAnimating(bool animating);

 protected:
  bool event(QEvent *event) override;

 Q_SIGNALS:
  void typeChanged();

 protected:
  bool animating_ { false };
};

//---

struct ProgramData {
  QOpenGLShaderProgram *program           { nullptr };
  GLint                 projectionUniform { 0 };
  GLint                 viewUniform       { 0 };
};

//---

class Canvas3D;
class Group3DObj;

class Object3D : public QObject {
  Q_OBJECT

 public:
  enum ModelMatrixFlags : unsigned int {
    NONE      = 0,
    TRANSLATE = (1<<0),
    SCALE     = (1<<1),
    ROTATE    = (1<<2),
    ALL       = (TRANSLATE|SCALE|ROTATE)
  };

 public:
  Object3D(Canvas3D *canvas);

  Canvas3D *canvas() const { return canvas_; }

  virtual const char *typeName() const = 0;

  //---

  size_t ind() const { return ind_; }
  void setInd(size_t ind) { ind_ = ind; }

  const QString &id() const { return id_; }
  void setId(const QString &s) { id_ = s; }

  QString calcId() const;

  bool isVisible() const { return visible_; }
  void setVisible(bool b) { visible_ = b; }

  bool isSelected() const { return selected_; }
  void setSelected(bool b) { selected_ = b; }

  bool isInside() const { return inside_; }
  void setInside(bool b) { inside_ = b; }

  double xAngle() const { return xAngle_; }
  void setXAngle(double r) { xAngle_ = r; updateModelMatrix(); }

  double yAngle() const { return yAngle_; }
  void setYAngle(double r) { yAngle_ = r; updateModelMatrix(); }

  double zAngle() const { return zAngle_; }
  void setZAngle(double r) { zAngle_ = r; updateModelMatrix(); }

  const CPoint3D &position() const { return position_; }
  void setPosition(const CPoint3D &p);

  double xscale() const { return xscale_; }
  double yscale() const { return yscale_; }
  double zscale() const { return zscale_; }

  void setScale(double s) { xscale_ = s; yscale_ = s; zscale_ = s; }
  void setScale(double xs, double ys, double zs) { xscale_ = xs; yscale_ = ys; zscale_ = zs; }

  const CGLMatrix3D &modelMatrix() const { return modelMatrix_; }

  //---

  Group3DObj *group() const { return group_; }
  void setGroup(Group3DObj *group) { group_ = group; }

  //---

  void setNeedsUpdate() { needsUpdate_ = true; }

  //---

  virtual void init();

  void setModelMatrix(uint flags=ModelMatrixFlags::ALL);

  virtual QVariant getValue(const QString &name, const QStringList &args);
  virtual void setValue(const QString &name, const QString &value, const QStringList &args);

  virtual QVariant exec(const QString &, const QStringList &) { return QVariant(); }

  //---

  virtual void updateModelMatrix() { }

  virtual void tick();

  virtual void render();

  virtual bool intersect(const CGLVector3D &, const CGLVector3D &, CPoint3D &, CPoint3D &) const {
    return false;
  }

  QString getCommandName() const;

 protected:
  Canvas3D* canvas_ { nullptr };
  size_t    ind_    { 0 };

  QString id_;
  bool    visible_  { true };
  bool    selected_ { false };
  bool    inside_   { false };

  double   xAngle_   { 0.0 };
  double   yAngle_   { 0.0 };
  double   zAngle_   { 0.0 };
  CPoint3D position_ { 0, 0, 0 };
  double   xscale_   { 1.0 };
  double   yscale_   { 1.0 };
  double   zscale_   { 1.0 };

  CGLMatrix3D modelMatrix_;

  int ticks_ { 0 };

  Group3DObj *group_ { nullptr };

  bool needsUpdate_ { true };
};

//---

class Model3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Model3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Model"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

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

  CGLTexture *diffuseTexture_  { nullptr };
  CGLTexture *specularTexture_ { nullptr };
  CGLTexture *normalTexture_   { nullptr };

  CPoint3D    sceneCenter_ { 0 , 0, 0 };
  ObjectDatas objectDatas_;

  bool flipYZ_ { false };

  GLTextures glTextures_;
};

//---

class Csv3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Csv3DObj(Canvas3D *canvas, const QString &filename);

  const char *typeName() const override { return "Csv"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  QVariant exec(const QString &op, const QStringList &args) override;

  //---

  void init() override;

 protected:
  QString filename_;

  CQCsvModel *csv_ { nullptr };
};

//---

class Group3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Group3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Group"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void addObject(Object3D *obj);
  void removeObject(Object3D *obj);

 Q_SIGNALS:
  void objectsChanged();

 protected:
  using Objects = std::vector<Object3D *>;

  Objects objects_;
};

//---

class Shape3DObj : public Object3D {
  Q_OBJECT

 public:
  struct VertexData {
    CGLVector3D position;
    CGLVector3D normal;
    CGLVector2D texCoord;

    VertexData(const CGLVector3D &position, const CGLVector3D &normal,
               const CGLVector2D &texCoord) :
     position(position), normal(normal), texCoord(texCoord) {
    }
  };

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Shape3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Shape"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  const Color &color() const { return color_; }
  void setColor(const Color &c) { color_ = c; }

  void setTexture(const QString &filename);
  void setNormalTexture(const QString &filename);

  void init() override;

  void updateGL();

  bool intersect(const CGLVector3D &p1, const CGLVector3D &p2,
                 CPoint3D &pi1, CPoint3D &pi2) const override;

  void calcNormals();

  void render() override;

 protected:
  void addCone(double r, double h);
  void addCylinder(double r, double h);
  void addSphere(double r);
  void addCube(double sx, double sy, double sz);

  void addBodyRev(double *x, double *y, uint num_xy, uint num_patches);

  void addBodyRev(double *x, double *y, uint num_xy, uint num_patches,
                  std::vector<VertexData> &vertexDatas, std::vector<unsigned int> &indices);

 protected:
  using Points    = std::vector<CGLVector3D>;
  using Indices   = std::vector<unsigned int>;
  using Colors    = std::vector<Color>;
  using TexCoords = std::vector<CGLVector2D>;

  static ProgramData* s_program;

  Color color_ { 1.0, 1.0, 1.0, 1.0 };

  Points    points_;
  Points    normals_;
  Colors    colors_;
  Indices   indices_;
  TexCoords texCoords_;
  bool      wireframe_ { false };

  CQGLTexture *texture_       { nullptr };
  CQGLTexture *normalTexture_ { nullptr };

  bool useTexture_       { false };
  bool useNormalTexture_ { false };
  bool useTriangleStrip_ { false };
  bool useTriangleFan_   { false };

  unsigned int pointsBufferId_   { 0 };
  unsigned int normalsBufferId_  { 0 };
  unsigned int colorsBufferId_   { 0 };
  unsigned int texCoordBufferId_ { 0 };
  unsigned int vertexArrayId_    { 0 };
  unsigned int indBufferId_      { 0 };

  CShape3D *geom_ { nullptr };
};

//---

class Cube3DObj : public Shape3DObj {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Cube3DObj(Canvas3D *canvas);

  void init() override;
};

//---

class Shader3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Shader3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Shader"; }

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

  bool shaderValid_ { false };
};

//---

class ParticleList3DObj : public Object3D {
  Q_OBJECT

 public:
  using Points = std::vector<CGLVector3D>;

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  ParticleList3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "ParticleList"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void setPoints(const Points &points);

  void init() override;

  void tick() override;

  void render() override;

 protected:
  void setNumPoints(int n);

#ifdef CQSANDBOX_FLOCKING
  void updateFlocking();
#endif

#ifdef CQSANDBOX_FIREWORKS
  void updateFireworks();
#endif

 protected:
  struct ParticleListProgramData : public ProgramData {
    GLint positionAttr { 0 };
    GLint centerAttr   { 0 };
    GLint colorAttr    { 0 };
  };

  static size_t s_maxPoints;

  static ParticleListProgramData *s_program;

  using Colors = std::vector<Color>;

  Points points_;
  Colors colors_;

  GLuint particles_position_buffer_ { 0 };
  GLuint particles_color_buffer_    { 0 };
  GLuint billboard_vertex_buffer_   { 0 };

#ifdef CQSANDBOX_FLOCKING
  CFlocking* flocking_ { nullptr };
#endif

#ifdef CQSANDBOX_FIREWORKS
  CFireworks* fireworks_ { nullptr };
#endif
};

//---

class Surface3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Surface3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Surface"; }

  void init() override;

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void resizePoints();

  void tick() override;

#ifdef CQSANDBOX_WATER_SURFACE
  void updateWaterSurface();
#endif

#ifdef CQSANDBOX_FLAG
  void updateFlag();
#endif

  void updateGL();

  void calcNormals();

  void render() override;

 private:
  using Points  = std::vector<CGLVector3D>;
  using Colors  = std::vector<CGLVector3D>;
  using Indices = std::vector<unsigned int>;

  static ProgramData* s_program;

  Points  points_;
  Points  normals_;
  Colors  colors_;
  Indices indices_;
  int     nx_ { 0 };
  int     ny_ { 0 };

  bool wireframe_ { false };

#ifdef CQSANDBOX_WATER_SURFACE
  CWaterSurface *waterSurface_ { nullptr };
#endif

#ifdef CQSANDBOX_FLAG
  CFlag *flag_ { nullptr };
#endif

  unsigned int pointsBufferId_  { 0 };
  unsigned int normalsBufferId_ { 0 };
  unsigned int colorsBufferId_  { 0 };
  unsigned int vertexArrayId_   { 0 };
  unsigned int indBufferId_     { 0 };
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

  const char *typeName() const override { return "Text"; }

  const QString &text() const { return text_; }
  void setText(const QString &s);

  const Color &color() const { return color_; }
  void setColor(const Color &c) { color_ = c; }

  bool isRotated() const { return rotated_; }
  void setRotated(bool b) { rotated_ = b; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void render() override;

 private:
  void initFont();

  std::vector<uint8_t> readFile(const char *path) const;

  void updateTextData();

  void initGLData();

  GlyphInfo makeGlyphInfo(uint32_t character, float offsetX, float offsetY) const;

 private:
  struct GLData {
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

  GLData glData_;

  bool rotated_ { false };
};

//---

class Path3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Path3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Path"; }

  void setLine(const CGLVector3D &p1, const CGLVector3D &p2);

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void render() override;

 private:
  void updatePoints();

  void updateGL();

 private:
  CGLPath3D path_;

  using Points = std::vector<CGLVector3D>;

  static ProgramData* s_program;

  Points points_;

  unsigned int pointsBufferId_ { 0 };
  unsigned int vertexArrayId_  { 0 };
};

//---

class Axis3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Axis3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Axis"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void tick() override;

  void render() override;

 private:
  void updateObjects();

  void updateModelMatrix() override;

 private:
  using TextObjs = std::vector<Text3DObj *>;

  CGLVector3D start_ { 0, 0, 0 };
  CGLVector3D end_   { 1, 0, 0 };
  double      min_   { 0 };
  double      max_   { 1 };

  Path3DObj *path_ { nullptr };
  TextObjs   textObjs_;

  std::vector<double> ticks_;
};

//---

class Sprite3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Sprite3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Sprite"; }

  void setTexture(CQGLTexture *texture);

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void updateBuffer();

  void tick() override;

  void render() override;

 private:
  void updateObjects();

  void updateModelMatrix() override;

 private:
  static ProgramData* s_program;

  CQGLBuffer*                buffer_ { nullptr };
  std::vector<CQGLTexture *> textures_;
  int                        textureNum_ { 0 };
  int                        textureStart_ { 0 };
  int                        textureEnd_ { -1 };
  double                     xv_ { 0.0 };
  double                     yv_ { 0.0 };
};

//---

#ifdef CQSANDBOX_OTHELLO
class Othello3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Othello3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Othello"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void render() override;

 private:
  COthelloBoard* board_ { nullptr };
};
#endif

//---

#ifdef CQSANDBOX_FIELD_RUNNERS
class FieldRunners3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  FieldRunners3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "FieldRunners"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void tick() override;

  void render() override;

 private:
  using Sprites  = std::vector<Sprite3DObj *>;
  using Textures = std::map<QString, CQGLTexture *>;

  CFieldRunners* runners_ { nullptr };

  Sprites  bgSprites_;
  Sprites  runnerSprites_;
  Textures textures_;
};
#endif

//---

class Graph3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Graph3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Graph"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  void setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void tick() override;

  void render() override;

 private:
  void addDemoNodes();

  void initSteps();

  bool loadDotFile(const QString &name);

  void updatePoints();

  void updateModelMatrix() override;

  void updateTextObjs();

 private:
  using Points   = std::vector<CGLVector3D>;
  using TextObjs = std::vector<Text3DObj *>;

  static ProgramData* s_program1;
  static ProgramData* s_program2;

  CForceDirected3D *forceDirected_ { nullptr };

  CDotParse::Parse *parse_ { nullptr };

  double stepSize_ { 0.01 };

  Color lineColor_ { 1.0, 0.0, 0.0 };

  Points points_;
  Points linePoints_;

  TextObjs textObjs_;

  unsigned int pointsBufferId_ { 0 };
  unsigned int linesBufferId_  { 0 };
  unsigned int pointsArrayId_  { 0 };
  unsigned int linesArrayId_   { 0 };
};

//---

class Light3D {
 public:
  Light3D(Canvas3D *canvas);

  const CGLVector3D &pos() const { return pos_; }
  void setPos(const CGLVector3D &p) { pos_ = p; }

  const CGLVector3D &color() const { return color_; }
  void setColor(const CGLVector3D &c) { color_ = c; }

  void initBuffer();
  void initShader();

  void render();

 private:
  static QOpenGLShaderProgram* s_shaderProgram;

  Canvas3D* canvas_ { nullptr };

  CGLVector3D pos_   { 0.4f, 0.4f, 0.4f };
  CGLVector3D color_ { 1.0f, 1.0f, 1.0f };

  CQGLBuffer* buffer_ { nullptr };
};

//---

class Canvas3D : public OpenGLWindow {
  Q_OBJECT

 public:
  enum class Type {
    CAMERA = 0,
    LIGHT  = 1,
    MODEL  = 2,
    GAME   = 3
  };

 public:
  Canvas3D(App *app);

  App *app() const { return app_; }

  //---

  int redrawTimeOut() const { return redrawTimeOut_; }

  //---

  const QColor &bgColor() const { return bgColor_; }
  void setBgColor(const QColor &c) { bgColor_ = c; }

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

  //---

  CGLCamera *camera() const { return camera_; }

  float modelXAngle() const { return modelXAngle_; }
  float modelYAngle() const { return modelYAngle_; }
  float modelZAngle() const { return modelZAngle_; }

  //---

  Light3D *light() const { return light_; }

  //---

  const Type &type() const { return type_; }
  void setType(const Type &type);

  //---

  bool isDepthTest  () { return true; }
  bool isCullFace   () { return true; }
  bool isLighting   () { return true; }
  bool isFrontFace  () { return true; }
  bool isSmoothShade() { return true; }
  bool isOutline    () { return false; }

  //---

  double aspect() const { return aspect_; }

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

#if 0
  static int loadModelProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
#endif

  QVariant getValue(const QString &name, const QStringList &args);
  void setValue(const QString &name, const QString &value, const QStringList &args);

  QVariant getCameraValue(const QString &name, const QStringList &args);
  void setCameraValue(const QString &name, const QString &value, const QStringList &args);

 protected Q_SLOTS:
  void timerSlot();

 Q_SIGNALS:
  void cameraChanged();

  void objectsChanged();

 private:
  using Objects = std::vector<Object3D *>;
  using Points  = std::vector<CGLVector3D>;

  App* app_ { nullptr };

  QTimer *timer_ { nullptr };
  int     redrawTimeOut_ { 100 };

  size_t lastInd_ { 0 };

  QColor bgColor_ { 0, 0, 0 };

  double ambient_   { 0.5 };
  double diffuse_   { 0.5 };
  double specular_  { 1.0 };
  double shininess_ { 32.0 };

  bool polygonLine_ { false };
  bool wireframe_   { false };

  double pixelWidth_  { 100.0 };
  double pixelHeight_ { 100.0 };

  Type type_ { Type::CAMERA };

  double aspect_ { 1.0 };

  CRMinMax xrange_ { -1.0, 1.0 };
  CRMinMax yrange_ { -1.0, 1.0 };
  CRMinMax zrange_ { -1.0, 1.0 };

  CGLMatrix3D projectionMatrix_;
  CGLMatrix3D viewMatrix_;
  CGLVector3D viewPos_;

  CGLCamera* camera_ { nullptr };

  Light3D* light_ { nullptr };

  Path3DObj* eyeLine_ { nullptr };

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
