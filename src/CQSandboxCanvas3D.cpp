#include <CQSandboxCanvas3D.h>
#include <CQSandboxToolbar3D.h>
#include <CQSandboxLight3D.h>
#include <CQSandboxModel3DObj.h>
#include <CQSandboxSkybox3DObj.h>
#include <CQSandboxCsv3DObj.h>
#include <CQSandboxShape3DObj.h>
#include <CQSandboxShaderShape3DObj.h>
#include <CQSandboxCube3DObj.h>
#include <CQSandboxPlane3DObj.h>
#include <CQSandboxShader3DObj.h>
#include <CQSandboxGraph3DObj.h>
#include <CQSandboxSurface3DObj.h>
#include <CQSandboxText3DObj.h>
#include <CQSandboxAxis3DObj.h>
#include <CQSandboxSprite3DObj.h>
#include <CQSandboxOthello3DObj.h>
#include <CQSandboxDungeon3DObj.h>
#include <CQSandboxPath3DObj.h>
#include <CQSandboxParticleList3DObj.h>
#include <CQSandboxGroup3DObj.h>
#include <CQSandboxBBox3DObj.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>
#include <CQSandboxShaderToyProgram.h>

#include <CGLCamera.h>
#include <CQGLUtil.h>

#include <QMouseEvent>
#include <QTimer>

#define Q(x) #x
#define QUOTE(x) Q(x)

namespace CQSandbox {

//---

QString Canvas3D::s_buildDir = QUOTE(BUILD_DIR);

//---

template<typename T>
int createObjectProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv) {
  auto *th = static_cast<Canvas3D *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);

  if (! T::create(th, args))
    return TCL_ERROR;

  return TCL_OK;
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

//std::cerr << "OpenGL: " << glGetString(GL_VERSION) << "\n";

#if 0
  auto strs = QString(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS))).split(" ");
  strs.sort();

  for (const auto &str : strs)
    std::cerr << " " << str.toStdString() << "\n";
#endif

//if (! hasOpenGLFeature(QOpenGLFunctions::Framebuffers))
//  std::cerr << "No frame buffer support\n";

#if 0
  int argc = 1;
  const char *argv[] = { "test", nullptr };
  glutInit(&argc, (char **) argv);
#endif
}

void
OpenGLWindow::
resizeGL(int, int)
{
  const qreal retinaScale = devicePixelRatio();

  pixelWidth_  = width ()*retinaScale;
  pixelHeight_ = height()*retinaScale;

  glViewport(0, 0, pixelWidth_, pixelHeight_);

  aspect_ = double(pixelWidth_)/double(pixelHeight_);
}

void
OpenGLWindow::
paintGL()
{
  glClearColor(bgColor_.redF(), bgColor_.greenF(), bgColor_.blueF(), 1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  //---

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

bool
OpenGLWindow::
event(QEvent *event)
{
  return QOpenGLWidget::event(event);
}

//---

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
  app_->runTclCmd("proc click { args } { }");
  app_->runTclCmd("proc keyPress { args } { }");
  app_->runTclCmd("proc tick { args } { }");
  app_->runTclCmd("proc setMode { args } { }");

  timer_ = new QTimer(this);

  connect(timer_, &QTimer::timeout, this, &Canvas3D::timerSlot);

  timer_->start(redrawTimeOut());
}

void
Canvas3D::
setType(const Type &type)
{
  if (type != type_) {
    type_ = type;

    QString mode;

    switch (type_) {
      case Type::CAMERA: mode = "camera"; break;
      case Type::LIGHT : mode = "light" ; break;
      case Type::MODEL : mode = "model" ; break;
      case Type::GAME  : mode = "game"  ; break;
    }

    app_->runTclCmd(QString("setMode {%1}").arg(mode));

    Q_EMIT typeChanged();
  }
}

void
Canvas3D::
addCommands()
{
  auto *tcl = app_->tcl();

  // global
  tcl->createObjCommand("sb3d::canvas",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas3D::canvasProc),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::camera",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas3D::cameraProc),
    static_cast<CQTcl::ObjCmdData>(this));

#if 0
  tcl->createObjCommand("sb3d::load_model",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas3D::loadModelProc),
    static_cast<CQTcl::ObjCmdData>(this));
