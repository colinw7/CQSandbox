#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>
#include <CQGLUtil.h>

#include <CImportBase.h>
#include <CGeomScene3D.h>
#include <CGeomTexture.h>
#include <CLorenzCalc.h>
#include <CForceDirected3D.h>
#include <CDotParse.h>
#include <CInterval.h>

#include <CQGLBuffer.h>
#include <CQGLTexture.h>
#include <CGLTexture.h>
#include <CGLCamera.h>
#include <CWaterSurface.h>
#include <CFlag.h>
#include <CFlocking.h>
#include <CFireworks.h>

#include <CFile.h>
#include <CMinMax.h>

#include <QFileInfo>
#include <QMouseEvent>
#include <QTimer>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <fstream>

#define Q(x) #x
#define QUOTE(x) Q(x)

namespace CQSandbox {

QVector3D toVector(const Color &c)
{
  return QVector3D(c.r, c.g, c.b);
}

template<typename T>
int createObjectProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv) {
  auto *th = static_cast<Canvas3D *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);

  if (! T::create(th, args))
    return TCL_ERROR;

  return TCL_OK;
}

QString
point3DToString(const CPoint3D &p) {
  auto xstr = QString::number(p.x);
  auto ystr = QString::number(p.y);
  auto zstr = QString::number(p.z);

  return xstr + " " + ystr + " " + zstr;
}

QString
vector3DToString(const CGLVector3D &p) {
  auto xstr = QString::number(p.x());
  auto ystr = QString::number(p.y());
  auto zstr = QString::number(p.z());

  return xstr + " " + ystr + " " + zstr;
}

CGLVector3D
stringToVector3D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CGLVector3D p;

  if (strs.size() >= 3) {
    bool ok;
    auto x = strs[0].toDouble(&ok);
    auto y = strs[1].toDouble(&ok);
    auto z = strs[2].toDouble(&ok);

    p = CGLVector3D(x, y, z);
  }

  return p;
}

CGLVector2D
stringToVector2D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CGLVector2D p;

  if (strs.size() >= 2) {
    bool ok;
    auto x = strs[0].toDouble(&ok);
    auto y = strs[1].toDouble(&ok);

    p = CGLVector2D(x, y);
  }

  return p;
}

CPoint3D
stringToPoint3D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CPoint3D p;

  if (strs.size() >= 3) {
    bool ok;
    auto x = strs[0].toDouble(&ok);
    auto y = strs[1].toDouble(&ok);
    auto z = strs[2].toDouble(&ok);

    p.x = x;
    p.y = y;
    p.z = z;
  }

  return p;
}

std::vector<CGLVector3D>
stringToVectors3D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  std::vector<CGLVector3D> points;

  for (const auto &str : strs) {
    auto p = stringToVector3D(tcl, str);

    points.push_back(p);
  }

  return points;
}

std::vector<CGLVector2D>
stringToVectors2D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  std::vector<CGLVector2D> points;

  for (const auto &str : strs) {
    auto p = stringToVector2D(tcl, str);

    points.push_back(p);
  }

  return points;
}

std::vector<unsigned int>
stringToUIntArray(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  std::vector<unsigned int> integers;

  for (const auto &str : strs) {
    bool ok;
    auto i = str.toInt(&ok);

    integers.push_back(i);
  }

  return integers;
}

QString
colorToString(const Color &c) {
  auto rstr = QString::number(c.r);
  auto gstr = QString::number(c.g);
  auto bstr = QString::number(c.b);

  return rstr + " " + gstr + " " + bstr;
}

Color
stringToColor(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  Color c;

  if (strs.size() >= 3) {
    bool ok;
    auto r = strs[0].toDouble(&ok);
    auto g = strs[1].toDouble(&ok);
    auto b = strs[2].toDouble(&ok);

    c.r = r;
    c.g = g;
    c.b = b;
  }

  return c;
}

//---

OpenGLWindow::
OpenGLWindow(QWidget *parent) :
 QOpenGLWidget(parent)
{
  //setSurfaceType(QWindow::OpenGLSurface);
}

OpenGLWindow::
~OpenGLWindow()
{
}


void
OpenGLWindow::
initializeGL()
{
  initializeOpenGLFunctions();

  initialize();
}

void
OpenGLWindow::
resizeGL(int, int)
{
}

void
OpenGLWindow::
paintGL()
{
  render();
}

void
OpenGLWindow::
initialize()
{
}

void
OpenGLWindow::
render()
{
}

void
OpenGLWindow::
setAnimating(bool animating)
{
  animating_ = animating;

  update();
}

void
OpenGLWindow::
setType(const Type &type)
{
  if (type != type_) {
    type_ = type;

    Q_EMIT typeChanged();
  }
}

bool
OpenGLWindow::
event(QEvent *event)
{
  return QOpenGLWidget::event(event);
}

//---

QOpenGLShaderProgram* Canvas3D::s_lightShaderProgram;

Canvas3D::
Canvas3D(App *app) :
 app_(app)
{
  setFocusPolicy(Qt::StrongFocus);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void
Canvas3D::
init()
{
  addCommands();

  app_->runTclCmd("proc init { args } { }");
  app_->runTclCmd("proc update { args } { }");

  timer_ = new QTimer;

  connect(timer_, &QTimer::timeout, this, &Canvas3D::timerSlot);

  timer_->start(redrawTimeOut_);
}

void
Canvas3D::
addCommands()
{
   auto *tcl = app_->tcl();

  // global
  tcl->createObjCommand("sb3d::load_model",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas3D::loadModelProc),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::shape",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Shape3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::cube",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<CubeObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::particle_list",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<ParticleListObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::shader",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<ShaderObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::graph",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Graph3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::surface",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<SurfaceObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::text",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Text3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::path",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Path3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::axis",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Axis3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));
}

void
Canvas3D::
createObjCommand(Object3D *obj)
{
  auto *tcl = app_->tcl();

  auto name = obj->getCommandName();

  tcl->createObjCommand(name,
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas3D::objectCommandProc),
    static_cast<CQTcl::ObjCmdData>(obj));
}

QString
Canvas3D::
addNewObject(Object3D *obj)
{
  addObject(obj);

  obj->setInd(++lastInd_);

  createObjCommand(obj);

  return obj->calcId();
}

void
Canvas3D::
addObject(Object3D *obj)
{
  objects_.push_back(obj);
}

void
Canvas3D::
removeObject(Object3D *obj)
{
  Objects objects;

  for (auto *obj1 : objects_) {
    if (obj != obj1)
      objects.push_back(obj);
  }

  std::swap(objects_, objects);
}

int
Canvas3D::
loadModelProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas3D *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);
  if (args.size() < 1) return TCL_ERROR;

  auto filename = args[0];

  QFileInfo fi(filename);

  if (! fi.exists()) {
    th->app()->errorMsg(QString("File '%1' does not exist").arg(filename));
    return TCL_ERROR;
  }

  auto *model = new Model3D(th);

  if (! model->load(filename)) {
    delete model;
    return TCL_ERROR;
  }

  auto name = th->addNewObject(model);

  auto *tcl = th->app()->tcl();

  tcl->setResult(name);

  return TCL_OK;
}

int
Canvas3D::
objectCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *obj = static_cast<Object3D *>(clientData);
  assert(obj);

  auto *canvas = obj->canvas();
  auto *app    = canvas->app();

  auto *tcl = app->tcl();

  auto args = app->getArgs(objc, objv);

  if      (args[0] == "get") {
    if (args.size() > 1) {
      auto name = args[1];

      QStringList args1;

      for (int i = 2; i < args.length(); ++i)
        args1.push_back(args[i]);

      auto res = obj->getValue(name, args1);

      tcl->setResult(res);
    }
    else {
      app->errorMsg("Missing args for get");
    }
  }
  else if (args[0] == "set") {
    if (args.size() > 2) {
      auto name  = args[1];
      auto value = args[2];

      QStringList args1;

      for (int i = 3; i < args.length(); ++i)
        args1.push_back(args[i]);

      obj->setValue(args[1], args[2], args1);
    }
    else {
      app->errorMsg("Missing args for set");
    }
  }
  else {
    app->errorMsg(QString("Bad object command '%1'").arg(args[0]));
  }

  return TCL_OK;
}

void
Canvas3D::
initialize()
{
  // camera
  camera_ = new CGLCamera(CGLVector3D(0.0f, 0.0f, 1.0f));

  camera_->setLastPos(float(this->width())/2.0f, float(this->height())/2.0f);

  //---

  if (! s_lightShaderProgram) {
    QString buildDir = QUOTE(BUILD_DIR);

    s_lightShaderProgram = new QOpenGLShaderProgram;

    if (! s_lightShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                        buildDir + "/shaders/light.vs"))
      std::cerr << s_lightShaderProgram->log().toStdString() << "\n";

    if (! s_lightShaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                                        buildDir + "/shaders/light.fs"))
      std::cerr << s_lightShaderProgram->log().toStdString() << "\n";

    s_lightShaderProgram->link();
  }

  //---

  // set up vertex data (and buffer(s)) and configure vertex attributes
  lightBuffer_ = new CQGLBuffer(s_lightShaderProgram);

  lightBuffer_->addPoint(-0.5f, -0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f, -0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f,  0.5f, -0.5f);

  lightBuffer_->addPoint( 0.5f,  0.5f, -0.5f);
  lightBuffer_->addPoint(-0.5f,  0.5f, -0.5f);
  lightBuffer_->addPoint(-0.5f, -0.5f, -0.5f);

  lightBuffer_->addPoint(-0.5f, -0.5f,  0.5f);
  lightBuffer_->addPoint( 0.5f, -0.5f,  0.5f);
  lightBuffer_->addPoint( 0.5f,  0.5f,  0.5f);

  lightBuffer_->addPoint( 0.5f,  0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f,  0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f, -0.5f,  0.5f);

  lightBuffer_->addPoint(-0.5f,  0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f,  0.5f, -0.5f);
  lightBuffer_->addPoint(-0.5f, -0.5f, -0.5f);

  lightBuffer_->addPoint(-0.5f, -0.5f, -0.5f);
  lightBuffer_->addPoint(-0.5f, -0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f,  0.5f,  0.5f);

  lightBuffer_->addPoint( 0.5f,  0.5f,  0.5f);
  lightBuffer_->addPoint( 0.5f,  0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f, -0.5f, -0.5f);

  lightBuffer_->addPoint( 0.5f, -0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f, -0.5f,  0.5f);
  lightBuffer_->addPoint( 0.5f,  0.5f,  0.5f);

  lightBuffer_->addPoint(-0.5f, -0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f, -0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f, -0.5f,  0.5f);

  lightBuffer_->addPoint( 0.5f, -0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f, -0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f, -0.5f, -0.5f);

  lightBuffer_->addPoint(-0.5f,  0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f,  0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f,  0.5f,  0.5f);

  lightBuffer_->addPoint( 0.5f,  0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f,  0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f,  0.5f, -0.5f);

  lightBuffer_->load();

  //---

  app_->runTclCmd("init");
}

void
Canvas3D::
timerSlot()
{
  auto objects = objects_;

  for (auto *obj : objects)
    obj->tick();

  app_->runTclCmd("update");

  update();
}

