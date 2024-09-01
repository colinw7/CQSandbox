#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>
#include <CQGLUtil.h>

#include <CImportBase.h>
#include <CGeomScene3D.h>
#include <CGeomTexture.h>
#include <CLorenzCalc.h>

#include <CQGLBuffer.h>
#include <CQGLTexture.h>
#include <CGLTexture.h>
#include <CGLCamera.h>

#include <CFile.h>
#include <CMinMax.h>

#include <QFileInfo>
#include <QMouseEvent>
#include <QTimer>

#define Q(x) #x
#define QUOTE(x) Q(x)

namespace CQSandbox {

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
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<ShapeObj>),
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
  for (auto *obj : objects_)
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
    auto modelMatrix = CGLMatrix3D::identity();
    modelMatrix.scaled(sceneScale_, sceneScale_, sceneScale_);
    modelMatrix.rotated(xAngle(), CGLVector3D(1.0, 0.0, 0.0));
    modelMatrix.rotated(yAngle(), CGLVector3D(0.0, 1.0, 0.0));
    modelMatrix.rotated(zAngle(), CGLVector3D(0.0, 0.0, 1.0));
    modelMatrix.translated(float(-sceneCenter_.getX()),
                           float(-sceneCenter_.getY()),
                           float(-sceneCenter_.getZ()));
    s_modelShaderProgram->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix));

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

ProgramData *ShapeObj::s_program   = nullptr;

bool
ShapeObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new ShapeObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

ShapeObj::
ShapeObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

QVariant
ShapeObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

void
ShapeObj::
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
    position_ = stringToPoint3D(tcl, value);

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
ShapeObj::
setTexture(const QString &filename)
{
  texture_ = new CQGLTexture;

  if (! texture_->load(filename, /*flip*/true)) {
    delete texture_;
    texture_ = nullptr;
  }
}

void
ShapeObj::
init()
{
  modelMatrix_ = CGLMatrix3D::identity();

  //---

  if (! s_program) {
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "layout (location = 1) in vec3 aColor;\n"
      "layout (location = 2) in vec2 aTexCoord;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "out vec3 ourColor;\n"
      "out vec2 TexCoord;\n"
      "void main()\n"
      "{\n"
      "   gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
      "   ourColor = aColor;\n"
      "   TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
      "}";
    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "out vec4 FragColor;\n"
      "in vec3 ourColor;\n"
      "in vec2 TexCoord;\n"
      "uniform sampler2D textureId;\n"
      "uniform bool useTexture;\n"
      "void main()\n"
      "{\n"
      "   if (useTexture) {\n"
      "     FragColor = texture(textureId, TexCoord);\n"
      "   } else {\n"
      "     FragColor = vec4(ourColor, 1.0f);\n"
      "   }\n"
      "}\n";

    s_program = new ProgramData;

    s_program->program = new QOpenGLShaderProgram(this);

    s_program->program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShaderSource);
    s_program->program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    s_program->program->link();
  }

  //---

  canvas_->glGenVertexArrays(1, &vertexArrayId_);

  canvas_->glGenBuffers(1, &vertexBufferId_);
  canvas_->glGenBuffers(1, &colorBufferId_);
  canvas_->glGenBuffers(1, &texCoordBufferId_);

  canvas_->glGenBuffers(1, &indBufferId_);
}

void
ShapeObj::
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
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D), &points_[0], GL_STATIC_DRAW);

  // set points attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(0);

  //---

  int nc = colors_.size();

  // store color data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, colorBufferId_);
  if (nc > 0) {
    canvas_->glBufferData(GL_ARRAY_BUFFER, nc*sizeof(CGLVector3D), &colors_[0], GL_STATIC_DRAW);
  }
  else {
    static Colors s_colors_;

    if (int(s_colors_.size()) != np) {
      s_colors_.resize(np);

      for (int i = 0; i < np; ++i) {
        double r = (np == 0 ? 1.0 : 0.0);
        double g = (np == 1 ? 1.0 : 0.0);
        double b = (np == 2 ? 1.0 : 0.0);

        s_colors_[i] = CGLVector3D(r, g, b);
      }
    }

    canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D),
                          &s_colors_[0], GL_STATIC_DRAW);
  }

  // set colors attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(1);

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
  canvas_->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(CGLVector2D), nullptr);
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
ShapeObj::
render()
{
  updateGL();

  if (wireframe_)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  //---

  s_program->program->bind();

  s_program->program->setUniformValue("projection",
    CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program->program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  auto modelMatrix = CGLMatrix3D::identity();
  modelMatrix.translated(float(position_.getX()),
                         float(position_.getY()),
                         float(position_.getZ()));
  modelMatrix.scaled(scale(), scale(), scale());
  modelMatrix.rotated(xAngle(), CGLVector3D(1.0, 0.0, 0.0));
  modelMatrix.rotated(yAngle(), CGLVector3D(0.0, 1.0, 0.0));
  modelMatrix.rotated(zAngle(), CGLVector3D(0.0, 0.0, 1.0));
  s_program->program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix));

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
  else
    glDrawArrays(GL_TRIANGLES, 0, np);

  //canvas_->glBindVertexArray(0);

  if (texture_) {
    glDisable(GL_TEXTURE_2D);
  }
}

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
 ShapeObj(canvas)
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

  ShapeObj::init();
}

//---

bool
ShaderObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new ShaderObj(canvas);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

ShaderObj::
ShaderObj(Canvas3D *canvas) :
 Object3D(canvas)
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

  tcl->setResult(name);

  return true;
}

ParticleListObj::
ParticleListObj(Canvas3D *canvas) :
 Object3D(canvas)
{
  updateGL();
}

QVariant
ParticleListObj::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "size")
    return Util::intToString(int(points_.size()));
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
updateGL()
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

    s_program->modelUniform = s_program->program->uniformLocation("model");
    Q_ASSERT(s_program->modelUniform != -1);
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
render()
{
  s_program->program->bind();

  s_program->program->setUniformValue("projection",
    CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program->program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));
  auto modelMatrix = CGLMatrix3D::identity();
  s_program->program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix));

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

}