#endif

  //---

  // data
  tcl->createObjCommand("sb3d::csv",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Csv3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  //---

  // objects
  tcl->createObjCommand("sb3d::group",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Group3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::shape",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Shape3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::shader_shape",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<ShaderShape3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::cube",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Cube3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::bbox",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<BBox3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::plane",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Plane3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::particle_list",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<ParticleList3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::shader",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Shader3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::model",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Model3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::graph",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Graph3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::surface",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Surface3DObj>),
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

  tcl->createObjCommand("sb3d::sprite",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Sprite3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::skybox",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Skybox3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

#ifdef CQSANDBOX_OTHELLO
  tcl->createObjCommand("sb3d::othello",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Othello3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));
#endif

#ifdef CQSANDBOX_FIELD_RUNNERS
  tcl->createObjCommand("sb3d::field_runners",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<FieldRunners3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));
#endif

#ifdef CQSANDBOX_DUNGEON
  tcl->createObjCommand("sb3d::dungeon",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Dungeon3DObj>),
    static_cast<CQTcl::ObjCmdData>(this));
#endif

  tcl->createObjCommand("sb3d::custom_form",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas3D::customFormProc),
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

  allObjects_.push_back(obj);

  obj->setInd(++lastInd_);

  createObjCommand(obj);

  return obj->calcId();
}

void
Canvas3D::
addObject(Object3D *obj)
{
  objects_.push_back(obj);

  obj->setGroup(nullptr);

  Q_EMIT objectsChanged();
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

  Q_EMIT objectsChanged();
}

Object3D *
Canvas3D::
getObjectByName(const QString &name) const
{
  for (auto *obj : allObjects_) {
    if (name == obj->getCommandName())
      return obj;
  }

  for (auto *obj : allObjects_) {
    if (name == obj->calcId())
      return obj;
  }

  return nullptr;
}

int
Canvas3D::
canvasProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas3D *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);
  if (args.size() < 1) return TCL_ERROR;

  auto *tcl = th->app_->tcl();

  if      (args[0] == "get") {
    if (args.size() >= 2) {
      auto name = args[1];

      QStringList args1;
      for (int i = 2; i < args.length(); ++i)
        args1.push_back(args[i]);

      auto res = th->getValue(name, args1);

      tcl->setResult(res);
    }
  }
  else if (args[0] == "set") {
    if (args.size() >= 3) {
      auto name  = args[1];
      auto value = args[2];

      QStringList args1;
      for (int i = 3; i < args.length(); ++i)
        args1.push_back(args[i]);

      th->setValue(name, value, args1);
    }
  }
  else if (args[0] == "delete") {
    if (args.size() >= 2) {
      if (args[1] == "all") {
        Objects objects;

        std::swap(objects, th->objects_);

        for (auto *obj : objects)
          delete obj;

      }
    }
  }
  else
    return TCL_ERROR;

  return TCL_OK;
}

int
Canvas3D::
cameraProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas3D *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);
  if (args.size() < 1) return TCL_ERROR;

  auto *tcl = th->app_->tcl();

  if      (args[0] == "get") {
    if (args.size() >= 2) {
      auto name = args[1];

      QStringList args1;
      for (int i = 2; i < args.length(); ++i)
        args1.push_back(args[i]);

      auto res = th->getCameraValue(name, args1);

      tcl->setResult(res);
    }
  }
  else if (args[0] == "set") {
    if (args.size() >= 3) {
      auto name  = args[1];
      auto value = args[2];

      QStringList args1;
      for (int i = 3; i < args.length(); ++i)
        args1.push_back(args[i]);

      th->setCameraValue(name, value, args1);
    }
  }
  else
    return TCL_ERROR;

  return TCL_OK;
}

int
Canvas3D::
customFormProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas3D *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);
  auto nargs = args.size();

  if (nargs < 1) return TCL_ERROR;

//auto *tcl = th->app_->tcl();

  if      (args[0] == "string") {
    QString label, proc;

    for (int i = 1; i < nargs; ++i) {
      if      (args[i] == "-label") {
        ++i;

        if (i >= nargs)
          return TCL_ERROR;

        label = args[i];
      }
      else if (args[i] == "-proc") {
        ++i;

        if (i >= nargs)
          return TCL_ERROR;

        proc = args[i];
      }
      else
        return TCL_ERROR;
    }
  }
  else if (args[0] == "number") {
  }

  return TCL_OK;
}