void
Canvas3D::
render()
{
  const qreal retinaScale = devicePixelRatio();

  glViewport(0, 0, width()*retinaScale, height()*retinaScale);

  glClearColor(bgColor_.redF(), bgColor_.greenF(), bgColor_.blueF(), 1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glDisable(GL_CULL_FACE);

  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  //---

  aspect_ = float(width())/float(height());

  projectionMatrix_ = CGLMatrix3D::perspective(camera_->zoom(), aspect_, 0.1f, 100.0f);
  viewMatrix_       = camera_->getViewMatrix();

  //---

  // lighting
  viewPos_ = camera_->position();

  //---

  for (auto *obj : objects_)
    obj->render();

  //---

  // setup light shader
  lightBuffer_->bind();

  s_lightShaderProgram->bind();

  s_lightShaderProgram->setUniformValue("projection",
    CQGLUtil::toQMatrix(projectionMatrix()));
  s_lightShaderProgram->setUniformValue("view", CQGLUtil::toQMatrix(viewMatrix()));

  auto lightMatrix = CGLMatrix3D::translation(lightPos());
  lightMatrix.scaled(0.01f, 0.01f, 0.01f);
  s_lightShaderProgram->setUniformValue("model", CQGLUtil::toQMatrix(lightMatrix));

  // draw light
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  //lightBuffer_->drawTriangles();

  lightBuffer_->unbind();
}

void
Canvas3D::
mousePressEvent(QMouseEvent *e)
{
  float xpos = float(e->x());
  float ypos = float(e->y());

  auto type = this->type();

  if (type == Type::CAMERA)
    camera_->setLastPos(xpos, ypos);

  update();

  pressed_ = true;
}

void
Canvas3D::
mouseReleaseEvent(QMouseEvent *)
{
  update();

  pressed_ = false;
}

void
Canvas3D::
mouseMoveEvent(QMouseEvent *e)
{
  if (pressed_) {
    float xpos = float(e->x());
    float ypos = float(e->y());

    //---

    auto type = this->type();

    if (type == Type::CAMERA) {
      float xoffset, yoffset;

      camera_->deltaPos(xpos, ypos, xoffset, yoffset);

      camera_->setLastPos(xpos, ypos);

      camera_->processMouseMovement(xoffset, yoffset);

      //Q_EMIT cameraChanged();
    }

    update();
  }
}

void
Canvas3D::
wheelEvent(QWheelEvent *e)
{
  float y = float(e->angleDelta().y()/250.0);

  auto type = this->type();

  if (type == Type::CAMERA)
    camera_->processMouseScroll(y);

  update();
}

void
Canvas3D::
keyPressEvent(QKeyEvent *e)
{
  bool isShift = (e->modifiers() & Qt::ShiftModifier);

  auto dt = 0.01f; /* camera_->deltaTime(); */
  auto da = M_PI/180.0;

  if (isShift) {
    dt = -dt;
    da = -da;
  }

  auto type = this->type();

  if      (e->key() == Qt::Key_W) {
    if      (type == Type::CAMERA) {
      camera_->processKeyboard(CGLCamera::Movement::FORWARD, dt);

      Q_EMIT cameraChanged();
    }
    else if (type == Type::LIGHT)
      lightPos_ += CGLVector3D(0.0f, 0.1f, 0.0f);
  }
  else if (e->key() == Qt::Key_S) {
    if      (type == Type::CAMERA) {
      camera_->processKeyboard(CGLCamera::Movement::BACKWARD, dt);

      Q_EMIT cameraChanged();
    }
    else if (type == Type::LIGHT)
      lightPos_ -= CGLVector3D(0.0f, 0.1f, 0.0f);
  }
  else if (e->key() == Qt::Key_A) {
    if      (type == Type::CAMERA) {
      camera_->processKeyboard(CGLCamera::Movement::LEFT, dt);

      Q_EMIT cameraChanged();
    }
    else if (type == Type::LIGHT)
      lightPos_ -= CGLVector3D(0.1f, 0.0f, 0.0f);
  }
  else if (e->key() == Qt::Key_D) {
    if      (type == Type::CAMERA) {
      camera_->processKeyboard(CGLCamera::Movement::RIGHT, dt);

      Q_EMIT cameraChanged();
    }
    else if (type == Type::LIGHT)
      lightPos_ += CGLVector3D(0.1f, 0.0f, 0.0f);
  }
  else if (e->key() == Qt::Key_C) {
    setType(Type::CAMERA);
  }
  else if (e->key() == Qt::Key_L) {
    setType(Type::LIGHT);
  }
  else if (e->key() == Qt::Key_M) {
    setType(Type::MODEL);
  }
  else if (e->key() == Qt::Key_X) {
    if (type == Type::MODEL) {
      for (auto *obj : objects_)
        obj->setXAngle(obj->xAngle() + da);
    }
  }
  else if (e->key() == Qt::Key_Y) {
    if (type == Type::MODEL) {
      for (auto *obj : objects_)
        obj->setYAngle(obj->yAngle() + da);
    }
  }
  else if (e->key() == Qt::Key_Z) {
   if (type == Type::MODEL) {
      for (auto *obj : objects_)
        obj->setZAngle(obj->zAngle() + da);
    }
  }
  else if (e->key() == Qt::Key_Up) {
    if (type == Type::LIGHT)
      lightPos_ += CGLVector3D(0.0f, 0.0f, 0.1f);
  }
  else if (e->key() == Qt::Key_Down) {
    if (type == Type::LIGHT)
      lightPos_ -= CGLVector3D(0.0f, 0.0f, 0.1f);
  }

  update();
}

void
Canvas3D::
checkShaderErr(int shader)
{
  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (! success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
}

void
Canvas3D::
checkProgramErr(int program)
{
  int success;
  glGetProgramiv(program, GL_COMPILE_STATUS, &success);
  if (! success) {
    char infoLog[512];
    glGetShaderInfoLog(program, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
}

//---

Object3D::
Object3D(Canvas3D *canvas) :
 canvas_(canvas)
{
}

QString
Object3D::
getCommandName() const
{
  return QString("object3d.%1").arg(ind_);
}

QString
Object3D::
calcId() const
{
  auto id = this->id();

  if (id == "")
    id = getCommandName();

  return id;
}

void
Object3D::
setPosition(const CPoint3D &p)
{
  position_ = p;

  updateModelMatrix();

  needsUpdate_ = true;
}

QVariant
Object3D::
getValue(const QString &name, const QStringList &)
{
  auto *app = canvas()->app();

  if (name == "id")
    return id();
  else {
    app->errorMsg(QString("Invalid get name '%1'").arg(name));
    return QVariant();
  }
}

void
Object3D::
setValue(const QString &name, const QString &value, const QStringList &)
{
  auto *app = canvas()->app();

  if (name == "id")
    setId(value);
  else
    app->errorMsg(QString("Invalid set name '%1'").arg(name));
}

void
Object3D::
render()
{
}

//---

QOpenGLShaderProgram* Model3D::s_modelShaderProgram;

Model3D::
Model3D(Canvas3D *canvas) :
 Object3D(canvas)
{
  if (! s_modelShaderProgram) {
    QString buildDir = QUOTE(BUILD_DIR);

    s_modelShaderProgram = new QOpenGLShaderProgram;

    if (! s_modelShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                        buildDir + "/shaders/model.vs"))
      std::cerr << s_modelShaderProgram->log().toStdString() << "\n";

    if (! s_modelShaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                                        buildDir + "/shaders/model.fs"))
      std::cerr << s_modelShaderProgram->log().toStdString() << "\n";

    s_modelShaderProgram->link();
  }
}

bool
Model3D::
load(const QString &filename)
{
  filename_ = filename;

  QFileInfo fi(filename);

  auto suffix = fi.suffix().toLower();
  auto type   = CImportBase::suffixToType(suffix.toStdString());

  import_ = CImportBase::createModel(type);

  if (! import_) {
    canvas_->app()->errorMsg(QString("Invalid model type for '%1'").arg(filename_));
    return false;
  }

  CFile file(filename_.toStdString());

  if (! import_->read(file)) {
    canvas_->app()->errorMsg(QString("Failed to load file '%1'").arg(filename_));
    return false;
  }

  updateObjectData();

  return true;
}

void
Model3D::
render()
{
  // lighting
  CGLVector3D lightColor(1.0f, 1.0f, 1.0f);

  //---

  modelMatrix_ = CGLMatrix3D::identity();
  modelMatrix_.scaled(sceneScale_, sceneScale_, sceneScale_);
  modelMatrix_.rotated(xAngle(), CGLVector3D(1.0, 0.0, 0.0));
  modelMatrix_.rotated(yAngle(), CGLVector3D(0.0, 1.0, 0.0));
  modelMatrix_.rotated(zAngle(), CGLVector3D(0.0, 0.0, 1.0));
  modelMatrix_.translated(float(-sceneCenter_.getX()),
                          float(-sceneCenter_.getY()),
                          float(-sceneCenter_.getZ()));
  // setup model shader
  for (auto &po : objectDatas_) {
    auto *objectData = po.second;

    objectData->buffer->bind();

    s_modelShaderProgram->bind();

    s_modelShaderProgram->setUniformValue("lightColor", CQGLUtil::toVector(lightColor));
    s_modelShaderProgram->setUniformValue("lightPos"  , CQGLUtil::toVector(canvas_->lightPos()));
    s_modelShaderProgram->setUniformValue("viewPos"   , CQGLUtil::toVector(canvas_->viewPos()));

    s_modelShaderProgram->setUniformValue("ambientStrength" , float(canvas_->ambient()));
    s_modelShaderProgram->setUniformValue("diffuseStrength" , float(canvas_->diffuse()));
    s_modelShaderProgram->setUniformValue("specularStrength", float(canvas_->specular()));
    s_modelShaderProgram->setUniformValue("shininess"       , float(canvas_->shininess()));

    // pass projection matrix to shader (note that in this case it could change every frame)
    s_modelShaderProgram->setUniformValue("projection",
      CQGLUtil::toQMatrix(canvas_->projectionMatrix()));

    // camera/view transformation
    s_modelShaderProgram->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

    // model rotation
    s_modelShaderProgram->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

    // render model
    for (const auto &faceData : objectData->faceDatas) {
      bool useDiffuseTexture = faceData.diffuseTexture;

      s_modelShaderProgram->setUniformValue("useDiffuseTexture", useDiffuseTexture);

      if (useDiffuseTexture) {
        glActiveTexture(GL_TEXTURE0);
        faceData.diffuseTexture->bind();

        s_modelShaderProgram->setUniformValue("diffuseTexture", 0);
      }

      //---

      bool useSpecularTexture = faceData.specularTexture;

      s_modelShaderProgram->setUniformValue("useSpecularTexture", useSpecularTexture);

      if (useSpecularTexture) {
        glActiveTexture(GL_TEXTURE1);
        faceData.specularTexture->bind();

        s_modelShaderProgram->setUniformValue("specularTexture", 1);
      }

      //---

      bool useNormalTexture = faceData.normalTexture;

      s_modelShaderProgram->setUniformValue("useNormalTexture", useNormalTexture);

      if (useNormalTexture) {
        glActiveTexture(GL_TEXTURE2);
        faceData.normalTexture->bind();

        s_modelShaderProgram->setUniformValue("normalTexture", 2);
      }

      //---

      s_modelShaderProgram->setUniformValue("isWireframe", 0);

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      glDrawArrays(GL_TRIANGLE_FAN, faceData.pos, faceData.len);

      if (canvas_->isPolygonLine()) {
        s_modelShaderProgram->setUniformValue("isWireframe", 1);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glDrawArrays(GL_TRIANGLE_FAN, faceData.pos, faceData.len);
      }
    }

    objectData->buffer->unbind();
  }
}

void
Model3D::
updateObjectData()
{
  // set up vertex data (and buffer(s)) and configure vertex attributes
  CVector3D sceneSize(1, 1, 1);

  if (import_) {
    auto &scene = import_->getScene();

    CBBox3D bbox;

    scene.getBBox(bbox);

    sceneSize    = bbox.getSize();
    sceneCenter_ = bbox.getCenter();
    //std::cerr << "Scene Center : " << sceneCenter_.getX() << " " <<
    //             sceneCenter_.getY() << " " << sceneCenter_.getZ() << "\n";

    for (auto *object : scene.getObjects()) {
      ObjectData *objectData { nullptr };

      auto pd = objectDatas_.find(object);

      if (pd == objectDatas_.end())
        pd = objectDatas_.insert(pd, ObjectDatas::value_type(object, new ObjectData));

      objectData = (*pd).second;

      if (! objectData->buffer)
        objectData->buffer = new CQGLBuffer(s_modelShaderProgram);

      //---

      auto *buffer = objectData->buffer;

      buffer->clearPoints();
      buffer->clearNormals();
      buffer->clearColors();
      buffer->clearTexturePoints();

      objectData->faceDatas.clear();

      //---

      const auto &faces = object->getFaces();

      int pos = 0;

      for (const auto *face : faces) {
        FaceData faceData;

        //---

        const auto &color = face->getColor();

        auto *diffuseTexture  = face->getDiffuseTexture();
        auto *specularTexture = face->getSpecularTexture();
        auto *normalTexture   = face->getNormalTexture();

        if (diffuseTexture) {
          auto pt = glTextures_.find(diffuseTexture->id());

          if (pt == glTextures_.end()) {
            const auto &image = diffuseTexture->image()->image();

            auto *glTexture = new CGLTexture(image);

            pt = glTextures_.insert(pt, GLTextures::value_type(diffuseTexture->id(), glTexture));
          }

          faceData.diffuseTexture = (*pt).second;
        }

        if (specularTexture) {
          auto pt = glTextures_.find(specularTexture->id());

          if (pt == glTextures_.end()) {
            const auto &image = specularTexture->image()->image();

            auto *glTexture = new CGLTexture(image);

            pt = glTextures_.insert(pt, GLTextures::value_type(specularTexture->id(), glTexture));
          }

          faceData.specularTexture = (*pt).second;
        }

        if (normalTexture) {
          auto pt = glTextures_.find(normalTexture->id());

          if (pt == glTextures_.end()) {
            const auto &image = normalTexture->image()->image();

            auto *glTexture = new CGLTexture(image);

            pt = glTextures_.insert(pt, GLTextures::value_type(normalTexture->id(), glTexture));
          }

          faceData.normalTexture = (*pt).second;
        }

      //const auto &ambient   = face->getMaterial().getAmbient  ();
      //const auto &diffuse   = face->getMaterial().getDiffuse  ();
      //const auto &specular  = face->getMaterial().getSpecular ();
      //double      shininess = face->getMaterial().getShininess();

        CVector3D normal;

        if (face->getNormalSet())
          normal = face->getNormal();
        else
          face->calcNormal(normal);

        const auto &vertices = face->getVertices();

        faceData.pos = pos;
        faceData.len = int(vertices.size());

        for (const auto &v : vertices) {
          auto &vertex = object->getVertex(v);

          const auto &model = vertex.getModel();

          if (! flipYZ_) {
            buffer->addPoint(float(model.x), float(model.y), float(model.z));
            buffer->addNormal(float(normal.getX()), float(normal.getY()), float(normal.getZ()));
          }
          else {
            buffer->addPoint(float(model.x), float(model.z), float(model.y));
            buffer->addNormal(float(normal.getX()), float(normal.getZ()), float(normal.getY()));
          }

          buffer->addColor(color.getRedF(), color.getGreenF(), color.getBlueF());

          if (faceData.diffuseTexture) {
            const auto &tpoint = vertex.getTextureMap();

            buffer->addTexturePoint(float(tpoint.x), float(tpoint.y));
          }
          else
            buffer->addTexturePoint(0.0f, 0.0f);
        }

        pos += faceData.len;

        objectData->faceDatas.push_back(faceData);
      }

      objectData->buffer->load();
    }
  }

  auto max3 = [](double x, double y, double z) {
    return std::max(std::max(x, y), z);
  };

  sceneScale_ = float(1.0/max3(sceneSize.getX(), sceneSize.getY(), sceneSize.getZ()));
  //std::cerr << "Scene Scale : " << sceneScale_ << "\n";
}

//---

ProgramData *Shape3DObj::s_program = nullptr;

bool
Shape3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Shape3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Shape3DObj::
Shape3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

QVariant
Shape3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

void
Shape3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas_->app()->tcl();

  if      (name == "points") {
    points_ = stringToVectors3D(tcl, value);

    needsUpdate_ = true;
  }
  else if (name == "indices") {
    indices_ = stringToUIntArray(tcl, value);

    needsUpdate_ = true;
  }
  else if (name == "colors") {
    colors_ = stringToVectors3D(tcl, value);

    needsUpdate_ = true;
  }
  else if (name == "tex_coords") {
    texCoords_ = stringToVectors2D(tcl, value);

    needsUpdate_ = true;
  }
  else if (name == "texture") {
    setTexture(value);

    needsUpdate_ = true;
  }
  else if (name == "scale") {
    scale_ = Util::stringToReal(value);

    needsUpdate_ = true;
  }
  else if (name == "angle") {
    auto p = stringToPoint3D(tcl, value);

    xAngle_ = p.getX();
    yAngle_ = p.getY();
    zAngle_ = p.getZ();

    needsUpdate_ = true;
  }
  else if (name == "position") {
    setPosition(stringToPoint3D(tcl, value));
  }
  else if (name == "wireframe") {
    wireframe_ = Util::stringToBool(value);

    needsUpdate_ = true;
  }
  else if (name == "cone") {
    QStringList strs;
    (void) tcl->splitList(value, strs);
    if (strs.size() != 2) return;

    double r = Util::stringToReal(strs[0]);
    double h = Util::stringToReal(strs[1]);

    addCone(r, h);

    needsUpdate_ = true;
  }
  else if (name == "sphere") {
    double r = Util::stringToReal(value);

    addSphere(r);
  }
  else if (name == "cube") {
    double s = Util::stringToReal(value);

    addCube(s);
  }
  else
    Object3D::setValue(name, value, args);
}

void
Shape3DObj::
addCone(double r, double h)
{
  uint stacks = 20;
  uint slices = 20;

  std::vector<double> x, y;

  x.resize(stacks);
  y.resize(stacks);

  x[0         ] = r; y[0         ] = 0;
  x[stacks - 1] = 0; y[stacks - 1] = h;

  double dx = stacks > 2 ? (x[stacks - 1] - x[0])/(stacks - 1) : 0;
  double dy = stacks > 2 ? (y[stacks - 1] - y[0])/(stacks - 1) : 0;

  for (uint i = 1; i < stacks - 1; ++i) {
    x[i] = x[i - 1] + dx;
    y[i] = y[i - 1] + dy;
  }

  addBodyRev(&x[0], &y[0], stacks, slices);
}

void
Shape3DObj::
addSphere(double radius)
{
  uint stacks = 20;
  uint slices = 20;

  std::vector<double> x, y;

  x.resize(stacks);
  y.resize(stacks);

  double a = -M_PI*0.5;

  double da = M_PI/(stacks - 1);

  for (uint i = 0; i < stacks; ++i) {
    x[i] = radius*std::cos(a);
    y[i] = radius*std::sin(a);

    a += da;
  }

  addBodyRev(&x[0], &y[0], stacks, slices);
}

void
Shape3DObj::
addCube(double s)
{
  static CGLVector3D cube_normal[6] = {
    {-1.0,  0.0,  0.0},
    { 0.0,  1.0,  0.0},
    { 1.0,  0.0,  0.0},
    { 0.0, -1.0,  0.0},
    { 0.0,  0.0,  1.0},
    { 0.0,  0.0, -1.0}
  };

  static GLint cube_faces[6][4] = {
    {0, 1, 2, 3},
    {3, 2, 6, 7},
    {7, 6, 5, 4},
    {4, 5, 1, 0},
    {5, 6, 2, 1},
    {7, 4, 0, 3}
  };

  double xs = s/2.0;
  double ys = s/2.0;
  double zs = s/2.0;

  struct Point {
    float x;
    float y;
    float z;
  };

  std::vector<Point> v; v.resize(8);

  v[0].x = v[1].x = v[2].x = v[3].x = -xs/2;
  v[4].x = v[5].x = v[6].x = v[7].x =  xs/2;
  v[0].y = v[1].y = v[4].y = v[5].y = -ys/2;
  v[2].y = v[3].y = v[6].y = v[7].y =  ys/2;
  v[0].z = v[3].z = v[4].z = v[7].z = -zs/2;
  v[1].z = v[2].z = v[5].z = v[6].z =  zs/2;

  points_   .resize(36);
  texCoords_.resize(36);
  normals_  .resize(36);

  int k = 0;

  for (int i = 5; i >= 0; --i) {
    const auto &normal = cube_normal[i];

    auto addPoint = [&](const Point &p, const CGLVector2D &t) {
      points_   [k] = CGLVector3D(p.x, p.y, p.z);
      texCoords_[k] = t;
      normals_  [k] = normal;
      ++k;
    };

    addPoint(v[cube_faces[i][0]], CGLVector2D(0.0, 0.0));
    addPoint(v[cube_faces[i][1]], CGLVector2D(1.0, 0.0));
    addPoint(v[cube_faces[i][2]], CGLVector2D(1.0, 1.0));

    addPoint(v[cube_faces[i][2]], CGLVector2D(1.0, 1.0));
    addPoint(v[cube_faces[i][3]], CGLVector2D(0.0, 1.0));
    addPoint(v[cube_faces[i][0]], CGLVector2D(0.0, 0.0));
  }
}

void
Shape3DObj::
addBodyRev(double *x, double *y, uint num_xy, uint num_patches)
{
  std::vector<VertexData>   vertices;
  std::vector<unsigned int> indices;

  addBodyRev(&x[0], &y[0], num_xy, num_patches, vertices, indices);

  auto np = vertices.size();
  auto ni = indices.size();

  points_ .resize(np);
  normals_.resize(np);
  indices_.resize(ni);

  for (uint i = 0; i < np; ++i) {
    points_ [i] = vertices[i].position;
    normals_[i] = vertices[i].normal;
  }

  for (uint i = 0; i < ni; ++i) {
    indices_[i] = indices[i];
  }
}

void
Shape3DObj::
addBodyRev(double *x, double *y, uint num_xy, uint num_patches,
           std::vector<VertexData> &vertexDatas, std::vector<unsigned int> &indices)
{
  std::vector<double> c, s;

  c.resize(num_patches);
  s.resize(num_patches);

  double theta           = 0.0;
  double theta_increment = 2.0*M_PI/num_patches;

  for (uint i = 0; i < num_patches; i++) {
    c[i] = cos(theta);
    s[i] = sin(theta);

    theta += theta_increment;
  }

  uint num_vertices = 0;

  std::vector<uint> index1, index2;

  index1.resize(num_patches + 1);
  index2.resize(num_patches + 1);

  uint *pindex1 = &index1[0];
  uint *pindex2 = &index2[0];

  if (fabs(x[0]) < 1E-6) {
    CGLVector3D p(0.0, float(y[0]), 0.0);

    vertexDatas.push_back(VertexData(p, p, CGLVector2D(0, 0)));

    for (uint i = 0; i <= num_patches; i++)
      pindex1[i] = num_vertices;

    num_vertices++;
  }
  else {
    for (uint i = 0; i < num_patches; i++) {
      CGLVector3D p(float(x[0]*c[i]), float(y[0]), float(-x[0]*s[i]));

      vertexDatas.push_back(VertexData(p, p, CGLVector2D(0, 0)));

      pindex1[i] = num_vertices;

      num_vertices++;
    }

    pindex1[num_patches] = pindex1[0];
  }

  for (uint j = 1; j < num_xy; j++) {
    double ty = (num_xy > 1 ? 1.0*j/(num_xy - 1) : 0.0);

    if (fabs(x[j]) < 1E-6) {
      CGLVector3D p(0.0, float(y[j]), 0.0);

      vertexDatas.push_back(VertexData(p, p, CGLVector2D(0, float(ty))));

      for (uint i = 0; i <= num_patches; ++i)
        pindex2[i] = num_vertices;

      ++num_vertices;
    }
    else {
      for (uint i = 0; i < num_patches; i++) {
        double tx = (num_patches > 1 ? 1.0*i/(num_patches - 1) : 0.0);

        CGLVector3D p(float(x[j]*c[i]), float(y[j]), float(-x[j]*s[i]));

        vertexDatas.push_back(VertexData(p, p, CGLVector2D(float(tx), float(ty))));

        pindex2[i] = num_vertices;

        num_vertices++;
      }

      pindex2[num_patches] = pindex2[0];
    }

    if (pindex1[0] != pindex1[1]) {
      if (pindex2[0] == pindex2[1]) {
        for (uint i = 0; i < num_patches; ++i) {
          indices.push_back(GLushort(pindex1[i + 1]));
          indices.push_back(GLushort(pindex2[i    ]));
          indices.push_back(GLushort(pindex1[i    ]));
        }
      }
      else {
        for (uint i = 0; i < num_patches; ++i) {
          indices.push_back(GLushort(pindex1[i + 1]));
          indices.push_back(GLushort(pindex2[i + 1]));
          indices.push_back(GLushort(pindex2[i    ]));

          indices.push_back(GLushort(pindex1[i + 1]));
          indices.push_back(GLushort(pindex2[i    ]));
          indices.push_back(GLushort(pindex1[i    ]));
        }
      }
    }
    else {
      if (pindex2[0] != pindex2[1]) {
        for (uint i = 0; i < num_patches; ++i) {
          indices.push_back(GLushort(pindex2[i + 1]));
          indices.push_back(GLushort(pindex2[i    ]));
          indices.push_back(GLushort(pindex1[i    ]));
        }
      }
    }

    uint *pindex = pindex2;

    pindex2 = pindex1;
    pindex1 = pindex;
  }
}

void
Shape3DObj::
setTexture(const QString &filename)
{
  texture_ = new CQGLTexture;

  if (! texture_->load(filename, /*flip*/true)) {
    delete texture_;
    texture_ = nullptr;
  }
}

void
Shape3DObj::
init()
{
  modelMatrix_ = CGLMatrix3D::identity();

  //---

  if (! s_program) {
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "layout (location = 1) in vec3 aNormal;\n"
      "layout (location = 2) in vec3 aColor;\n"
      "layout (location = 3) in vec2 aTexCoord;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "out vec3 FragPos;\n"
      "out vec3 Normal;\n"
      "out vec3 Color;\n"
      "out vec2 TexCoord;\n"
      "void main() {\n"
      "  FragPos  = vec3(model * vec4(aPos, 1.0));\n"
      "  Normal   = mat3(transpose(inverse(model)))*aNormal;\n"
      "  Color    = aColor;\n"
      "  TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
      "  gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
      "}";
    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "in vec3 FragPos;\n"
      "in vec3 Normal;\n"
      "in vec3 Color;\n"
      "in vec2 TexCoord;\n"
      "out vec4 FragColor;\n"
      "uniform vec3 lightPos;\n"
      "uniform vec3 viewPos;\n"
      "uniform vec3 lightColor;\n"
      "uniform float ambientStrength;\n"
      "uniform float diffuseStrength;\n"
      "uniform float specularStrength;\n"
      "uniform float shininess;\n"
      "uniform sampler2D textureId;\n"
      "uniform bool useTexture;\n"
      "void main() {\n"
      "  vec3 norm = normalize(Normal);\n"
      "  vec3 lightDir = normalize(lightPos - FragPos);\n"
      "  float diff = max(dot(norm, lightDir), 0.0);\n"
      "  vec3 diffuseColor = Color;\n"
      "  if (useTexture) {\n"
      "    diffuseColor = texture(textureId, TexCoord).xyz;\n"
      "  }\n"
      "  vec3 diffuse = diffuseStrength*diff*diffuseColor;\n"
      "  vec3 ambient = ambientStrength*diffuseColor;\n"
      "  vec3 viewDir = normalize(viewPos - FragPos);\n"
      "  vec3 reflectDir = reflect(-lightDir, norm);\n"
      "  float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);\n"
      "  vec3 specColor = lightColor;\n"
      "  vec3 specular = specularStrength*spec*specColor;\n"
      "  vec3 result = ambient + diffuse + specular;\n"
      "  FragColor = vec4(result, 1.0f);\n"
      "}\n";

    s_program = new ProgramData;

    s_program->program = new QOpenGLShaderProgram(this);

    s_program->program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShaderSource);
    s_program->program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    s_program->program->link();
  }

  //---

  canvas_->glGenVertexArrays(1, &vertexArrayId_);

  canvas_->glGenBuffers(1, &pointsBufferId_);
  canvas_->glGenBuffers(1, &normalsBufferId_);
  canvas_->glGenBuffers(1, &colorsBufferId_);
  canvas_->glGenBuffers(1, &texCoordBufferId_);
  canvas_->glGenBuffers(1, &indBufferId_);
}