QVariant
Canvas3D::
getValue(const QString &name, const QStringList &args)
{
//auto *tcl = app_->tcl();

  if      (name == "bg")
    return Util::colorToString(bgColor());
  else if (name == "xmap") {
    if (args.size() >= 1) {
      auto x = Util::stringToReal(args[0]);

      auto x1 = xrange_.map(x, -0.5, 0.5);

      return QVariant(x1);
    }
    else {
      app_->errorMsg(QString("Missing value for '%1'").arg(name));
      return QVariant();
    }
  }
  else if (name == "ymap") {
    if (args.size() >= 1) {
      auto y = Util::stringToReal(args[0]);

      auto y1 = yrange_.map(y, -0.5, 0.5);

      return QVariant(y1);
    }
    else {
      app_->errorMsg(QString("Missing value for '%1'").arg(name));
      return QVariant();
    }
  }
  else if (name == "zmap") {
    if (args.size() >= 1) {
      auto z = Util::stringToReal(args[0]);

      auto z1 = zrange_.map(z, -0.5, 0.5);

      return QVariant(z1);
    }
    else {
      app_->errorMsg(QString("Missing value for '%1'").arg(name));
      return QVariant();
    }
  }
  else if (name == "lights.simple") {
    return QVariant(isSimpleLights());
  }
  else if (name == "depth_test") {
    return QVariant(isDepthTest());
  }
  else if (name == "cull_face") {
    return QVariant(isCullFace());
  }
  else if (name == "lighting") {
    return QVariant(isLighting());
  }
  else if (name == "front_face") {
    return QVariant(isFrontFace());
  }
  else if (name == "smooth_shade") {
    return QVariant(isSmoothShade());
  }
  else {
    app_->errorMsg(QString("Invalid value name '%1'").arg(name));
    return QVariant();
  }
}

bool
Canvas3D::
setValue(const QString &name, const QString &value, const QStringList &)
{
  auto *tcl = app_->tcl();

  if      (name == "bg") {
    setBgColor(Util::stringToColor(value));
  }
  else if (name == "mode") {
    if      (value == "camera")
      setType(Type::CAMERA);
    else if (value == "light")
      setType(Type::LIGHT);
    else if (value == "model")
      setType(Type::MODEL);
    else if (value == "game")
      setType(Type::GAME);
  }
  else if (name == "xrange") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() != 2) {
      app_->errorMsg("Invalid values for range");
      return false;
    }

    double xmin = Util::stringToReal(strs[0]);
    double xmax = Util::stringToReal(strs[1]);

    xrange_ = CRMinMax(xmin, xmax);
  }
  else if (name == "yrange") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() != 2) {
      app_->errorMsg("Invalid values for range");
      return false;
    }

    double xmin = Util::stringToReal(strs[0]);
    double xmax = Util::stringToReal(strs[1]);

    yrange_ = CRMinMax(xmin, xmax);
  }
  else if (name == "zrange") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() != 2) {
      app_->errorMsg("Invalid values for range");
      return false;
    }

    double xmin = Util::stringToReal(strs[0]);
    double xmax = Util::stringToReal(strs[1]);

    zrange_ = CRMinMax(xmin, xmax);
  }
  else if (name == "lights.simple") {
    setSimpleLights(true);
  }
  else if (name == "depth_test") {
    setDepthTest(Util::stringToBool(value));
  }
  else if (name == "cull_face") {
    setCullFace(Util::stringToBool(value));
  }
  else if (name == "lighting") {
    setLighting(Util::stringToBool(value));
  }
  else if (name == "front_face") {
    setFrontFace(Util::stringToBool(value));
  }
  else if (name == "smooth_shade") {
    setSmoothShade(Util::stringToBool(value));
  }
  else {
    app_->errorMsg(QString("Invalid value name '%1'").arg(name));
    return false;
  }

  return true;
}

QVariant
Canvas3D::
getCameraValue(const QString &name, const QStringList &)
{
//auto *tcl = app_->tcl();

  if      (name == "zoom")
    return Util::realToString(camera()->zoom());
  else if (name == "near")
    return Util::realToString(camera()->near());
  else if (name == "far")
    return Util::realToString(camera()->far());
  else if (name == "yaw")
    return Util::realToString(camera()->yaw());
  else if (name == "pitch")
    return Util::realToString(camera()->pitch());
  else if (name == "position")
    return Util::vector3DToString(camera()->position());
  else {
    app_->errorMsg(QString("Invalid value name '%1'").arg(name));
    return QVariant();
  }
}