void
Shape3DObj::
updateGL()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  //---

  calcNormals();

  //---

  // bind the Vertex Array Object
  canvas_->glBindVertexArray(vertexArrayId_);

  //---

  int np = points_.size();

  // store point data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, pointsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D), &points_[0], GL_STATIC_DRAW);

  // set points attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(0);

  // store normal data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, normalsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D), &normals_[0], GL_STATIC_DRAW);

  // set normals attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(1);

  //---

  int nc = colors_.size();

  // store color data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, colorsBufferId_);
  if (nc > 0) {
    canvas_->glBufferData(GL_ARRAY_BUFFER, nc*sizeof(CGLVector3D), &colors_[0], GL_STATIC_DRAW);
  }
  else {
    static Colors s_colors_;

    if (int(s_colors_.size()) != np) {
      s_colors_.resize(np);

      for (int i = 0; i < np; ++i) {
        s_colors_[i] = CGLVector3D(1.0, 1.0, 1.0);
      }
    }

    canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D),
                          &s_colors_[0], GL_STATIC_DRAW);
  }

  // set colors attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(2);

  //---

  int nt = texCoords_.size();

  useTexture_ = (texture_ && nt > 0);

  // store point data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, texCoordBufferId_);
  if (useTexture_) {
    canvas_->glBufferData(GL_ARRAY_BUFFER, nt*sizeof(CGLVector2D), &texCoords_[0], GL_STATIC_DRAW);
  }
  else {
    static TexCoords s_texCoords;

    if (int(s_texCoords.size()) != np) {
      s_texCoords.resize(np);

      for (int i = 0; i < np; ++i)
        s_texCoords[i] = CGLVector2D(0, 0);
    }

    canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector2D),
                          &s_texCoords[0], GL_STATIC_DRAW);
  }

  // set points attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(CGLVector2D), nullptr);
  canvas_->glEnableVertexAttribArray(3);

  //---

  // store index data in element buffer
  int ni = indices_.size();

  if (ni > 0) {
    canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufferId_);
    canvas_->glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni*sizeof(unsigned int),
                          &indices_[0], GL_STATIC_DRAW);
  }

  //---

  canvas_->glBindBuffer(GL_ARRAY_BUFFER, 0);
//canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,  0);

  canvas_->glBindVertexArray(0);
}

void
Shape3DObj::
calcNormals()
{
  auto np = points_.size();

  if (normals_.size() != np) {
    normals_.resize(np);

    for (uint i = 0; i < np; ++i)
      normals_[i] = CGLVector3D(0, 0, 1);
  }
}

void
Shape3DObj::
render()
{
  updateGL();

  if (wireframe_ || canvas_->isWireframe())
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  //---

  CGLVector3D lightColor(1.0f, 1.0f, 1.0f);

  auto *program = s_program->program;

  program->bind();

  program->setUniformValue("lightColor", CQGLUtil::toVector(lightColor));
  program->setUniformValue("lightPos"  , CQGLUtil::toVector(canvas_->lightPos()));
  program->setUniformValue("viewPos"   , CQGLUtil::toVector(canvas_->viewPos()));

  program->setUniformValue("ambientStrength" , float(canvas_->ambient()));
  program->setUniformValue("diffuseStrength" , float(canvas_->diffuse()));
  program->setUniformValue("specularStrength", float(canvas_->specular()));
  program->setUniformValue("shininess"       , float(canvas_->shininess()));

  program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  modelMatrix_ = CGLMatrix3D::identity();
  modelMatrix_.translated(float(position().getX()),
                          float(position().getY()),
                          float(position().getZ()));
  modelMatrix_.scaled(scale(), scale(), scale());
  modelMatrix_.rotated(xAngle(), CGLVector3D(1.0, 0.0, 0.0));
  modelMatrix_.rotated(yAngle(), CGLVector3D(0.0, 1.0, 0.0));
  modelMatrix_.rotated(zAngle(), CGLVector3D(0.0, 0.0, 1.0));

  program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

  //---

  canvas_->glBindVertexArray(vertexArrayId_);

  //---

  s_program->program->setUniformValue("useTexture", useTexture_);
  s_program->program->setUniformValue("textureId", 0);

  if (useTexture_) {
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);

    texture_->bind();
  }

  int np = points_.size();
  int ni = indices_.size();

  if (ni > 0)
    glDrawElements(GL_TRIANGLES, ni, GL_UNSIGNED_INT, nullptr);
  else {
    if      (useTriangleStrip_)
      glDrawArrays(GL_TRIANGLE_STRIP, 0, np);
    else if (useTriangleFan_)
      glDrawArrays(GL_TRIANGLE_FAN, 0, np);
    else
      glDrawArrays(GL_TRIANGLES, 0, np);
  }

  //canvas_->glBindVertexArray(0);

  if (texture_) {
    glDisable(GL_TEXTURE_2D);
  }
}

//---

bool
CubeObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new CubeObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

CubeObj::
CubeObj(Canvas3D *canvas) :
 Shape3DObj(canvas)
{
}

void
CubeObj::
init()
{
  static Points points = {
    CGLVector3D(-0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f,  0.5f, -0.5f),
    CGLVector3D( 0.5f,  0.5f, -0.5f),
    CGLVector3D(-0.5f,  0.5f, -0.5f),
    CGLVector3D(-0.5f, -0.5f, -0.5f),

    CGLVector3D(-0.5f, -0.5f,  0.5f),
    CGLVector3D( 0.5f, -0.5f,  0.5f),
    CGLVector3D( 0.5f,  0.5f,  0.5f),
    CGLVector3D( 0.5f,  0.5f,  0.5f),
    CGLVector3D(-0.5f,  0.5f,  0.5f),
    CGLVector3D(-0.5f, -0.5f,  0.5f),

    CGLVector3D(-0.5f,  0.5f,  0.5f),
    CGLVector3D(-0.5f,  0.5f, -0.5f),
    CGLVector3D(-0.5f, -0.5f, -0.5f),
    CGLVector3D(-0.5f, -0.5f, -0.5f),
    CGLVector3D(-0.5f, -0.5f,  0.5f),
    CGLVector3D(-0.5f,  0.5f,  0.5f),

    CGLVector3D( 0.5f,  0.5f,  0.5f),
    CGLVector3D( 0.5f,  0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f,  0.5f),
    CGLVector3D( 0.5f,  0.5f,  0.5f),

    CGLVector3D(-0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f,  0.5f),
    CGLVector3D( 0.5f, -0.5f,  0.5f),
    CGLVector3D(-0.5f, -0.5f,  0.5f),
    CGLVector3D(-0.5f, -0.5f, -0.5f),

    CGLVector3D(-0.5f,  0.5f, -0.5f),
    CGLVector3D( 0.5f,  0.5f, -0.5f),
    CGLVector3D( 0.5f,  0.5f,  0.5f),
    CGLVector3D( 0.5f,  0.5f,  0.5f),
    CGLVector3D(-0.5f,  0.5f,  0.5f),
    CGLVector3D(-0.5f,  0.5f, -0.5f)
  };

  static TexCoords texCoords = {
    CGLVector2D(0.0f, 0.0f),
    CGLVector2D(1.0f, 0.0f),
    CGLVector2D(1.0f, 1.0f),
    CGLVector2D(1.0f, 1.0f),
    CGLVector2D(0.0f, 1.0f),
    CGLVector2D(0.0f, 0.0f),

    CGLVector2D(0.0f, 0.0f),
    CGLVector2D(1.0f, 0.0f),
    CGLVector2D(1.0f, 1.0f),
    CGLVector2D(1.0f, 1.0f),
    CGLVector2D(0.0f, 1.0f),
    CGLVector2D(0.0f, 0.0f),

    CGLVector2D(1.0f, 0.0f),
    CGLVector2D(1.0f, 1.0f),
    CGLVector2D(0.0f, 1.0f),
    CGLVector2D(0.0f, 1.0f),
    CGLVector2D(0.0f, 0.0f),
    CGLVector2D(1.0f, 0.0f),

    CGLVector2D(1.0f, 0.0f),
    CGLVector2D(1.0f, 1.0f),
    CGLVector2D(0.0f, 1.0f),
    CGLVector2D(0.0f, 1.0f),
    CGLVector2D(0.0f, 0.0f),
    CGLVector2D(1.0f, 0.0f),

    CGLVector2D(0.0f, 1.0f),
    CGLVector2D(1.0f, 1.0f),
    CGLVector2D(1.0f, 0.0f),
    CGLVector2D(1.0f, 0.0f),
    CGLVector2D(0.0f, 0.0f),
    CGLVector2D(0.0f, 1.0f),

    CGLVector2D(0.0f, 1.0f),
    CGLVector2D(1.0f, 1.0f),
    CGLVector2D(1.0f, 0.0f),
    CGLVector2D(1.0f, 0.0f),
    CGLVector2D(0.0f, 0.0f),
    CGLVector2D(0.0f, 1.0f)
  };

  points_    = points;
  texCoords_ = texCoords;

  Shape3DObj::init();
}

//---

bool
ShaderObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new ShaderObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

ShaderObj::
ShaderObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

void
ShaderObj::
init()
{
  fragmentShader_ = QString("\
void mainImage(out vec4 fragColor, in vec2 fragCoord) {\n\
  vec2 uv = fragCoord/iResolution;\n\
  fragColor = vec4(uv.x, uv.y, 0.0, 1.0);\n\
}\n\
");

  vertexShader_ = QString("\
attribute vec4 a_Position;\n\
attribute vec2 a_Coordinates;\n\
\n\
void main() {\n\
  gl_Position = vec4(a_Coordinates.x, a_Coordinates.y, 1.0f, 1.0f);\n\
}");
}

QVariant
ShaderObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

void
ShaderObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "vertex_shader") {
    vertexShader_ = value;
    shaderValid_  = false;
  }
  else if (name == "fragment_shader") {
    fragmentShader_ = value;
    shaderValid_    = false;
  }
  else
    Object3D::setValue(name, value, args);
}

void
ShaderObj::
updateShaders()
{
  if (shaderValid_)
    return;

  //---

  auto fragmentShader = QString("\
#version 330\n\
\n\
uniform float iTime;\n\
uniform int   iFrame;\n\
uniform vec2  iResolution;\n\
\n");

  fragmentShader += fragmentShader_;

  fragmentShader += QString("\n\
void main() {\n\
  mainImage(gl_FragColor, gl_FragCoord.xy);\n\
}\n");

  //---

  auto vertexShader = QString("\
#version 330\n\
\n");

  vertexShader += vertexShader_;

  program_ = new QOpenGLShaderProgram(this);

  program_->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShader);
  program_->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader);

  program_->link();

  //---

  shaderValid_ = true;
}

void
ShaderObj::
tick()
{
  elapsed_ += canvas_->redrawTimeOut()/1000.0;

  ++frame_;
}

void
ShaderObj::
render()
{
  updateShaders();

  program_->bind();

  //---

  int elapsedLocation = program_->uniformLocation("iTime");
  program_->setUniformValue(elapsedLocation, GLfloat(elapsed_));

  //---

  int frameLocation = program_->uniformLocation("iFrame");
  program_->setUniformValue(frameLocation, GLint(frame_));

  //---

  int resLocation = program_->uniformLocation("iResolution");

  program_->setUniformValue(resLocation, GLfloat(canvas_->width()), GLfloat(canvas_->height()));

  //---

  QVector3D vertices[] = {
    QVector3D(-1.0f, -1.0f, 1.0f),
    QVector3D( 1.0f, -1.0f, 1.0f),
    QVector3D( 1.0f,  1.0f, 1.0f),
    QVector3D( 1.0f,  1.0f, 1.0f),
    QVector3D(-1.0f,  1.0f, 1.0f),
    QVector3D(-1.0f, -1.0f, 1.0f)
  };

  int coordsLocation = program_->attributeLocation("a_Coordinates");

  program_->enableAttributeArray(coordsLocation);

  program_->setAttributeArray(coordsLocation, vertices);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  program_->disableAttributeArray(coordsLocation);

  program_->release();
}