void
Canvas3D::
setCameraValue(const QString &name, const QString &value, const QStringList &)
{
  auto *tcl = app_->tcl();

  if      (name == "zoom")
    camera()->setZoom(Util::stringToReal(value));
  else if (name == "near")
    camera()->setNear(Util::stringToReal(value));
  else if (name == "far")
    camera()->setFar(Util::stringToReal(value));
  else if (name == "yaw")
    camera()->setYaw(Util::stringToReal(value));
  else if (name == "pitch")
    camera()->setPitch(Util::stringToReal(value));
  else if (name == "position")
    camera()->setPosition(Util::stringToVector3D(tcl, value));
  else
    app_->errorMsg(QString("Invalid value name '%1'").arg(name));
}

#if 0
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

  auto *model = new Model3DObj(th);

  if (! model->load(filename)) {
    delete model;
    return TCL_ERROR;
  }

  auto name = th->addNewObject(model);

  auto *tcl = th->app()->tcl();

  tcl->setResult(name);

  return TCL_OK;
}
#endif

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
  else if (args[0] == "exec") {
    if (args.size() > 1) {
      QString op = args[1];

      QStringList args1;
      for (int i = 2; i < args.length(); ++i)
        args1.push_back(args[i]);

      auto res = obj->exec(op, args1);

      tcl->setResult(res);
    }
    else {
      app->errorMsg("Missing args for exec");
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

  eyeLine_ = new Path3DObj(this);

  (void) addNewObject(eyeLine_);

  eyeLine_->init();
  eyeLine_->setVisible(false);
  eyeLine_->setPseudo(true);

  //---

  intersectParticles_ = new ParticleList3DObj(this);

  (void) addNewObject(intersectParticles_);

  intersectParticles_->init();
  intersectParticles_->setVisible(false);
  intersectParticles_->setPseudo(true);

  //---

  app_->runTclCmd("init");
}

void
Canvas3D::
setRedrawTimeOut(int t)
{
  redrawTimeOut_ = t;

  timer_->stop();

  timer_->start(redrawTimeOut());
}

Light3D *
Canvas3D::
currentLight() const
{
  auto *th = const_cast<Canvas3D *>(this);

  if (lights_.empty())
    th->lights_.push_back(new Light3D(th, Light3D::Type::DIRECTIONAL));

  while (th->lightNum_ >= lights_.size())
    th->lightNum_ -= int(lights_.size());

  return lights_[lightNum_];
}

void
Canvas3D::
updateLights()
{
  if (isSimpleLights())
    return;

  //---

  auto numLights = numDirectionalLights_ + numPointLights_ + numSpotLights_;
  if (numLights == lights_.size()) return;

  while (lights_.size() < numLights)
    lights_.push_back(new Light3D(this, Light3D::Type::DIRECTIONAL));

  int il = 0;

  for (auto *light : lights_)
    light->setId(il++);

  il = 0;

  lights_[il]->setType     (Light3D::Type::DIRECTIONAL);
  lights_[il]->setEnabled  (true);
  lights_[il]->setColor    (CGLVector3D(1, 1, 0));
  lights_[il]->setDirection(CGLVector3D(1, -1, -1));
  lights_[il]->setIntensity(1.0f);

  ++il;

  lights_[il]->setType     (Light3D::Type::POINT);
  lights_[il]->setEnabled  (true);
  lights_[il]->setColor    (CGLVector3D(1, 0, 0));
  lights_[il]->setPosition (CGLVector3D(-0.5, 0.35, 0.25));
  lights_[il]->setRadius   (1.0f);
  lights_[il]->setIntensity(1.0f);

  ++il;

  lights_[il]->setType     (Light3D::Type::POINT);
  lights_[il]->setEnabled  (true);
  lights_[il]->setColor    (CGLVector3D(0, 1, 0));
  lights_[il]->setPosition (CGLVector3D(0.5, 0.35, 0.25));
  lights_[il]->setRadius   (1.0f);
  lights_[il]->setIntensity(1.0f);

  ++il;

  auto cpos = camera()->position();

  lights_[il]->setType     (Light3D::Type::SPOT);
  lights_[il]->setEnabled  (true);
  lights_[il]->setColor    (CGLVector3D(0, 0, 1));
  lights_[il]->setPosition (cpos + CGLVector3D(0.0, 0.5, 0));
  lights_[il]->setDirection(CGLVector3D(0, 0, -1));
  lights_[il]->setIntensity(1.0);
  lights_[il]->setCutoff   (cos(35.0f));

  ++il;

  lights_[il]->setType     (Light3D::Type::SPOT);
  lights_[il]->setEnabled  (true);
  lights_[il]->setColor    (CGLVector3D(0, 1, 1));
  lights_[il]->setPosition (cpos + CGLVector3D(0.0, -0.5, 0));
  lights_[il]->setDirection(CGLVector3D(0, 0, -1));
//lights_[il]->setIntensity(1.5);
  lights_[il]->setCutoff   (cos(15.0f));

  //++il;
}

void
Canvas3D::
setProgramLights(ShaderProgram *program)
{
  if (isSimpleLights()) {
    auto *light = lights_[0];

    program->setUniformValue("lightPos"  , CQGLUtil::toVector(light->position()));
    program->setUniformValue("lightColor", CQGLUtil::toVector(light->color()));

    return;
  }

  //---

  static char nameStr[256];

  auto STR = [&](const QString &str) {
    assert(str.length() < 255);
    strcpy(nameStr, str.toStdString().c_str());
    return nameStr;
  };

  int indD = 0, indP = 0, indS = 0;

  for (const auto *light : lights()) {
    if (light->type() == Light3D::Type::DIRECTIONAL) {
      auto lightName = QString("directionalLights[%1]").arg(indD++);

      program->setUniformValue(
       STR(lightName + ".enabled"  ), light->isEnabled());
      program->setUniformValue(
       STR(lightName + ".direction"), CQGLUtil::toVector(light->direction()));
      program->setUniformValue(
       STR(lightName + ".color"    ), CQGLUtil::toVector(light->color()));
    }
    else if (light->type() == Light3D::Type::POINT) {
      auto lightName = QString("pointLights[%1]").arg(indP++);

      program->setUniformValue(
       STR(lightName + ".enabled" ), light->isEnabled());
      program->setUniformValue(
       STR(lightName + ".position"), CQGLUtil::toVector(light->position()));
      program->setUniformValue(
       STR(lightName + ".color"   ), CQGLUtil::toVector(light->color()));
      program->setUniformValue(
       STR(lightName + ".radius"  ), light->radius());
    }
    else if (light->type() == Light3D::Type::SPOT) {
      auto lightName = QString("spotLights[%1]").arg(indS++);

      program->setUniformValue(
       STR(lightName + ".enabled"  ), light->isEnabled());
      program->setUniformValue(
       STR(lightName + ".position" ), CQGLUtil::toVector(light->position()));
      program->setUniformValue(
       STR(lightName + ".direction"), CQGLUtil::toVector(light->direction()));
      program->setUniformValue(
       STR(lightName + ".color"    ), CQGLUtil::toVector(light->color()));
      program->setUniformValue(
       STR(lightName + ".cutoff"   ), light->cutoff());
    }
  }
}

Object3D *
Canvas3D::
objectFromInd(uint ind) const
{
  for (auto *obj : objects())
    if (obj->ind() == ind)
      return obj;

  return nullptr;
}

Object3D *
Canvas3D::
getCurrentObject() const
{
  Object3D *firstObj = nullptr;

  for (auto *obj : objects()) {
    if (obj->isSelected())
      return obj;

    if (! firstObj && ! obj->isPseudo())
      firstObj = obj;
  }

  if (! firstObj && ! objects().empty())
    return objects().front();

  return firstObj;
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
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  for (auto *obj : objects_) {
    if (! obj || ! obj->isVisible())
      continue;

    obj->preRender();
  }

  glPopAttrib();

  //---

  updateLights();

  //---

  glDepthMask(GL_TRUE);

  isDepthTest() ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
  isCullFace () ? glEnable(GL_CULL_FACE ) : glDisable(GL_CULL_FACE );
//isLighting () ? glEnable(GL_LIGHTING  ) : glDisable(GL_LIGHTING  );

  isFrontFace() ? glFrontFace(GL_CW) : glFrontFace(GL_CCW);

  isSmoothShade() ? glShadeModel(GL_SMOOTH) : glShadeModel(GL_FLAT);

//isOutline() ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) :
//              glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glDepthFunc(GL_LEQUAL);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  //---

  projectionMatrix_ = camera_->getPerspectiveMatrix(aspect_);
  viewMatrix_       = camera_->getViewMatrix();

  //---

  viewPos_ = camera_->position();

  //---

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  for (auto *obj : objects_) {
    if (! obj || ! obj->isVisible())
      continue;

    obj->render();
  }

  glPopAttrib();

  //---

  for (auto *light : lights())
    light->render();
}

void
Canvas3D::
mousePressEvent(QMouseEvent *e)
{
  float xpos = float(e->x());
  float ypos = float(e->y());

  setMousePos(xpos, ypos);

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
  Objects clickObjs;

  for (auto *obj : objects_) {
    if (! obj->isVisible())
      continue;

    if (obj->isInside()) {
      clickObjs.push_back(obj);
      obj->setInside(false);

      obj->setNeedsUpdate();
    }
  }

  eyeLine_           ->setVisible(false);
  intersectParticles_->setVisible(false);

  update();

  pressed_ = false;

  if (clickObjs.size() == 1) {
    auto *clickObj = clickObjs[0];

    for (auto *obj : objects())
      obj->setSelected(obj == clickObj);

    app_->runTclCmd(QString("click {%1}").arg(clickObj->id()));
  }
}

void
Canvas3D::
mouseMoveEvent(QMouseEvent *e)
{
  float xpos = float(e->x());
  float ypos = float(e->y());

  setMousePos(xpos, ypos);

  if (pressed_) {
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
setMousePos(float xpos, float ypos)
{
  // unobserve
  auto x1 = CMathUtil::map(xpos, 0, pixelWidth_  - 1, -1,  1);
  auto y1 = CMathUtil::map(ypos, 0, pixelHeight_ - 1,  1, -1);
  auto z1 = 0.1;

  auto x2 = x1;
  auto y2 = y1;
  auto z2 = 10.0;

  auto imatrix1 = projectionMatrix_.inverse();

  float xp1, yp1, zp1;
  imatrix1.multiplyPoint(x1, y1, z1, &xp1, &yp1, &zp1);
  float xp2, yp2, zp2;
  imatrix1.multiplyPoint(x2, y2, z2, &xp2, &yp2, &zp2);

  const auto &viewMatrix = camera_->getViewMatrix();
  auto imatrix2 = viewMatrix.inverse();

  float xv1, yv1, zv1;
  imatrix2.multiplyPoint(xp1, yp1, zp1, &xv1, &yv1, &zv1);

  float xv2, yv2, zv2;
  imatrix2.multiplyPoint(xp2, yp2, zp2, &xv2, &yv2, &zv2);

  app_->toolbar3D()->setPos(QString("%1 %2 %3").arg(xv1).arg(yv1).arg(zv1));

  CGLVector3D pe1(xv1, yv1, zv1);
  CGLVector3D pe2(xv2, yv2, zv2);

  eyeLine_->setLine(pe1, pe2);
  eyeLine_->setVisible(true);

  //---

  intersectPoints_.clear();

  for (auto *obj : objects_) {
    if (! obj->isVisible())
      continue;

    auto imodelMatrix = obj->modelMatrix().inverse();

    float mx1, my1, mz1;
    imodelMatrix.multiplyPoint(xv1, yv1, zv1, &mx1, &my1, &mz1);

    float mx2, my2, mz2;
    imodelMatrix.multiplyPoint(xv2, yv2, zv2, &mx2, &my2, &mz2);

    CGLVector3D pm1(mx1, my1, mz1);
    CGLVector3D pm2(mx2, my2, mz2);

    CPoint3D pi1, pi2;

    bool inside = obj->intersect(pm1, pm2, pi1, pi2);

    if (inside != obj->isInside()) {
      obj->setInside(inside);

      obj->setNeedsUpdate();
    }

    if (inside) {
      auto mapPoint = [&](const CPoint3D &p) {
        float x1, y1, z1;
        obj->modelMatrix().multiplyPoint(p.x, p.y, p.z, &x1, &y1, &z1);
        return CGLVector3D(x1, y1, z1);
      };

      intersectPoints_.push_back(mapPoint(pi1));

      if (pi2 != pi1)
        intersectPoints_.push_back(mapPoint(pi2));
    }
  }

  intersectParticles_->setPoints(intersectPoints_);
  intersectParticles_->setVisible(true);
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

  //---

  auto dt = 0.01f; /* camera_->deltaTime(); */
  auto da = M_PI/180.0;

  if (isShift) {
    dt = -dt;
    da = -da;
  }

  //---

  auto type = this->type();

  if (e->key() == Qt::Key_Escape) {
    if (type == Type::GAME)
      setType(Type::CAMERA);
    else
      setType(Type::GAME);

    update();

    return;
  }

  //---

  if (type == Type::GAME) {
    QString text;

    if      (e->key() == Qt::Key_Left ) text = "left";
    else if (e->key() == Qt::Key_Right) text = "right";
    else if (e->key() == Qt::Key_Up   ) text = "up";
    else if (e->key() == Qt::Key_Down ) text = "down";
    else if (e->key() == Qt::Key_Space) text = "space";
    else                                text = e->text();

    if (text == "")
      text = QString("key.%1").arg(e->key());

    app_->runTclCmd(QString("keyPress {%1}").arg(text));

    update();

    return;
  }

  auto *light = currentLight();

  if      (e->key() == Qt::Key_W) {
    if      (type == Type::CAMERA) {
      camera_->processKeyboard(CGLCamera::Movement::FORWARD, dt);
      Q_EMIT cameraChanged();
    }
    else if (type == Type::LIGHT)
      light->setPosition(light->position() + CGLVector3D(0.0f, 0.1f, 0.0f));
  }
  else if (e->key() == Qt::Key_S) {
    if      (type == Type::CAMERA) {
      camera_->processKeyboard(CGLCamera::Movement::BACKWARD, dt);
      Q_EMIT cameraChanged();
    }
    else if (type == Type::LIGHT)
      light->setPosition(light->position() - CGLVector3D(0.0f, 0.1f, 0.0f));
  }
  else if (e->key() == Qt::Key_A) {
    if      (type == Type::CAMERA) {
      camera_->processKeyboard(CGLCamera::Movement::LEFT, dt);
      Q_EMIT cameraChanged();
    }
    else if (type == Type::LIGHT)
      light->setPosition(light->position() - CGLVector3D(0.1f, 0.0f, 0.0f));
  }
  else if (e->key() == Qt::Key_D) {
    if      (type == Type::CAMERA) {
      camera_->processKeyboard(CGLCamera::Movement::RIGHT, dt);
      Q_EMIT cameraChanged();
    }
    else if (type == Type::LIGHT)
      light->setPosition(light->position() + CGLVector3D(0.1f, 0.0f, 0.0f));
  }
  else if (e->key() == Qt::Key_C) {
    setType(Type::CAMERA);
  }
  else if (e->key() == Qt::Key_L) {
    if (type == Type::LIGHT) {
      setLightNum(lightNum() + 1);
    }
    else {
      setType(Type::LIGHT);
      setLightNum(0);
    }

    Q_EMIT typeChanged();
  }
  else if (e->key() == Qt::Key_M) {
    setType(Type::MODEL);
  }
  else if (e->key() == Qt::Key_X) {
    if (type == Type::MODEL) {
      auto *obj = getCurrentObject();
      if (! obj) return;

      obj->setXAngle(obj->xAngle() + da);
    }
  }
  else if (e->key() == Qt::Key_Y) {
    if (type == Type::MODEL) {
      auto *obj = getCurrentObject();
      if (! obj) return;

      obj->setYAngle(obj->yAngle() + da);
    }
  }
  else if (e->key() == Qt::Key_Z) {
   if (type == Type::MODEL) {
      auto *obj = getCurrentObject();
      if (! obj) return;

      obj->setZAngle(obj->zAngle() + da);
    }
  }
  else if (e->key() == Qt::Key_Up) {
    if      (type == Type::CAMERA) {
      camera_->processKeyboard(CGLCamera::Movement::UP, dt);
      Q_EMIT cameraChanged();
    }
    else if (type == Type::LIGHT)
      light->setPosition(light->position() + CGLVector3D(0.0f, 0.0f, 0.1f));
  }
  else if (e->key() == Qt::Key_Down) {
    if      (type == Type::CAMERA) {
      camera_->processKeyboard(CGLCamera::Movement::DOWN, dt);
      Q_EMIT cameraChanged();
    }
    else if (type == Type::LIGHT)
      light->setPosition(light->position() - CGLVector3D(0.0f, 0.0f, 0.1f));
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

}