//---

size_t                                    ParticleListObj::s_maxPoints = 10000;
ParticleListObj::ParticleListProgramData *ParticleListObj::s_program   = nullptr;

bool
ParticleListObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new ParticleListObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

ParticleListObj::
ParticleListObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

QVariant
ParticleListObj::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "size") {
    return Util::intToString(int(points_.size()));
  }
  else if (name == "position") {
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(points_.size()))
        return QVariant();

      return vector3DToString(points_[i]);
    }
    else
      return QVariant();
  }
  else if (name == "color") {
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(colors_.size()))
        return QVariant();

      return colorToString(colors_[i]);
    }
    else
      return QVariant();
  }
  else
    return Object3D::getValue(name, args);
}

void
ParticleListObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

  if      (name == "size") {
    auto n = Util::stringToInt(value);

    setNumPoints(n);
  }
  else if (name == "position") {
    // get index from args
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(points_.size()))
        return;

      points_[i] = stringToPoint3D(tcl, value);
    }
    else
      app->errorMsg("Missing index for position");
  }
  else if (name == "color") {
    // get index from args
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(colors_.size()))
        return;

      colors_[i] = stringToColor(tcl, value);
    }
    else
      app->errorMsg("Missing index for color");
  }
  else if (name == "generator") {
    int n = 10000;

    if (args.size() > 0)
      n = Util::stringToInt(args[0]);

    if (value == "lorenz") {
      CRMinMax xrange, yrange, zrange;

      setNumPoints(n);

      CLorenzCalc calc(0, n);

      int j = 0;

      for (int i = 0; i < calc.getIterationEnd(); i++) {
        double x, y, z;

        calc.nextValue(x, y, z);

        if (i >= calc.getIterationStart()) {
          points_[j] = CGLVector3D(x, y, z);

#if 0
          int i1 = int(j  - 1000*(j /1000));
          int i2 =     i1 -  100*(i1/ 100);
          int i3 =     i2 -   10*(i2/  10);

          i1 -= i2 + i3;
          i2 -= i3;

          float b = i1/1000.0;
          float g = i2/100.0;
          float r = i3/10.0;

          colors_[j] = Color(r, g, b);
#endif

          xrange.add(x);
          yrange.add(y);
          zrange.add(z);

          ++j;
        }
      }

      //std::cerr << "X: " << xrange.min() << " " << xrange.max() << "\n";
      //std::cerr << "Y: " << yrange.min() << " " << yrange.max() << "\n";
      //std::cerr << "Z: " << zrange.min() << " " << zrange.max() << "\n";

      for (int i = 0; i < j; ++i) {
        auto x1 = CMathUtil::map(points_[i].x(), xrange.min(), xrange.max(), -1, 1);
        auto y1 = CMathUtil::map(points_[i].y(), yrange.min(), yrange.max(), -1, 1);
        auto z1 = CMathUtil::map(points_[i].z(), zrange.min(), zrange.max(), -1, -2);

        auto r = CMathUtil::map(points_[i].x(), xrange.min(), xrange.max(), 0, 1);
        auto g = CMathUtil::map(points_[i].y(), yrange.min(), yrange.max(), 0, 1);
        auto b = CMathUtil::map(points_[i].z(), zrange.min(), zrange.max(), 0, 1);

        colors_[i] = Color(r, g, b);
        points_[i] = CGLVector3D(x1, y1, z1);
      }
    }
  }
  else if (name == "flocking") {
    delete flocking_;
    delete fireworks_;

    flocking_ = new CFlocking;

    auto n = flocking_->numBoids();

    setNumPoints(n);

    updateFlocking();
  }
  else if (name == "fireworks") {
    delete flocking_;
    delete fireworks_;

    fireworks_ = new CFireworks;

    auto n = 100;

    setNumPoints(n);

    updateFireworks();
  }
  else
    Object3D::setValue(name, value, args);
}

void
ParticleListObj::
setNumPoints(int n)
{
  auto n1 = int(points_.size());

  if      (n > n1) {
    for (int i = 0; i < n - n1; ++i) {
      points_.emplace_back();
      colors_.emplace_back(1.0, 1.0, 1.0);
    }
  }
  else if (n < n1) {
    for (int i = 0; i < n1 - n; ++i) {
      points_.pop_back();
      colors_.pop_back();
    }
  }
}

void
ParticleListObj::
init()
{
  if (! s_program) {
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "attribute highp vec4 position;\n"
      "attribute highp vec4 center;\n"
      "attribute lowp  vec4 color;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "varying lowp vec4 col;\n"
      "void main() {\n"
      "  col = color;\n"
      "  gl_Position = projection * view * model * (center + 0.005*position);\n"
      "}\n";

    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "varying lowp vec4 col;\n"
      "void main() {\n"
      "  gl_FragColor = col;\n"
      "}\n";

    s_program = new ParticleListProgramData;

    s_program->program = new QOpenGLShaderProgram(this);

    s_program->program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShaderSource);
    s_program->program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    s_program->program->link();

    // get program variables
    s_program->positionAttr = s_program->program->attributeLocation("position");
    Q_ASSERT(s_program->positionAttr != -1);

    s_program->centerAttr = s_program->program->attributeLocation("center");
    Q_ASSERT(s_program->centerAttr != -1);

    s_program->colorAttr = s_program->program->attributeLocation("color");
    Q_ASSERT(s_program->colorAttr != -1);

    s_program->projectionUniform = s_program->program->uniformLocation("projection");
    Q_ASSERT(s_program->projectionUniform != -1);

    s_program->viewUniform = s_program->program->uniformLocation("view");
    Q_ASSERT(s_program->viewUniform != -1);
  }

  // The VBO containing the 4 vertices of the particles.
  // Thanks to instancing, they will be shared by all particles.
  static const GLfloat g_vertex_buffer_data[] = {
   -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
   -0.5f,  0.5f, 0.0f,
    0.5f,  0.5f, 0.0f,
  };

  canvas_->glGenBuffers(1, &billboard_vertex_buffer_);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
                        g_vertex_buffer_data, GL_STATIC_DRAW);

  // The VBO containing the positions and sizes of the particles
  canvas_->glGenBuffers(1, &particles_position_buffer_);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer_);
  // Initialize with empty (null) buffer : it will be updated later, each frame.
  canvas_->glBufferData(GL_ARRAY_BUFFER, s_maxPoints*sizeof(CGLVector3D),
                        nullptr, GL_STREAM_DRAW);

  // The VBO containing the colors of the particles
  canvas_->glGenBuffers(1, &particles_color_buffer_);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer_);
  // Initialize with empty (null) buffer : it will be updated later, each frame.
  canvas_->glBufferData(GL_ARRAY_BUFFER, s_maxPoints*sizeof(Color),
                        nullptr, GL_STREAM_DRAW);
}

void
ParticleListObj::
tick()
{
  if      (flocking_) {
    flocking_->update(0.1);

    updateFlocking();

    needsUpdate_ = true;
  }
  else if (fireworks_) {
    fireworks_->step();

    fireworks_->updateParticles();

    fireworks_->updateCurrentParticles();

    updateFireworks();

    needsUpdate_ = true;
  }
}

void
ParticleListObj::
updateFlocking()
{
  double w = CFlock::getWorld().getXSize()/2;
  double h = CFlock::getWorld().getYSize()/2;
  double l = CFlock::getWorld().getZSize()/2;

  uint i = 0;

  for (auto *boid : flocking_->getBoids()) {
    auto p = boid->getPos();

    auto x = CMathUtil::map(p.getX(), -w, w, -0.9, 0.9);
    auto y = CMathUtil::map(p.getY(), -h, h, -0.9, 0.9);
    auto z = CMathUtil::map(p.getZ(), -l, l, -0.9, 0.9);

    points_[i] = CGLVector3D(x, y, z);

    auto c = boid->getFlock()->getColor();

    colors_[i] = Color(c.getRed(), c.getGreen(), c.getBlue());

    ++i;
  }
}

void
ParticleListObj::
updateFireworks()
{
  const auto &particles = fireworks_->currentParticles();

  auto n = std::min(particles.size(), points_.size());

  for (uint i = 0; i < n; ++i) {
    auto *particle = particles[i];
    if (! particle) continue;

    auto p = particle->getPosition();

    auto x = CMathUtil::map(p.getX(), -100, 100, -0.9, 0.9);
    auto y = CMathUtil::map(p.getY(), 0, 200, -0.9, 0.9);
    auto z = 0.0;

    points_[i] = CGLVector3D(x, y, z);

    auto c = particle->getColor();

    colors_[i] = Color(c.getRed(), c.getGreen(), c.getBlue());
  }
}

void
ParticleListObj::
render()
{
  s_program->program->bind();

  s_program->program->setUniformValue("projection",
    CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program->program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  modelMatrix_ = CGLMatrix3D::identity();
  modelMatrix_.translated(float(position().getX()),
                          float(position().getY()),
                          float(position().getZ()));
  modelMatrix_.rotated(xAngle(), CGLVector3D(1.0, 0.0, 0.0));
  modelMatrix_.rotated(yAngle(), CGLVector3D(0.0, 1.0, 0.0));
  modelMatrix_.rotated(zAngle(), CGLVector3D(0.0, 0.0, 1.0));

  s_program->program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

  auto n = points_.size();

  // Update the buffers that OpenGL uses for rendering.
  // There are much more sophisticated means to stream data from the CPU to the GPU,
  // but this is outside the scope of this tutorial.
  // http://www.opengl.org/wiki/Buffer_Object_Streaming
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer_);
  // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  canvas_->glBufferData(GL_ARRAY_BUFFER, s_maxPoints*sizeof(CGLVector3D),
                        nullptr, GL_STREAM_DRAW);
  canvas_->glBufferSubData(GL_ARRAY_BUFFER, 0, n*sizeof(CGLVector3D), &points_[0]);

  canvas_->glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer_);
  // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  canvas_->glBufferData(GL_ARRAY_BUFFER, s_maxPoints*sizeof(Color),
                        nullptr, GL_STREAM_DRAW);
  canvas_->glBufferSubData(GL_ARRAY_BUFFER, 0, n*sizeof(Color), &colors_[0]);

  // 1rst attribute buffer : vertices
  canvas_->glEnableVertexAttribArray(s_program->positionAttr);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer_);
  canvas_->glVertexAttribPointer(
   s_program->positionAttr,
   3,
   GL_FLOAT, // type
   GL_FALSE, // normalized?
   0,
   nullptr
  );

  // 2nd attribute buffer : positions of particles' centers
  canvas_->glEnableVertexAttribArray(s_program->centerAttr);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer_);
  canvas_->glVertexAttribPointer(
   s_program->centerAttr,
   3,
   GL_FLOAT, // type
   GL_FALSE, // normalized?
   0,
   nullptr
  );

  // 3rd attribute buffer : particles' colors
  canvas_->glEnableVertexAttribArray(s_program->colorAttr);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer_);
  canvas_->glVertexAttribPointer(
   s_program->colorAttr,
   3,
   GL_FLOAT, // type
   GL_FALSE, // normalized?
   0,
   nullptr
  );

  canvas_->glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, n);

  // These functions are specific to glDrawArrays*Instanced*.
  // The first parameter is the attribute buffer we're talking about.
  // The second parameter is the "rate at which generic vertex attributes advance when
  // rendering multiple instances"
  // http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml

  // particles vertices : always reuse the same 4 vertices -> 0
  canvas_->glVertexAttribDivisor(s_program->positionAttr, 0);
  // center per quad -> 1
  canvas_->glVertexAttribDivisor(s_program->centerAttr, 1);
  // color per quad -> 1
  canvas_->glVertexAttribDivisor(s_program->colorAttr, 1);

  // Draw the particules !
  // This draws many times a small triangle_strip (which looks like a quad).
  // This is equivalent to :
  // for(i in n) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4),
  // but faster.
  canvas_->glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, n);

  s_program->program->release();
}

//---

ProgramData *Graph3DObj::s_program1 = nullptr;
ProgramData *Graph3DObj::s_program2 = nullptr;

bool
Graph3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Graph3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Graph3DObj::
Graph3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

void
Graph3DObj::
addDemoNodes()
{
  delete forceDirected_;

  forceDirected_ = new CForceDirected3D;

  //---

  auto node1 = forceDirected_->newNode();
  auto node2 = forceDirected_->newNode();
  auto node3 = forceDirected_->newNode();
  auto node4 = forceDirected_->newNode();

  auto node5 = forceDirected_->newNode();
  auto node6 = forceDirected_->newNode();
  auto node7 = forceDirected_->newNode();
  auto node8 = forceDirected_->newNode();

  node1->setLabel("Node 1");
  node2->setLabel("Node 2");
  node3->setLabel("Node 3");
  node4->setLabel("Node 4");
  node5->setLabel("Node 5");
  node6->setLabel("Node 6");
  node7->setLabel("Node 7");
  node8->setLabel("Node 8");

  auto edge1 = forceDirected_->newEdge(node1, node2);
  auto edge2 = forceDirected_->newEdge(node2, node3);
  auto edge3 = forceDirected_->newEdge(node3, node1);
  auto edge4 = forceDirected_->newEdge(node1, node4);
  auto edge5 = forceDirected_->newEdge(node2, node4);
  auto edge6 = forceDirected_->newEdge(node3, node4);

  auto edge7  = forceDirected_->newEdge(node1, node5);
  auto edge8  = forceDirected_->newEdge(node2, node6);
  auto edge9  = forceDirected_->newEdge(node3, node7);
  auto edge10 = forceDirected_->newEdge(node4, node8);
}

void
Graph3DObj::
initSteps()
{
#if 0
  int initSteps { 1000 };

  for (int i = 0; i < initSteps; ++i)
    forceDirected_->step(stepSize_);
#endif
}

void
Graph3DObj::
init()
{
  if (! s_program1) {
    static const char *vertexShader1 =
      "#version 330 core\n"
      "layout (location = 0) in vec3 point;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "void main()\n"
      "{\n"
      "  gl_Position = projection * view * model * vec4(point.x, point.y, point.z, 1.0);\n"
      "}";
    static const char *geometryShader1 =
      "#version 330 core\n"
      "layout (points) in;\n"
      "layout (line_strip, max_vertices = 5) out;\n"
      "\n"
      "void main() {\n"
      "  gl_Position = gl_in[0].gl_Position + vec4(-0.01, -0.01, 0.0, 0.0);\n"
      "  EmitVertex();\n"
      "  gl_Position = gl_in[0].gl_Position + vec4( 0.01, -0.01, 0.0, 0.0);\n"
      "  EmitVertex();\n"
      "  gl_Position = gl_in[0].gl_Position + vec4( 0.01,  0.01, 0.0, 0.0);\n"
      "  EmitVertex();\n"
      "  gl_Position = gl_in[0].gl_Position + vec4(-0.01,  0.01, 0.0, 0.0);\n"
      "  EmitVertex();\n"
      "  gl_Position = gl_in[0].gl_Position + vec4(-0.01, -0.01, 0.0, 0.0);\n"
      "  EmitVertex();\n"
      "  EndPrimitive();\n"
      "}\n";
    static const char *fragmentShader1 =
      "#version 330 core\n"
      "uniform vec3 lineColor;\n"
      "void main()\n"
      "{\n"
      "  gl_FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
      "}\n";

    s_program1 = new ProgramData;

    s_program1->program = new QOpenGLShaderProgram(this);

    s_program1->program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShader1);
    s_program1->program->addShaderFromSourceCode(QOpenGLShader::Geometry, geometryShader1);
    s_program1->program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader1);

    s_program1->program->link();

    //---

    static const char *vertexShader2 =
      "#version 330 core\n"
      "layout (location = 1) in vec3 line;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "void main()\n"
      "{\n"
      "  gl_Position = projection * view * model * vec4(line.x, line.y, line.z, 1.0);\n"
      "}";
    static const char *fragmentShader2 =
      "#version 330 core\n"
      "uniform vec3 lineColor;\n"
      "void main()\n"
      "{\n"
      "  gl_FragColor = vec4(lineColor.x, lineColor.y, lineColor.z, 1.0f);\n"
      "}\n";

    s_program2 = new ProgramData;

    s_program2->program = new QOpenGLShaderProgram(this);

    s_program2->program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShader2);
    s_program2->program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader2);

    s_program2->program->link();
  }

  //---

  canvas_->glGenVertexArrays(1, &pointsArrayId_);

  canvas_->glGenBuffers(1, &pointsBufferId_);
  canvas_->glGenBuffers(1, &linesBufferId_);
}

QVariant
Graph3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

void
Graph3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if (name == "dot_file") {
    (void) loadDotFile(value);
  }
  else
    Object3D::setValue(name, value, args);
}

bool
Graph3DObj::
loadDotFile(const QString &filename)
{
  delete parse_;

  parse_ = new CDotParse::Parse(filename.toStdString());

  if (! parse_->parse())
    return false;

  //---

  delete forceDirected_;

  forceDirected_ = new CForceDirected3D;

  //---

  using NodeMap = std::map<uint, CForceDirected3D::NodeP>;

  NodeMap nodeMap;

  for (const auto &pg : parse_->graphs()) {
    const auto &graph = pg.second;

    for (const auto &pn : graph->nodes()) {
      const auto &node = pn.second;

      auto node1 = forceDirected_->newNode();

      node1->setLabel(node->name());

      nodeMap[node->id()] = node1;
    }

    for (const auto &edge : graph->edges()) {
      auto *fromNode = edge->fromNode();
      auto *toNode   = edge->toNode  ();

      const auto &node1 = nodeMap[fromNode->id()];
      const auto &node2 = nodeMap[toNode  ->id()];

      (void) forceDirected_->newEdge(node1, node2);
    }
  }

  return true;
}

void
Graph3DObj::
tick()
{
  forceDirected_->step(stepSize_);

  updatePoints();

  updateTextObjs();
}

void
Graph3DObj::
updatePoints()
{
  double xmin, ymin, zmin, xmax, ymax, zmax;
  forceDirected_->calcRange(xmin, ymin, zmin, xmax, ymax, zmax);

  auto mapPoint = [&](const Springy3D::Point &p) {
    auto x = CMathUtil::map(p.p().x(), xmin, xmax, -0.45, 0.45);
    auto y = CMathUtil::map(p.p().y(), ymin, ymax, -0.45, 0.45);
    auto z = CMathUtil::map(p.p().z(), zmin, zmax, -0.45, 0.45);

    return CGLVector3D(x, y, z);
  };

  if (xmax == xmin) xmax = xmin + 0.01;
  if (ymax == ymin) ymax = ymin + 0.01;
  if (zmax == zmin) zmax = zmin + 0.01;

  auto nodes = forceDirected_->nodes();

  auto nn = nodes.size();

  if (nn != points_.size())
    points_.resize(nn);

  for (size_t i = 0; i < nn; ++i) {
    auto point = forceDirected_->point(nodes[i]);

    points_[i] = mapPoint(point->p());
  }

  auto edges = forceDirected_->edges();

  auto ne = edges.size();

  if (2*ne != linePoints_.size())
    linePoints_.resize(2*ne);

  for (size_t i = 0; i < ne; ++i) {
    auto point1 = forceDirected_->point(edges[i]->source());
    auto point2 = forceDirected_->point(edges[i]->target());

    linePoints_[2*i    ] = mapPoint(point1->p());
    linePoints_[2*i + 1] = mapPoint(point2->p());
  }

  //---

  // bind the Vertex Array Object
  canvas_->glBindVertexArray(pointsArrayId_);

  //---

  // store point data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, pointsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, nn*sizeof(CGLVector3D), &points_[0], GL_STATIC_DRAW);

  // set points attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(0);

  //---

  // store point data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, linesBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, 2*ne*sizeof(CGLVector3D), &linePoints_[0], GL_STATIC_DRAW);

  // set points attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(1);

  //---

  canvas_->glBindBuffer(GL_ARRAY_BUFFER, 0);
  canvas_->glBindVertexArray(0);

  //---

  updateModelMatrix();
}

void
Graph3DObj::
updateModelMatrix()
{
  modelMatrix_ = CGLMatrix3D::identity();

  modelMatrix_.translated(float(position().getX()),
                          float(position().getY()),
                          float(position().getZ()));

  modelMatrix_.rotated(xAngle(), CGLVector3D(1.0, 0.0, 0.0));
  modelMatrix_.rotated(yAngle(), CGLVector3D(0.0, 1.0, 0.0));
  modelMatrix_.rotated(zAngle(), CGLVector3D(0.0, 0.0, 1.0));
}

void
Graph3DObj::
updateTextObjs()
{
  auto nodes = forceDirected_->nodes();

  auto nn = nodes.size();

  while (textObjs_.size() > nn) {
    auto *obj = textObjs_.back();

    textObjs_.pop_back();

    canvas_->removeObject(obj);

    delete obj;
  }

  while (textObjs_.size() < nn) {
    auto *obj = new Text3DObj(canvas_);

    (void) canvas_->addNewObject(obj);

    obj->init();

    textObjs_.push_back(obj);
  }

  int i = 0;

  for (auto &node : nodes) {
    auto *textObj = textObjs_[i];

    textObj->setText(QString::fromStdString(node->label()));

    const auto &p = points_[i];

    float x, y, z;
    modelMatrix_.multiplyPoint(p.x(), p.y(), p.z(), &x, &y, &z);

    auto p1 = CPoint3D(x, y, z);

    textObj->setPosition(p1);

    ++i;
  }
}

void
Graph3DObj::
render()
{
  s_program1->program->bind();

  s_program1->program->setUniformValue("projection",
    CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program1->program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  s_program1->program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

  canvas_->glBindVertexArray(pointsArrayId_);

  int np = points_.size();

  glDrawArrays(GL_POINTS, 0, np);
//glDrawArrays(GL_TRIANGLES, 0, np);

  s_program1->program->release();

  //------

  s_program2->program->bind();

  s_program2->program->setUniformValue("lineColor", toVector(lineColor_));

  s_program2->program->setUniformValue("projection",
    CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program2->program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  s_program2->program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

  canvas_->glBindVertexArray(pointsArrayId_);

  int nl = linePoints_.size();

  glDrawArrays(GL_LINES, 0, nl);

  s_program2->program->release();
}

//---

ProgramData *SurfaceObj::s_program   = nullptr;

bool
SurfaceObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new SurfaceObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

SurfaceObj::
SurfaceObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

QVariant
SurfaceObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

void
SurfaceObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

  if      (name == "size") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() >= 2) {
      bool ok;
      nx_ = std::max(strs[0].toInt(&ok), 0);
      ny_ = std::max(strs[1].toInt(&ok), 0);
    }

    resizePoints();

    needsUpdate_ = true;
  }
  else if (name == "water_surface") {
    bool ok;
    nx_ = std::max(value.toInt(&ok), 1);
    ny_ = nx_;

    resizePoints();

    delete waterSurface_;
    delete flag_;

    waterSurface_ = new CWaterSurface(nx_);
    flag_         = nullptr;

    //---

    for (int iy = 1; iy < nx_ - 1; ++iy) {
      for (int ix = 1; ix < nx_ - 1; ++ix) {
        waterSurface_->setDampening(ix, iy, 1.0);

        waterSurface_->setZ(ix, iy, 0.0);
      }
    }

    waterSurface_->setZ(int(    nx_/4.0), int(    nx_/4.0), 1.0);
    waterSurface_->setZ(int(3.0*nx_/4.0), int(3.0*nx_/4.0), 1.0);

    //---

    updateWaterSurface();
  }
  else if (name == "flag") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() >= 2) {
      bool ok;
      nx_ = std::max(strs[0].toInt(&ok), 0);
      ny_ = std::max(strs[1].toInt(&ok), 0);
    }

    resizePoints();

    delete waterSurface_;
    delete flag_;

    waterSurface_ = nullptr;
    flag_         = new CFlag(-0.5, -0.5, 1, 1, nx_, ny_);

    flag_->setWind(true);
    flag_->setWindForce(3.0);

    //---

    updateFlag();
  }
  else if (name == "point") {
    int ix = -1, iy = -1;

    // get index from args
    if (args.size() > 0) {
      QStringList strs;
      (void) tcl->splitList(args[0], strs);

      if (strs.size() >= 2) {
        ix = Util::stringToInt(strs[0]);
        iy = Util::stringToInt(strs[1]);
      }

      if (ix < 0 || ix >= nx_ || iy < 0 || iy >= ny_)
        return;

      int ixy = iy*nx_ + ix;

      auto z = Util::stringToReal(value);

      points_[ixy].setZ(z);
    }
    else
      app->errorMsg("Missing index for point");

    needsUpdate_ = true;
  }
  else if (name == "color") {
    int ix = -1, iy = -1;

    // get index from args
    if (args.size() > 0) {
      QStringList strs;
      (void) tcl->splitList(args[0], strs);

      if (strs.size() >= 2) {
        ix = Util::stringToInt(strs[0]);
        iy = Util::stringToInt(strs[1]);
      }

      if (ix < 0 || ix >= nx_ || iy < 0 || iy >= ny_)
        return;

      int ixy = iy*nx_ + ix;

      QStringList cstrs;
      (void) tcl->splitList(value, cstrs);

      auto r = Util::stringToReal(cstrs[0]);
      auto g = Util::stringToReal(cstrs[1]);
      auto b = Util::stringToReal(cstrs[2]);

      colors_[ixy] = CGLVector3D(r, g, b);
    }
    else
      app->errorMsg("Missing index for color");

    needsUpdate_ = true;
  }
  else if (name == "wireframe") {
    wireframe_ = Util::stringToBool(value);

    needsUpdate_ = true;
  }
  else
    Object3D::setValue(name, value, args);
}

void
SurfaceObj::
resizePoints()
{
  auto np = nx_*ny_;

  points_.resize(np);

  int ip = 0;

  for (int iy = 0; iy < ny_; ++iy) {
    auto y = CMathUtil::map(iy, 0, ny_ - 1, 0.0, 1.0);

    for (int ix = 0; ix < nx_; ++ix) {
      auto x = CMathUtil::map(ix, 0, nx_ - 1, 0.0, 1.0);

      points_[ip++] = CGLVector3D(x, y, 0.0);
    }
  }

  auto ni = std::max(6*(nx_ - 1)*(ny_ - 1), 0);

  indices_.resize(ni);

  int ii = 0;

  for (int iy = 0; iy < ny_ - 1; ++iy) {
    for (int ix = 0; ix < nx_ - 1; ++ix) {
      int ixy = iy*nx_ + ix;

      indices_[ii++] = ixy;
      indices_[ii++] = ixy + 1;
      indices_[ii++] = ixy + nx_;

      indices_[ii++] = ixy + 1;
      indices_[ii++] = ixy + nx_ + 1;
      indices_[ii++] = ixy + nx_;
    }
  }

  assert(ii == ni);

  colors_.resize(np);

  for (int i = 0; i < np; ++i)
    colors_[i] = CGLVector3D(0.5, 0.7, 0.4);
}

void
SurfaceObj::
init()
{
  modelMatrix_ = CGLMatrix3D::identity();

  //---

  if (! s_program) {
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "layout (location = 1) in vec3 aNormal;\n"
      "layout (location = 2) in vec3 aColor;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "out vec3 FragPos;\n"
      "out vec3 Normal;\n"
      "out vec3 Color;\n"
      "void main() {\n"
      "  FragPos = vec3(model * vec4(aPos, 1.0));\n"
      "  Normal  = mat3(transpose(inverse(model)))*aNormal;\n"
      "  Color   = aColor;\n"
      "  gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
      "}";
    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "in vec3 FragPos;\n"
      "in vec3 Normal;\n"
      "in vec3 Color;\n"
      "out vec4 FragColor;\n"
      "uniform vec3 lightPos;\n"
      "uniform vec3 viewPos;\n"
      "uniform vec3 lightColor;\n"
      "uniform float ambientStrength;\n"
      "uniform float diffuseStrength;\n"
      "uniform float specularStrength;\n"
      "uniform float shininess;\n"
      "void main() {\n"
      "  vec3 norm = normalize(Normal);\n"
      "  vec3 lightDir = normalize(lightPos - FragPos);\n"
      "  float diff = max(dot(norm, lightDir), 0.0);\n"
      "  vec3 diffuseColor = Color;\n"
      "  vec3 diffuse = diffuseStrength*diff*diffuseColor;\n"
      "  vec3 ambient = ambientStrength*diffuseColor;\n"
      "  vec3 viewDir = normalize(viewPos - FragPos);\n"
      "  vec3 reflectDir = reflect(-lightDir, norm);\n"
      "  float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);\n"
      "  vec3 specColor = lightColor;\n"
      "  vec3 specular = specularStrength*spec*specColor;\n"
      "  vec3 result = ambient + diffuse + specular;\n"
      "  FragColor = vec4(result, 1.0f);\n"
      "}\n";

    s_program = new ProgramData;

    s_program->program = new QOpenGLShaderProgram(this);

    s_program->program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShaderSource);
    s_program->program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    s_program->program->link();
  }

  //---

  canvas_->glGenVertexArrays(1, &vertexArrayId_);

  canvas_->glGenBuffers(1, &pointsBufferId_);
  canvas_->glGenBuffers(1, &normalsBufferId_);
  canvas_->glGenBuffers(1, &colorsBufferId_);
  canvas_->glGenBuffers(1, &indBufferId_);
}

void
SurfaceObj::
tick()
{
  if      (waterSurface_) {
    waterSurface_->step(0.1);

    updateWaterSurface();

    needsUpdate_ = true;
  }
  else if (flag_) {
    flag_->step(0.0005);

    updateFlag();

    needsUpdate_ = true;
  }
}

void
SurfaceObj::
updateWaterSurface()
{
  uint nxy = nx_*ny_;

  for (uint i = 0; i < nxy; ++i) {
    points_[i].setZ(waterSurface_->getZ(i));
  }
}

void
SurfaceObj::
updateFlag()
{
  int i = 0;

  for (int iy = 0; iy < ny_; ++iy) {
    for (int ix = 0; ix < nx_; ++ix) {
      auto *particle = flag_->getParticle(ix, iy);

      auto pos = particle->getPosition();

      points_[i].setX(pos.getX());
      points_[i].setY(pos.getY());
      points_[i].setZ(pos.getZ());

      ++i;
    }
  }
}

void
SurfaceObj::
updateGL()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  //---

  calcNormals();

  //---

  // bind the Vertex Array Object
  canvas_->glBindVertexArray(vertexArrayId_);

  //---

  int np = points_.size();

  // store point data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, pointsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D), &points_[0], GL_STATIC_DRAW);

  // set points attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(0);

  // store normal data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, normalsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D), &normals_[0], GL_STATIC_DRAW);

  // set normals attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(1);

  // store color data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, colorsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D), &colors_[0], GL_STATIC_DRAW);

  // set colors attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(2);

  //---

  // store index data in element buffer
  int ni = indices_.size();

  if (ni > 0) {
    canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufferId_);
    canvas_->glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni*sizeof(unsigned int),
                          &indices_[0], GL_STATIC_DRAW);
  }

  //---

  canvas_->glBindBuffer(GL_ARRAY_BUFFER, 0);
//canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,  0);

  canvas_->glBindVertexArray(0);
}

void
SurfaceObj::
calcNormals()
{
  auto np = points_.size();

  normals_.resize(np);

  for (int iy = 0; iy < ny_ - 1; ++iy) {
    for (int ix = 0; ix < nx_ - 1; ++ix) {
      auto ixy = iy*nx_ + ix;

      const auto &v1 = points_[ixy];
      const auto &v2 = points_[ixy + 1];
      const auto &v3 = points_[ixy + nx_];

      CGLVector3D diff1(v1, v2);
      CGLVector3D diff2(v2, v3);

      auto n = diff1.crossProduct(diff2).normalized();

      normals_[ixy          ] = n;
      normals_[ixy + 1      ] = n;
      normals_[ixy + nx_    ] = n;
      normals_[ixy + nx_ + 1] = n;
    }
  }
}

void
SurfaceObj::
render()
{
  updateGL();

  if (wireframe_)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  //---

  CGLVector3D lightColor(1.0f, 1.0f, 1.0f);

  auto *program = s_program->program;

  program->bind();

  program->setUniformValue("lightColor", CQGLUtil::toVector(lightColor));
  program->setUniformValue("lightPos"  , CQGLUtil::toVector(canvas_->lightPos()));
  program->setUniformValue("viewPos"   , CQGLUtil::toVector(canvas_->viewPos()));

  program->setUniformValue("ambientStrength" , float(canvas_->ambient()));
  program->setUniformValue("diffuseStrength" , float(canvas_->diffuse()));
  program->setUniformValue("specularStrength", float(canvas_->specular()));
  program->setUniformValue("shininess"       , float(canvas_->shininess()));

  program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  modelMatrix_ = CGLMatrix3D::identity();
  modelMatrix_.translated(float(position().getX()),
                          float(position().getY()),
                          float(position().getZ()));
  modelMatrix_.rotated(xAngle(), CGLVector3D(1.0, 0.0, 0.0));
  modelMatrix_.rotated(yAngle(), CGLVector3D(0.0, 1.0, 0.0));
  modelMatrix_.rotated(zAngle(), CGLVector3D(0.0, 0.0, 1.0));

  program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

  //---

  canvas_->glBindVertexArray(vertexArrayId_);

  //---

  int np = points_.size();
  int ni = indices_.size();

  if (ni > 0)
    glDrawElements(GL_TRIANGLES, ni, GL_UNSIGNED_INT, nullptr);
  else
    glDrawArrays(GL_TRIANGLES, 0, np);

  //canvas_->glBindVertexArray(0);
}

//---

struct FontData {
  uint32_t                            size = 40;
  uint32_t                            atlasWidth = 1024;
  uint32_t                            atlasHeight = 1024;
  uint32_t                            oversampleX = 2;
  uint32_t                            oversampleY = 2;
  uint32_t                            firstChar = ' ';
  uint32_t                            charCount = '~' - ' ';
  std::unique_ptr<stbtt_packedchar[]> charInfo;
  GLuint                              texture = 0;
};

Text3DObj::TextProgramData* Text3DObj::s_program  = nullptr;
FontData*                   Text3DObj::s_fontData = nullptr;

bool
Text3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Text3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Text3DObj::
Text3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

void
Text3DObj::
init()
{
  if (! s_program) {
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "attribute highp vec4 position;\n"
      "attribute highp vec2 texCoord0;\n"
      "attribute lowp  vec3 color;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "varying highp vec2 uv0;\n"
      "varying lowp vec3 col;\n"
      "void main() {\n"
      "  gl_Position = projection * view * model * position;\n"
      "  uv0 = texCoord0;\n"
      "  col = color;\n"
      "}\n";

    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "varying lowp vec3 col;\n"
      "varying highp vec2 uv0;\n"
      "uniform sampler2D mainTex;\n"
      "void main() {\n"
       " vec4 tc = texture(mainTex, uv0);\n"
      "  gl_FragColor = vec4(col.r, col.g, col.b, tc.r);\n"
      "}\n";

    s_program = new TextProgramData;

    s_program->program = new QOpenGLShaderProgram(this);

    s_program->program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShaderSource);
    s_program->program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    s_program->program->link();

    //---

    s_program->posAttr = s_program->program->attributeLocation("position");
    Q_ASSERT(s_program->posAttr != -1);

    s_program->colAttr = s_program->program->attributeLocation("color");
  //Q_ASSERT(s_program->colAttr != -1);

    s_program->texPosAttr = s_program->program->attributeLocation("texCoord0");
    Q_ASSERT(s_program->texPosAttr != -1);

    s_program->projectionUniform = s_program->program->uniformLocation("projection");
    Q_ASSERT(s_program->projectionUniform != -1);

    s_program->viewUniform = s_program->program->uniformLocation("view");
    Q_ASSERT(s_program->viewUniform != -1);

    s_program->textureUniform = s_program->program->uniformLocation("mainTex");
    Q_ASSERT(s_program->textureUniform != -1);
  }

  initFont();
}

void
Text3DObj::
initFont()
{
  if (s_fontData)
    return;

  s_fontData = new FontData;

  //---

  auto fontData = readFile("fonts/OpenSans-Regular.ttf");
  auto atlasData = std::make_unique<uint8_t[]>(s_fontData->atlasWidth*s_fontData->atlasHeight);

  s_fontData->charInfo = std::make_unique<stbtt_packedchar[]>(s_fontData->charCount);

  stbtt_pack_context context;
  if (! stbtt_PackBegin(&context, atlasData.get(), s_fontData->atlasWidth,
                        s_fontData->atlasHeight, 0, 1, nullptr))
    assert(false);

  stbtt_PackSetOversampling(&context, s_fontData->oversampleX, s_fontData->oversampleY);

  if (! stbtt_PackFontRange(&context, fontData.data(), 0, s_fontData->size, s_fontData->firstChar,
                            s_fontData->charCount, s_fontData->charInfo.get()))
    assert(false);

  stbtt_PackEnd(&context);

  glGenTextures(1, &s_fontData->texture);
  glBindTexture(GL_TEXTURE_2D, s_fontData->texture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s_fontData->atlasWidth, s_fontData->atlasHeight,
               0, GL_RED, GL_UNSIGNED_BYTE, atlasData.get());
  glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

  canvas_->glGenerateMipmap(GL_TEXTURE_2D);
}

std::vector<uint8_t>
Text3DObj::
readFile(const char *path) const
{
  std::ifstream file(path, std::ios::binary | std::ios::ate);

  if (! file.is_open())
    assert(false);

  const auto size = file.tellg();

  file.seekg(0, std::ios::beg);
  auto bytes = std::vector<uint8_t>(size);
  file.read(reinterpret_cast<char *>(&bytes[0]), size);
  file.close();

  return bytes;
}

void
Text3DObj::
setText(const QString &text)
{
  text_ = text;

  textDataValid_  = false;
}

QVariant
Text3DObj::
getValue(const QString &name, const QStringList &args)
{
  if (name == "text")
    return text();
  else
    return Object3D::getValue(name, args);
}

void
Text3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if (name == "text")
    setText(value);
  else
    Object3D::setValue(name, value, args);
}

void
Text3DObj::
updateTextData()
{
  vertices_.clear();
  uvs_     .clear();
  indexes_ .clear();
  colors_  .clear();

  const auto &color = color_;

  uint16_t lastIndex = 0;

  float offsetX = 0, offsetY = 0;

  double f = 1.0/s_fontData->size;

  for (const auto &c : text_) {
    const auto glyphInfo = makeGlyphInfo(c.toLatin1(), offsetX, offsetY);

    offsetX = glyphInfo.offsetX;
    offsetY = glyphInfo.offsetY;

    auto addPos = [&](int i) {
      // x, y, z
      auto pos = glyphInfo.positions[i];

      pos.scaleX(f);
      pos.scaleY(f);

      vertices_.emplace_back(pos);

      // u, v
      uvs_.emplace_back(glyphInfo.uvs[i]);

      // color
      colors_.push_back(color);
    };

    addPos(0);
    addPos(1);
    addPos(2);
    addPos(3);

    //---

    indexes_.push_back(lastIndex);
    indexes_.push_back(lastIndex + 1);
    indexes_.push_back(lastIndex + 2);
    indexes_.push_back(lastIndex);
    indexes_.push_back(lastIndex + 2);
    indexes_.push_back(lastIndex + 3);

    lastIndex += 4;
  }

  glData_.indexElementCount = indexes_.size();
}

void
Text3DObj::
initGLData()
{
  if (! glData_.vao)
    canvas_->glGenVertexArrays(1, &glData_.vao);

  if (! glData_.vertexBuffer)
    canvas_->glGenBuffers(1, &glData_.vertexBuffer);

  if (! glData_.uvBuffer)
    canvas_->glGenBuffers(1, &glData_.uvBuffer);

  if (! glData_.colBuffer)
    canvas_->glGenBuffers(1, &glData_.colBuffer);

  if (! glData_.indexBuffer)
    canvas_->glGenBuffers(1, &glData_.indexBuffer);
}

Text3DObj::GlyphInfo
Text3DObj::
makeGlyphInfo(uint32_t character, float offsetX, float offsetY) const
{
  stbtt_aligned_quad quad;

  stbtt_GetPackedQuad(s_fontData->charInfo.get(), s_fontData->atlasWidth, s_fontData->atlasHeight,
                      character - s_fontData->firstChar, &offsetX, &offsetY, &quad, 1);

  const auto xmin =  quad.x0;
  const auto ymin = -quad.y1;
  const auto xmax =  quad.x1;
  const auto ymax = -quad.y0;

  GlyphInfo info{};

  info.offsetX = offsetX;
  info.offsetY = offsetY;

  info.positions[0] = CGLVector3D(xmin, ymin, 0.0f);
  info.positions[1] = CGLVector3D(xmin, ymax, 0.0f);
  info.positions[2] = CGLVector3D(xmax, ymax, 0.0f);
  info.positions[3] = CGLVector3D(xmax, ymin, 0.0f);

  info.uvs[0] = CGLVector2D(quad.s0, quad.t1);
  info.uvs[1] = CGLVector2D(quad.s0, quad.t0);
  info.uvs[2] = CGLVector2D(quad.s1, quad.t0);
  info.uvs[3] = CGLVector2D(quad.s1, quad.t1);

  return info;
}

void
Text3DObj::
render()
{
  if (! textDataValid_) {
    updateTextData();

    initGLData();

    textDataValid_ = true;
  }

  //---

  modelMatrix_ = CGLMatrix3D::identity();

  modelMatrix_.translated(float(position().getX()),
                          float(position().getY()),
                          float(position().getZ()));
  if (isRotated()) {
    modelMatrix_.rotated(xAngle(), CGLVector3D(1.0, 0.0, 0.0));
    modelMatrix_.rotated(yAngle(), CGLVector3D(0.0, 1.0, 0.0));
    modelMatrix_.rotated(zAngle(), CGLVector3D(0.0, 0.0, 1.0));
  }
  modelMatrix_.scaled(size_, size_, 1.0);

  //------

  s_program->program->bind();

  s_program->program->setUniformValue("projection",
    CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program->program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  s_program->program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

  //---

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glEnable(GL_TEXTURE_2D);

  //---

  canvas_->glBindVertexArray(glData_.vao);

  // get GL_ARRAY_BUFFER (vertices) data from vertexBuffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, glData_.vertexBuffer);
  canvas_->glBufferData(GL_ARRAY_BUFFER, sizeof(CGLVector3D)*vertices_.size(),
                        vertices_.data(), GL_STATIC_DRAW);

  // position attribute populated from 3 floats (x, y, z) (data from GL_ARRAY_BUFFER)
  canvas_->glVertexAttribPointer(s_program->posAttr, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  canvas_->glEnableVertexAttribArray(s_program->posAttr);

  //--

  // get GL_ARRAY_BUFFER data from uvBuffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, glData_.uvBuffer);
  canvas_->glBufferData(GL_ARRAY_BUFFER, sizeof(CGLVector2D)*uvs_.size(),
                        uvs_.data(), GL_STATIC_DRAW);

  // texture pos attribute populated from 2 floats (u, v) (data from GL_ARRAY_BUFFER)
  canvas_->glVertexAttribPointer(s_program->texPosAttr, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  canvas_->glEnableVertexAttribArray(s_program->texPosAttr);

  //--

  // get GL_ARRAY_BUFFER data from colBuffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, glData_.colBuffer);
  canvas_->glBufferData(GL_ARRAY_BUFFER, sizeof(Color)*colors_.size(),
                        colors_.data(), GL_STATIC_DRAW);

  // color attribute populated from 3 floats (r, g, b)
  canvas_->glVertexAttribPointer(s_program->colAttr, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  canvas_->glEnableVertexAttribArray(s_program->colAttr);

  //--

  // get GL_ELEMENT_ARRAY_BUFFER from indexBuffer
  canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glData_.indexBuffer);
  canvas_->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t)*glData_.indexElementCount,
                        indexes_.data(), GL_STATIC_DRAW);

  //matrix1.translate(pos_.x, pos_.y, pos_.z - 2);
//matrix1.rotate(rotation_, rotation_, rotation_, 0);
//matrix1.scale(size_, size_, 1.0f);

  glBindTexture(GL_TEXTURE_2D, s_fontData->texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
  glActiveTexture(GL_TEXTURE0);

  s_program->program->setUniformValue(s_program->textureUniform, GL_TEXTURE0);

  canvas_->glBindVertexArray(glData_.vao);
  canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glData_.indexBuffer);
  glDrawElements(GL_TRIANGLES, glData_.indexElementCount, GL_UNSIGNED_SHORT, nullptr);

  //---

  canvas_->glBindBuffer(GL_ARRAY_BUFFER, 0);
  canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  canvas_->glBindVertexArray(0);

  canvas_->glDisableVertexAttribArray(s_program->posAttr);
  canvas_->glDisableVertexAttribArray(s_program->texPosAttr);
  canvas_->glDisableVertexAttribArray(s_program->colAttr);

  //---

  s_program->program->release();

  glPopAttrib();
}

//---

ProgramData *Path3DObj::s_program = nullptr;

bool
Path3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Path3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Path3DObj::
Path3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

void
Path3DObj::
init()
{
  modelMatrix_ = CGLMatrix3D::identity();

  //---

  if (! s_program) {
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "void main() {\n"
      "  gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
      "}";
    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "void main() {\n"
      " gl_FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
      "}\n";

    s_program = new ProgramData;

    s_program->program = new QOpenGLShaderProgram(this);

    s_program->program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShaderSource);
    s_program->program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    s_program->program->link();
  }

  //---

  canvas_->glGenVertexArrays(1, &vertexArrayId_);

  canvas_->glGenBuffers(1, &pointsBufferId_);
}

void
Path3DObj::
setLine(const CGLVector3D &p1, const CGLVector3D &p2)
{
  path_.clear();

  path_.moveTo(p1);
  path_.lineTo(p2);

  updatePoints();

  needsUpdate_ = true;
}

QVariant
Path3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

void
Path3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

  if (name == "path") {
    path_.clear();

    QStringList strs;
    (void) tcl->splitList(value, strs);

    for (const auto &str : strs) {
      QStringList strs1;
      (void) tcl->splitList(str, strs1);

      if (strs1.size() < 2) continue;

      if      (strs1[0] == "M") {
        auto p = stringToVector3D(tcl, strs1[1]);

        path_.moveTo(p);
      }
      else if (strs1[0] == "L") {
        auto p = stringToVector3D(tcl, strs1[1]);

        path_.lineTo(p);
      }
    }

    updatePoints();

    needsUpdate_ = true;
  }
  else
    Object3D::setValue(name, value, args);
}

void
Path3DObj::
updatePoints()
{
  points_.clear();

  for (const auto &element : path_.elements()) {
    points_.push_back(element.pos);
  }
}

void
Path3DObj::
updateGL()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  //---

  // bind the Vertex Array Object
  canvas_->glBindVertexArray(vertexArrayId_);

  //---

  int np = points_.size();

  // store point data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, pointsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D), &points_[0], GL_STATIC_DRAW);

  // set points attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(0);

  //---

  canvas_->glBindBuffer(GL_ARRAY_BUFFER, 0);
//canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,  0);

  canvas_->glBindVertexArray(0);
}

void
Path3DObj::
render()
{
  updateGL();

  //---

  auto *program = s_program->program;

  program->bind();

  program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  modelMatrix_ = CGLMatrix3D::identity();
  modelMatrix_.translated(float(position().getX()),
                          float(position().getY()),
                          float(position().getZ()));
  modelMatrix_.rotated(xAngle(), CGLVector3D(1.0, 0.0, 0.0));
  modelMatrix_.rotated(yAngle(), CGLVector3D(0.0, 1.0, 0.0));
  modelMatrix_.rotated(zAngle(), CGLVector3D(0.0, 0.0, 1.0));

  program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

  //---

  canvas_->glBindVertexArray(vertexArrayId_);

  //---

  auto np = points_.size();

  glDrawArrays(GL_LINES, 0, np);
}

//---

bool
Axis3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Axis3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Axis3DObj::
Axis3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
  path_ = new Path3DObj(canvas);

  (void) canvas_->addNewObject(path_);

  path_->init();
}

void
Axis3DObj::
init()
{
}

QVariant
Axis3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

void
Axis3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

  if      (name == "start") {
    start_ = stringToVector3D(tcl, value);

    needsUpdate_ = true;
  }
  else if (name == "end") {
    end_ = stringToVector3D(tcl, value);

    needsUpdate_ = true;
  }
  else if (name == "min") {
    min_ = Util::stringToReal(value);

    needsUpdate_ = true;
  }
  else if (name == "max") {
    max_ = Util::stringToReal(value);

    needsUpdate_ = true;
  }
  else {
    Object3D::setValue(name, value, args);
  }
}

void
Axis3DObj::
tick()
{
  updateObjects();
}

void
Axis3DObj::
updateObjects()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  //---

  path_->setLine(start_, end_);

  CInterval interval(min_, max_);

  ticks_.clear();

  for (uint i = 0; i <= interval.calcNumMajor(); ++i) {
    double x = interval.interval(i);

    ticks_.push_back(x);
  }

  auto nt = ticks_.size();

  while (textObjs_.size() > nt) {
    auto *text = textObjs_.back();

    textObjs_.pop_back();

    canvas_->removeObject(text);

    delete text;
  }

  while (textObjs_.size() < nt) {
    auto *text = new Text3DObj(canvas_);

    (void) canvas_->addNewObject(text);

    text->init();

    textObjs_.push_back(text);
  }

  for (uint i = 0; i < nt; ++i)
    textObjs_[i]->setText(QString::number(ticks_[i]));

  updateModelMatrix();
}

void
Axis3DObj::
updateModelMatrix()
{
  modelMatrix_ = CGLMatrix3D::identity();

  modelMatrix_.translated(float(position().getX()),
                          float(position().getY()),
                          float(position().getZ()));

  modelMatrix_.rotated(xAngle(), CGLVector3D(1.0, 0.0, 0.0));
  modelMatrix_.rotated(yAngle(), CGLVector3D(0.0, 1.0, 0.0));
  modelMatrix_.rotated(zAngle(), CGLVector3D(0.0, 0.0, 1.0));

  //---

  auto setTextPos = [&](uint i, const CGLVector3D &p) {
    float x, y, z;
    modelMatrix_.multiplyPoint(p.x(), p.y(), p.z(), &x, &y, &z);

    auto p1 = CPoint3D(x, y, z);

    textObjs_[i]->setPosition(p1);
  };

  auto nt = ticks_.size();

  for (uint i = 0; i < nt; ++i) {
    auto r = CMathUtil::map(ticks_[i], min_, max_, 0.0, 1.0);

    auto x = CMathUtil::map(r, 0.0, 1.0, start_.getX(), end_.getX());
    auto y = CMathUtil::map(r, 0.0, 1.0, start_.getY(), end_.getY());
    auto z = CMathUtil::map(r, 0.0, 1.0, start_.getZ(), end_.getZ());

    setTextPos(i, CGLVector3D(x, y, z));
  }
}

void
Axis3DObj::
render()
{
}

//---

}
