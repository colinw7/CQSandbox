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
#include <CQSandboxCamera.h>
#include <CQSandboxOverview3D.h>
#include <CQSandboxGeomObject.h>
#include <CQSandboxTexture.h>
#include <CQSandboxUtil.h>
#include <CQSandboxShaderToyProgram.h>

#include <CQGLUtil.h>
#include <CQGLBuffer.h>
#include <CGeometry3D.h>

#ifdef CQ_PERF_GRAPH
#include <CQPerfMonitor.h>
#else
struct CQPerfTrace {
  CQPerfTrace(const char *) { }
};
#endif

#include <QMouseEvent>
#include <QTimer>

//---

namespace CQSandbox {

class GeomFactory : public CGeometryFactory {
 public:
  GeomFactory(Canvas3D *canvas) :
   canvas_(canvas) {
  }

 ~GeomFactory() override { }

  CGeomObject3D *createObject3D(CGeomScene3D *pscene, const std::string &name) const override {
    return new GeomObject(pscene, name);
  }

#if 0
  CGeomFace3D *createFace3D() const override {
    return new GeomFace;
  }

  CGeomLine3D *createLine3D() const override {
    return new GeomLine;
  }
#endif

  CGeomLight3D *createLight3D(CGeomScene3D *, const std::string &name) const override {
    auto *light = new Light3D(canvas_);
    light->setName(name);
    return light;
  }

  CGeomTexture *createTexture() const override {
    return new Texture;
  }

 private:
  Canvas3D* canvas_ { nullptr };
};

}

//---

namespace CQSandbox {

//---

template<typename T>
int createObjectProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv) {
  auto *th = static_cast<Canvas3D *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);

  auto *obj = T::create(th, args);

  if (! obj)
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
  CQPerfTrace trace("OpenGLWindow::paintGL");

  //---

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

  //---

  CGeometry3DInst->setFactory(new GeomFactory(this));

  scene_ = CGeometry3DInst->createScene3D();
}

void
Canvas3D::
init()
{
  addCommands();

  //---

  app_->runTclCmd("proc init { args } { }");
  app_->runTclCmd("proc update { args } { }");
  app_->runTclCmd("proc click { args } { }");
  app_->runTclCmd("proc keyPress { args } { }");
  app_->runTclCmd("proc tick { args } { }");
  app_->runTclCmd("proc setMode { args } { }");

  //---

  timer_ = new QTimer(this);

  connect(timer_, &QTimer::timeout, this, &Canvas3D::timerSlot);

  //---

  uiTimer_ = new QTimer(this);

  connect(uiTimer_, &QTimer::timeout, this, &Canvas3D::uiTimerSlot);

  uiTimer_->start(250);

  //---

  initCamera();
}

void
Canvas3D::
initCamera()
{
  camera_ = new Camera(app_);
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

  tcl->createAlias("echo", "puts");

  // global
  tcl->createObjCommand("sb3d::canvas",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas3D::canvasProc),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::camera",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas3D::cameraProc),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::light",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas3D::lightProc),
    static_cast<CQTcl::ObjCmdData>(this));

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

  objectsValid_ = false;

  Q_EMIT objectsChanged();
}

void
Canvas3D::
removeObject(Object3D *obj)
{
  Objects objects;

  for (auto *obj1 : objects_) {
    if (obj != obj1)
      objects.push_back(obj1);
  }

  std::swap(objects_, objects);

  objectsValid_ = false;

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

      QVariant res;
      if (! th->getValue(name, args1, res))
        return TCL_ERROR;

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

      if (! th->setValue(name, value, args1))
        return TCL_ERROR;
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

      th->objectsValid_ = false;

      Q_EMIT th->objectsChanged();
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

      QVariant res;
      if (! th->getCameraValue(name, args1, res))
        return TCL_ERROR;

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

      if (! th->setCameraValue(name, value, args1))
        return TCL_ERROR;
    }
  }
  else if (args[0] == "exec") {
    if (args.size() > 1) {
      QString op = args[1];

      QStringList args1;
      for (int i = 2; i < args.length(); ++i)
        args1.push_back(args[i]);

      QVariant res;
      if (! th->execCamera(op, args1, res))
        return TCL_ERROR;

      tcl->setResult(res);
    }
    else {
      (void) th->app()->errorMsg("Missing args for exec");
      return TCL_ERROR;
    }
  }
  else
    return TCL_ERROR;

  return TCL_OK;
}

int
Canvas3D::
lightProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
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

      QVariant res;
      if (! th->getLightValue(name, args1, res))
        return TCL_ERROR;

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

      if (! th->setLightValue(name, value, args1))
        return TCL_ERROR;
    }
  }
  else if (args[0] == "exec") {
    if (args.size() > 1) {
      QString op = args[1];

      QStringList args1;
      for (int i = 2; i < args.length(); ++i)
        args1.push_back(args[i]);

      QVariant res;
      if (! th->execLight(op, args1, res))
        return TCL_ERROR;

      tcl->setResult(res);
    }
    else {
      (void) th->app()->errorMsg("Missing args for exec");
      return TCL_ERROR;
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

bool
Canvas3D::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
//auto *tcl = app_->tcl();

  if      (name == "bg")
    value = Util::colorToString(bgColor());
  else if (name == "mode") {
    if      (type_ == Type::CAMERA)
      value = "camera";
    else if (type_ == Type::LIGHT)
      value = "light";
    else if (type_ == Type::MODEL)
      value = "model";
    else if (type_ == Type::GAME)
      value = "game";
  }
  else if (name == "loop.enabled") {
    value = QVariant(isLooping());
  }
  else if (name == "loop.timeout") {
    value = QVariant(redrawTimeOut());
  }
  else if (name == "xmap") {
    if (args.size() >= 1) {
      auto x = Util::stringToReal(args[0]);

      auto x1 = xrange_.map(x, -0.5, 0.5);

      value = QVariant(x1);
    }
    else
      return app_->errorMsg(QString("Missing value for '%1'").arg(name));
  }
  else if (name == "ymap") {
    if (args.size() >= 1) {
      auto y = Util::stringToReal(args[0]);

      auto y1 = yrange_.map(y, -0.5, 0.5);

      value = QVariant(y1);
    }
    else
      return app_->errorMsg(QString("Missing value for '%1'").arg(name));
  }
  else if (name == "zmap") {
    if (args.size() >= 1) {
      auto z = Util::stringToReal(args[0]);

      auto z1 = zrange_.map(z, -0.5, 0.5);

      value = QVariant(z1);
    }
    else
      return app_->errorMsg(QString("Missing value for '%1'").arg(name));
  }
  else if (name == "lights.simple") {
    value = QVariant(isSimpleLights());
  }
  else if (name == "depth_test") {
    value = QVariant(isDepthTest());
  }
  else if (name == "cull_face") {
    value = QVariant(isCullFace());
  }
  else if (name == "lighting") {
    value = QVariant(isLighting());
  }
  else if (name == "front_face") {
    value = QVariant(isFrontFace());
  }
  else if (name == "smooth_shade") {
    value = QVariant(isSmoothShade());
  }
  else
    return app_->errorMsg(QString("Invalid value name '%1'").arg(name));

  return true;
}

bool
Canvas3D::
setValue(const QString &name, const QString &value, const QStringList &)
{
  auto *tcl = app_->tcl();

  if      (name == "bg") {
    setBgColor(Util::stringToColor(tcl, value));
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
  else if (name == "loop.enabled") {
    setLooping(Util::stringToBool(value));
  }
  else if (name == "loop.timeout") {
    setRedrawTimeOut(Util::stringToInt(value));
  }
  else if (name == "xrange") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() != 2)
      return app_->errorMsg("Invalid values for range");

    double xmin = Util::stringToReal(strs[0]);
    double xmax = Util::stringToReal(strs[1]);

    xrange_ = CRMinMax(xmin, xmax);
  }
  else if (name == "yrange") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() != 2)
      return app_->errorMsg("Invalid values for range");

    double xmin = Util::stringToReal(strs[0]);
    double xmax = Util::stringToReal(strs[1]);

    yrange_ = CRMinMax(xmin, xmax);
  }
  else if (name == "zrange") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() != 2)
      return app_->errorMsg("Invalid values for range");

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
  else if (name == "model_dir") {
    modelDirs_.push_back(value);
  }
  else
    return app_->errorMsg(QString("Invalid value name '%1'").arg(name));

  return true;
}

bool
Canvas3D::
getCameraValue(const QString &name, const QStringList &, QVariant &res)
{
//auto *tcl = app_->tcl();

  if      (name == "near")
    res = Util::realToString(camera()->near());
  else if (name == "far")
    res = Util::realToString(camera()->far());
  else if (name == "yaw")
    res = Util::realToString(CMathGen::RadToDeg(camera()->yaw()));
  else if (name == "pitch")
    res = Util::realToString(CMathGen::RadToDeg(camera()->pitch()));
  else if (name == "roll")
    res = Util::realToString(CMathGen::RadToDeg(camera()->roll()));
  else if (name == "position")
    res = Util::vector3DToString(camera()->position());
#if 0
  else if (name == "zoom")
    res = Util::realToString(camera()->zoom());
#endif
  else if (name == "disable_roll")
    res = QVariant(camera()->isDisableRoll());
  else
    return app_->errorMsg(QString("Invalid value name '%1'").arg(name));

  return true;
}

bool
Canvas3D::
setCameraValue(const QString &name, const QString &value, const QStringList &)
{
  auto *tcl = app_->tcl();

  if      (name == "near")
    camera()->setNear(Util::stringToReal(value));
  else if (name == "far")
    camera()->setFar(Util::stringToReal(value));
  else if (name == "yaw")
    camera()->setYaw(CMathGen::DegToRad(Util::stringToReal(value)));
  else if (name == "pitch")
    camera()->setPitch(CMathGen::DegToRad(Util::stringToReal(value)));
  else if (name == "roll")
    camera()->setRoll(CMathGen::DegToRad(Util::stringToReal(value)));
  else if (name == "position")
    camera()->setPosition(Util::stringToVector3D(tcl, value));
#if 0
  else if (name == "zoom")
    camera()->setZoom(Util::stringToReal(value));
#endif
  else if (name == "look_at") {
    auto pos = Util::stringToVector3D(tcl, value);

    auto v = CVector3D(camera()->position(), pos);

    auto q = CQuaternion::rotationArc(camera()->front(), v);

    double pitch, yaw, roll;
    q.toEuler(pitch, yaw, roll);

    camera()->setPitch(pitch);
    camera()->setYaw  (yaw);
  }
  else if (name == "disable_roll")
    camera()->setDisableRoll(Util::stringToBool(value));
  else
    return app_->errorMsg(QString("Invalid value name '%1'").arg(name));

  return true;
}

bool
Canvas3D::
execCamera(const QString &op, const QStringList &, QVariant &res)
{
  res = QVariant();

  if      (op == "reset") {
    resetCamera();
  }
  else
    return false;

  return true;
}

void
Canvas3D::
resetCamera()
{
  auto bbox = this->bbox();

  auto center  = bbox.getCenter();
  auto maxSize = bbox.getMaxSize();

  auto s2 = std::sqrt(2.0);

  auto maxSize1 = s2*maxSize + camera_->near();

  auto origin = CVector3D(center.x, center.y, center.z);
  auto pos    = CVector3D(center.x, center.y + maxSize1/2.0, center.z + maxSize1);

  camera_->setPosition(pos);

  camera_->setPitch(-M_PI/6.0);
  camera_->setYaw(0.0);
  camera_->setRoll(0.0);

  camera_->setOrigin(origin);

  camera_->moveAroundX(0.1);
  camera_->moveAroundY(0.1);
}

bool
Canvas3D::
getLightValue(const QString &name, const QStringList &, QVariant &res)
{
  auto *light = currentLight();

  if      (name == "current") {
    res = lightNum();
  }
  else if (name == "position") {
    res = Util::point3DToString(light->getPosition());
  }
  else if (name == "direction") {
    if (light->getType() == CGeomLight3DType::SPOT)
      res = Util::vector3DToString(light->getSpotDirection());
    else
      res = Util::vector3DToString(light->getDirection());
  }
  else
    return app_->errorMsg(QString("Invalid value name '%1'").arg(name));

  return true;
}

bool
Canvas3D::
setLightValue(const QString &name, const QString &value, const QStringList &)
{
  auto *tcl = app_->tcl();

  auto *light = currentLight();

  if      (name == "type") {
    auto lvalue = value.toLower();

    if      (lvalue == "directional")
      light->setType(CGeomLight3DType::DIRECTIONAL);
    else if (lvalue == "point")
      light->setType(CGeomLight3DType::POINT);
    else if (lvalue == "spot")
      light->setType(CGeomLight3DType::SPOT);
    else if (lvalue == "flashlight")
      light->setType(CGeomLight3DType::FLASHLIGHT);
    else
      return app_->errorMsg(QString("Invalid type '%1'").arg(value));
  }
  else if (name == "current") {
    auto n = Util::stringToInt(value);

    setLightNum(n);
  }
  else if (name == "position") {
    auto pos = Util::stringToPoint3D(tcl, value);

    light->setPosition(pos);
  }
  else if (name == "direction") {
    auto dir = Util::stringToVector3D(tcl, value);

    if (light->getType() == CGeomLight3DType::SPOT)
      light->setSpotDirection(dir);
    else
      light->setDirection(dir);
  }
  else if (name == "color" || name == "diffuse") {
    light->setDiffuse(Util::QColorToRGBA(Util::stringToColor(tcl, value)));
  }
  else if (name == "point_radius") {
    auto r = Util::stringToReal(value);

    light->setPointRadius(r);
  }
  else
    return app_->errorMsg(QString("Invalid value name '%1'").arg(name));

  return true;
}

bool
Canvas3D::
execLight(const QString &op, const QStringList &, QVariant &)
{
  return app_->errorMsg(QString("Invalid op '%1'").arg(op));
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

      QVariant res;
      if (! obj->getValue(name, args1, res))
        return TCL_ERROR;

      tcl->setResult(res);
    }
    else {
      (void) app->errorMsg("Missing args for get");
      return TCL_ERROR;
    }
  }
  else if (args[0] == "set") {
    if (args.size() > 2) {
      auto name  = args[1];
      auto value = args[2];

      QStringList args1;
      for (int i = 3; i < args.length(); ++i)
        args1.push_back(args[i]);

      if (! obj->setValue(args[1], args[2], args1))
        return TCL_ERROR;
    }
    else {
      (void) app->errorMsg("Missing args for set");
      return TCL_ERROR;
    }
  }
  else if (args[0] == "exec") {
    if (args.size() > 1) {
      QString op = args[1];

      QStringList args1;
      for (int i = 2; i < args.length(); ++i)
        args1.push_back(args[i]);

      QVariant res;
      if (! obj->exec(op, args1, res))
        return TCL_ERROR;

      tcl->setResult(res);
    }
    else {
      (void) app->errorMsg("Missing args for exec");
      return TCL_ERROR;
    }
  }
  else {
    (void) app->errorMsg(QString("Bad object command '%1'").arg(args[0]));
    return TCL_ERROR;
  }

  return TCL_OK;
}

void
Canvas3D::
initialize()
{
  // camera
//camera_->setLastPos(this->width()/2.0, this->height()/2.0);

  connect(camera_, SIGNAL(stateChangedSignal()), this, SLOT(cameraChangeSlot()));

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

  ignoreChange_ = true;

  app_->runTclCmd("init");

  ignoreChange_ = false;
}

void
Canvas3D::
setLooping(bool b)
{
  if (b != looping_) {
    if (looping_)
      timer_->stop();

    looping_ = b;

    if (looping_)
      timer_->start(redrawTimeOut());
  }
}

void
Canvas3D::
setRedrawTimeOut(int t)
{
  redrawTimeOut_ = t;

  if (looping_) {
    setLooping(false);

    setLooping(true);
  }
}

Light3D *
Canvas3D::
currentLight() const
{
  auto *th = const_cast<Canvas3D *>(this);

  if (lights_.empty()) {
    auto *light = new Light3D(th, Light3D::Type::DIRECTIONAL);

    connect(light, SIGNAL(changedSignal()), this, SLOT(lightChangeSlot()));

    th->lights_.push_back(light);

    Q_EMIT th->lightAdded();
  }

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

  while (lights_.size() < numLights) {
    auto *light = new Light3D(this, Light3D::Type::DIRECTIONAL);

    connect(light, SIGNAL(changedSignal()), this, SLOT(lightChangeSlot()));

    lights_.push_back(light);
  }

  int il = 0;

  for (auto *light : lights_)
    light->setId(il++);

  il = 0;

  // directional
  lights_[il]->setType     (Light3D::Type::DIRECTIONAL);
  lights_[il]->setEnabled  (false);
  lights_[il]->setDiffuse  (CRGBA(1, 1, 1));
  lights_[il]->setDirection(CVector3D(0, -1, 0));

  ++il;

  // point (1)
  lights_[il]->setType       (Light3D::Type::POINT);
  lights_[il]->setEnabled    (true);
  lights_[il]->setDiffuse    (CRGBA(1, 1, 1));
  lights_[il]->setPosition   (CPoint3D(0.0, 1.0, 0.0));
  lights_[il]->setDirection  (CVector3D(0.0, -1.0, 0.0));
  lights_[il]->setPointRadius(8.0);

  ++il;

  // point (2)
  lights_[il]->setType       (Light3D::Type::POINT);
  lights_[il]->setEnabled    (false);
  lights_[il]->setDiffuse    (CRGBA(1, 1, 1));
  lights_[il]->setPosition   (CPoint3D(0.0, -1.0, 0.0));
  lights_[il]->setDirection  (CVector3D(0.0, 1.0, 0.0));
  lights_[il]->setPointRadius(8.0);

  ++il;

  // spot (1)
  auto cpos = camera()->position().point();

  lights_[il]->setType           (Light3D::Type::SPOT);
  lights_[il]->setEnabled        (false);
  lights_[il]->setDiffuse        (CRGBA(1, 1, 1));
  lights_[il]->setPosition       (cpos + CPoint3D(0.0, 0.5, 0));
  lights_[il]->setSpotDirection  (CVector3D(0, 0, -1));
  lights_[il]->setSpotCutOffAngle(std::cos(35.0));

  ++il;

  // spot (2)
  lights_[il]->setType           (Light3D::Type::SPOT);
  lights_[il]->setEnabled        (false);
  lights_[il]->setDiffuse        (CRGBA(1, 1, 1));
  lights_[il]->setPosition       (cpos + CPoint3D(0.0, -0.5, 0));
  lights_[il]->setSpotDirection  (CVector3D(0, 0, -1));
  lights_[il]->setSpotCutOffAngle(std::cos(15.0));

  //++il;

  Q_EMIT lightAdded();
}

void
Canvas3D::
resetLight(Light3D *light)
{
  light->setDirection(CVector3D(0, 1, 0));

  if (bbox_.isSet()) {
    auto center  = bbox_.getCenter();
    auto maxSize = bbox_.getMaxSize();

    light->setPosition(CPoint3D(center.x + maxSize/2, center.y + maxSize/2.0, center.z + maxSize));

    light->setPointRadius(2*maxSize);

    light->setSpotDirection(CVector3D(center.x, center.y, center.z + maxSize));
  }
  else {
    light->setPosition(CPoint3D(0, 1, 0));

    light->setPointRadius(10);

    light->setSpotDirection(CVector3D(0, 0, 1));
  }

  light->setSpotCutOffAngle(45);
}

void
Canvas3D::
setProgramLights(ShaderProgram *program)
{
  if (isSimpleLights()) {
    auto *light = lights_[0];

    program->setUniformValue("lightPos"  , CQGLUtil::toVector(light->getPosition()));
    program->setUniformValue("lightColor", CQGLUtil::toVector(light->getDiffuse()));

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
    QString lightName;

    if     (light->getType() == Light3D::Type::DIRECTIONAL)
      lightName = QString("directionalLights[%1]").arg(indD++);
    else if (light->getType() == Light3D::Type::POINT)
      lightName = QString("pointLights[%1]").arg(indP++);
    else if (light->getType() == Light3D::Type::POINT)
      lightName = QString("pointLights[%1]").arg(indP++);
    else if (light->getType() == Light3D::Type::SPOT)
      lightName = QString("spotLights[%1]").arg(indS++);

    program->setUniformValue(STR(lightName + ".type"), int(light->getType()));
    program->setUniformValue(STR(lightName + ".enabled"), light->getEnabled());

    program->setUniformValue(STR(lightName + ".color"), CQGLUtil::toVector(light->getDiffuse()));

    if (light->getType() == Light3D::Type::DIRECTIONAL) {
      program->setUniformValue(STR(lightName + ".direction"),
                               CQGLUtil::toVector(light->getDirection()));
    }
    else if (light->getType() == Light3D::Type::POINT) {
      program->setUniformValue(STR(lightName + ".position"),
                               CQGLUtil::toVector(light->getPosition()));

      program->setUniformValue(STR(lightName + ".radius"  ), float(light->getPointRadius()));

#if 0
      program->setUniformValue(STR(lightName + ".attenuation0"),
        float(light->getConstantAttenuation()));
      program->setUniformValue(STR(lightName + ".attenuation1"),
        float(light->getLinearAttenuation()));
      program->setUniformValue(STR(lightName + ".attenuation2"),
        float(light->getQuadraticAttenuation()));
#endif
    }
    else if (light->getType() == Light3D::Type::SPOT) {
      program->setUniformValue(STR(lightName + ".position"),
                               CQGLUtil::toVector(light->getPosition()));

      program->setUniformValue(STR(lightName + ".direction"),
                               CQGLUtil::toVector(light->getSpotDirection()));

      auto cutOffCos = std::cos(CMathGen::DegToRad(light->getSpotCutOffAngle()));
      program->setUniformValue(STR(lightName + ".cutoff"), float(cutOffCos));

#if 0
      auto outerCutOffCos = std::cos(CMathGen::DegToRad(light->getSpotOuterCutOffAngle()));

      program->setUniformValue(STR(lightName + ".outerCutoff"), float(outerCutOffCos));

      program->setUniformValue(STR(lightName + ".exponent"), float(light->getSpotExponent()));
#endif
    }
#if 0
    else if (light->getType() == CGeomLight3DType::FLASHLIGHT) {
      // eye direction
      auto cutOffCos      = std::cos(CMathGen::DegToRad(light->getSpotCutOffAngle()));
      auto outerCutOffCos = std::cos(CMathGen::DegToRad(light->getSpotOuterCutOffAngle()));

      program->setUniformValue(STR(lightName + ".cutoff"), float(cutOffCos));
      program->setUniformValue(STR(lightName + ".outerCutoff"), float(outerCutOffCos));

      program->setUniformValue(STR(lightName + ".exponent"), float(light->getSpotExponent()));
    }
#endif
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
  for (auto *animObject : getAnimObjects())
    animObject->stepAnimTime();

  invalidateNodeMatrices();

  //---

  auto objects = objects_;

  for (auto *obj : objects)
    obj->tick();

  app_->runTclCmd("update");

  //---

  update();

  if (app_->overview3D())
    app_->overview3D()->update();
}

void
Canvas3D::
uiTimerSlot()
{
  Q_EMIT uiUpdateSignal();
}

void
Canvas3D::
cameraChangeSlot()
{
  if (ignoreChange_)
    return;

  update();
}

void
Canvas3D::
lightChangeSlot()
{
  if (ignoreChange_)
    return;

  update();
}

void
Canvas3D::
render()
{
  CQPerfTrace trace("Canvas3D::paintGL");

  //---

  if (! objectsValid_) {
    objectsValid_ = true;

    clearObjectMeshData();
  }

  //---

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  for (auto *obj : objects_) {
    if (! obj || ! obj->isVisible())
      continue;

    obj->preRender();
  }

  glPopAttrib();

  currentBuffer_  = nullptr;
  currentProgram_ = nullptr;

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

//projectionMatrix_ = camera_->perspectiveMatrix(aspect_);
  projectionMatrix_ = camera_->perspectiveMatrix();
  viewMatrix_       = camera_->viewMatrix();

  //---

  viewPos_ = camera_->position();

  //---

  bbox_ = CBBox3D();

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  for (auto *obj : objects_) {
    if (! obj || ! obj->isVisible())
      continue;

    if (obj->group())
      continue;

    obj->render();

    bbox_ += obj->bbox();
  }

  glPopAttrib();

  //---

  for (auto *light : lights())
    light->render();

  //---

  //std::cerr << "BBox: " << bbox_ << "\n";

  //---

  bindBuffer(nullptr);
  bindProgram(nullptr);
}

//---

void
Canvas3D::
clearObjectMeshData()
{
  objectMeshData_.clear();
}

bool
Canvas3D::
addObjectMeshData(CGeomObject3D *object, CMatrix3DH &meshMatrix)
{
  auto pm = objectMeshData_.find(object);

  if (pm != objectMeshData_.end()) {
    auto *animObject = object->getAnimObject();

    auto animTime = animObject->animTime();

    const auto &objectMeshData = (*pm).second;

    auto frame = int((animTime - objectMeshData.tmin)/objectMeshData.dt + 0.5);

    auto pf = objectMeshData.frameMatrix.find(frame);

    if (pf != objectMeshData.frameMatrix.end()) {
      meshMatrix = (*pf).second;

      return true;
    }
    else {
      std::cerr << "Bad meshMatrix anim time\n";
    }
  }

  return false;
}

Canvas3D::ObjectMeshData &
Canvas3D::
getObjectMeshData(CGeomObject3D *object)
{
  auto po = objectMeshData_.find(object);

  if (po == objectMeshData_.end())
    po = objectMeshData_.insert(po, ObjectMeshDataMap::value_type(object, ObjectMeshData()));

  return (*po).second;
}

//---

void
Canvas3D::
bindBuffer(CQGLBuffer *buffer)
{
  if (buffer) {
    if (buffer != currentBuffer_) {
      if (currentBuffer_)
        currentBuffer_->unbind();

      buffer->bind();

      currentBuffer_ = buffer;
    }
  }
  else {
    if (currentBuffer_)
      currentBuffer_->unbind();
  }
}

void
Canvas3D::
bindProgram(ShaderProgram *program)
{
  if (program) {
    if (program != currentProgram_) {
      if (currentProgram_)
        currentProgram_->release();

      program->bind();

      currentProgram_ = program;
    }
  }
  else {
    if (currentProgram_)
      currentProgram_->release();
  }
}

void
Canvas3D::
updateNodeMatrices(CGeomObject3D *object)
{
  // anim data on anim object
  auto *animObject = object->getAnimObject();

  if (animObject == paintData_.animObject && objectNodeMatricesValid_)
    return;

  paintData_.animObject = animObject;

  //---

  // get node matrices for anim name and anim time
  const auto &nodeMatrices = getObjectNodeMatrices(animObject);

  //---

  std::vector<CMatrix3D> nodeMatrixArray;

  for (int i = 0; i < NUM_NODE_MATRICES; i++)
    nodeMatrixArray.push_back(CMatrix3D::identity());

  for (const auto &pn : nodeMatrices) {
    auto nodeId = pn.first;

    if (nodeId < 0) {
      std::cerr << "Invalid node id " << nodeId << "\n";
      continue;
    }

    if (nodeId >= NUM_NODE_MATRICES) {
      std::cerr << "Too few node matrices for node " << nodeId << "\n";
      continue;
    }

    nodeMatrixArray[nodeId] = pn.second;
  }

  paintData_.nodeMatrices .resize(NUM_NODE_MATRICES);
  paintData_.nodeQMatrices.resize(NUM_NODE_MATRICES);

  int im = 0;
  for (const auto &m : nodeMatrixArray) {
    paintData_.nodeMatrices [im] = m;
    paintData_.nodeQMatrices[im] = CQGLUtil::toQMatrix(m);

    ++im;
  }
}

const Canvas3D::NodeMatrices &
Canvas3D::
getObjectNodeMatrices(CGeomObject3D *object) const
{
  const auto &objectNodeMatrices = getNodeMatrices();

  auto po = objectNodeMatrices.find(object->getInd());
  assert(po != objectNodeMatrices.end());

  return (*po).second;
}

const Canvas3D::ObjectNodeMatrices &
Canvas3D::
getNodeMatrices() const
{
  auto *th = const_cast<Canvas3D *>(this);

  if (! th->objectNodeMatricesValid_) {
    th->objectNodeMatrices_ = calcNodeMatrices();

    th->objectNodeMatricesValid_ = true;
  }

  return objectNodeMatrices_;
}

Canvas3D::ObjectNodeMatrices
Canvas3D::
calcNodeMatrices() const
{
  ObjectNodeMatrices objectNodeMatrices;

  auto animObjects = getAnimObjects();

  for (auto *animObject : animObjects) {
    if (! animObject->getVisible())
      continue;

    auto animName = animObject->animName();
    if (animName == "") continue;

    auto animTime = animObject->animTime();

    auto &nodeMatrices = objectNodeMatrices[animObject->getInd()];

    animObject->updateNodesAnimationData(animName, animTime);

    auto meshMatrix        = animObject->getMeshGlobalTransform();
    auto inverseMeshMatrix = meshMatrix.inverse();

    //---

    for (const auto &pn : animObject->getNodes()) {
      auto &node = const_cast<CGeomNodeData &>(pn.second);
      //if (! node.isJoint()) continue;

      if (node.index() < 0)
        continue;

      nodeMatrices[node.index()] = node.calcNodeAnimMatrix(inverseMeshMatrix);
    }
  }

  return objectNodeMatrices;
}

QMatrix4x4 *
Canvas3D::
nodeQMatrices() const
{
  return const_cast<QMatrix4x4 *>(&paintData_.nodeQMatrices[0]);
}

std::vector<CGeomObject3D *>
Canvas3D::
getAnimObjects() const
{
  std::set<CGeomObject3D *>    animObjectSet;
  std::vector<CGeomObject3D *> animObjects;

  const auto &objects = scene_->getObjects();

  for (auto *object : objects) {
    auto *animObject = object->getAnimObject();
    if (! animObject) continue;

    if (animObjectSet.find(animObject) == animObjectSet.end()) {
      animObjectSet.insert(animObject);

      animObjects.push_back(animObject);
    }
  }

  return animObjects;
}

CPoint3D
Canvas3D::
adjustAnimPoint(const CGeomVertex3D &vertex, const CPoint3D &p,
                const NodeMatrices &nodeMatrices) const
{
  const auto &jointData = vertex.getJointData();

  if (! jointData.set)
    return p;

  struct NodeWeight {
    int    nodeId { -1 };
    double weight { 0.0 };
  };

  std::vector<NodeWeight> nodeWeights;

  double total = 0.0;

  for (int i = 0; i < 4; ++i) {
    if (jointData.nodeDatas[i].node >= 0 && jointData.nodeDatas[i].weight > 0.0) {
      NodeWeight nodeWeight;

      nodeWeight.nodeId = jointData.nodeDatas[i].node;
      nodeWeight.weight = jointData.nodeDatas[i].weight;

      nodeWeights.push_back(nodeWeight);

      total += nodeWeight.weight;
    }
  }

  auto f = (total > 0.0 ? 1.0/total : 1.0);

  if (! nodeWeights.empty()) {
    for (auto &nodeWeight : nodeWeights)
      nodeWeight.weight *= f;

    auto p1 = CPoint3D(0, 0, 0);

    for (const auto &nodeWeight : nodeWeights) {
      CMatrix3D boneTransform;

      if (getNodeMatrix(nodeMatrices, nodeWeight.nodeId, boneTransform))
        p1 += nodeWeight.weight*(boneTransform*p);
      else
        p1 += nodeWeight.weight*p;
    }

    return p1;
  }
  else
    return p;
}

bool
Canvas3D::
getNodeMatrix(const NodeMatrices &nodeMatrices, int nodeId, CMatrix3D &m) const
{
  auto pm = nodeMatrices.find(nodeId);
  if (pm == nodeMatrices.end()) return false;

  m = (*pm).second;

  return true;
}

//---

void
Canvas3D::
mousePressEvent(QMouseEvent *e)
{
  mouseData_.pressed = true;
  mouseData_.button  = e->button();
  mouseData_.press   = CPoint2D(e->x(), e->y());
  mouseData_.move1   = mouseData_.press;
  mouseData_.move2   = mouseData_.move1;

  mouseData_.isShift   = (e->modifiers() & Qt::ShiftModifier);
  mouseData_.isControl = (e->modifiers() & Qt::ControlModifier);

  //---

  setMousePos(mouseData_.press.x, mouseData_.press.y);

#if 0
  auto type = this->type();

  if (type == Type::CAMERA)
    camera_->setLastPos(mouseData_.press.x, mouseData_.press.y);
#endif

  update();
}

void
Canvas3D::
mouseMoveEvent(QMouseEvent *e)
{
  mouseData_.move2 = CPoint2D(e->x(), e->y());

  mouseData_.isShift   = (e->modifiers() & Qt::ShiftModifier);
  mouseData_.isControl = (e->modifiers() & Qt::ControlModifier);

  //---

  setMousePos(mouseData_.move2.x, mouseData_.move2.y);

  if (mouseData_.pressed) {
    auto type = this->type();

    if (type == Type::CAMERA) {
#if 0
      double xoffset, yoffset;

      camera_->deltaPos(mouseData_.move2.x, mouseData_.move2.y, xoffset, yoffset);

      camera_->setLastPos(mouseData_.move2.x, mouseData_.move2.y);

      camera_->processMouseMovement(xoffset, yoffset);
#else
      auto dx = CMathUtil::sign(mouseData_.move2.x - mouseData_.move1.x);
      auto dy = CMathUtil::sign(mouseData_.move2.y - mouseData_.move1.y);

      if      (mouseData_.button == Qt::MiddleButton) {
        auto da = M_PI/180.0;

        camera_->rotateY(-dx*da);
        camera_->rotateX(-dy*da);
      }
      else if (mouseData_.button == Qt::RightButton) {
        camera_->moveRight(-dx/100.0);
        camera_->moveUp   ( dy/100.0);
      }
#endif
    }

    update();
  }

  mouseData_.move1 = mouseData_.move2;
}

void
Canvas3D::
mouseReleaseEvent(QMouseEvent *e)
{
  mouseData_.move2.x = e->x();
  mouseData_.move2.y = e->y();

  //---

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

  if (isEyeLineVisible()) {
    eyeLine_           ->setVisible(false);
    intersectParticles_->setVisible(false);
  }

  update();

  if (clickObjs.size() == 1) {
    auto *clickObj = clickObjs[0];

    for (auto *obj : objects())
      obj->setSelected(obj == clickObj);

    app_->runTclCmd(QString("click {%1}").arg(clickObj->id()));
  }

  //---

  mouseData_.pressed = false;
  mouseData_.button  = Qt::NoButton;
}

//---

void
Canvas3D::
deselectAll()
{
  for (auto *obj : objects()) {
    obj->setSelected(false);

    auto *model3DObj = dynamic_cast<Model3DObj *>(obj);

    if (model3DObj)
      model3DObj->object()->setSelected(false);
  }
}

void
Canvas3D::
selectObjects(const std::vector<Object3D *> &objs, bool clear, bool update)
{
  if (clear)
    deselectAll();

  for (auto *obj : objs) {
    auto *model3DObj = dynamic_cast<Model3DObj *>(obj);

    obj->setSelected(true);

    if (model3DObj)
      model3DObj->object()->setSelected(true);
  }

  if (update)
    this->update();
}

void
Canvas3D::
selectObject(Object3D *obj, bool clear, bool update)
{
  if (clear)
    deselectAll();

  auto *model3DObj = dynamic_cast<Model3DObj *>(obj);

  obj->setSelected(true);

  if (model3DObj)
    model3DObj->object()->setSelected(true);

  if (update)
    this->update();
}

void
Canvas3D::
selectFace(CGeomFace3D *face, bool clear, bool update)
{
  if (clear)
    deselectAll();

  face->setSelected(true);

  if (update)
    this->update();
}

//---

void
Canvas3D::
setMousePos(double xpos, double ypos)
{
  // unobserve
  auto x1 = CMathUtil::map(xpos, 0, pixelWidth_  - 1, -1,  1);
  auto y1 = CMathUtil::map(ypos, 0, pixelHeight_ - 1,  1, -1);
  auto z1 = 0.1;

  auto x2 = x1;
  auto y2 = y1;
  auto z2 = 10.0;

  auto imatrix1 = projectionMatrix_.inverse();

  double xp1, yp1, zp1;
  imatrix1.multiplyPoint(x1, y1, z1, &xp1, &yp1, &zp1);
  double xp2, yp2, zp2;
  imatrix1.multiplyPoint(x2, y2, z2, &xp2, &yp2, &zp2);

  const auto &viewMatrix = camera_->viewMatrix();
  auto imatrix2 = viewMatrix.inverse();

  double xv1, yv1, zv1;
  imatrix2.multiplyPoint(xp1, yp1, zp1, &xv1, &yv1, &zv1);

  double xv2, yv2, zv2;
  imatrix2.multiplyPoint(xp2, yp2, zp2, &xv2, &yv2, &zv2);

  app_->canvasToolbar3D()->setPos(QString("%1 %2 %3").arg(xv1).arg(yv1).arg(zv1));

  //---

  if (isEyeLineVisible()) {
    CVector3D pe1(xv1, yv1, zv1);
    CVector3D pe2(xv2, yv2, zv2);

    eyeLine_->setLine(pe1, pe2);
    eyeLine_->setVisible(true);
  }

  //---

  intersectPoints_.clear();

  for (auto *obj : objects_) {
    if (! obj->isVisible())
      continue;

    auto imodelMatrix = obj->modelMatrix().inverse();

    double mx1, my1, mz1;
    imodelMatrix.multiplyPoint(xv1, yv1, zv1, &mx1, &my1, &mz1);

    double mx2, my2, mz2;
    imodelMatrix.multiplyPoint(xv2, yv2, zv2, &mx2, &my2, &mz2);

    CVector3D pm1(mx1, my1, mz1);
    CVector3D pm2(mx2, my2, mz2);

    CPoint3D pi1, pi2;

    bool inside = obj->intersect(pm1, pm2, pi1, pi2);

    if (inside != obj->isInside()) {
      obj->setInside(inside);

      obj->setNeedsUpdate();
    }

    if (inside) {
      auto mapPoint = [&](const CPoint3D &p) {
        double x1, y1, z1;
        obj->modelMatrix().multiplyPoint(p.x, p.y, p.z, &x1, &y1, &z1);
        return CVector3D(x1, y1, z1);
      };

      intersectPoints_.push_back(mapPoint(pi1));

      if (pi2 != pi1)
        intersectPoints_.push_back(mapPoint(pi2));
    }
  }

  if (isEyeLineVisible()) {
    std::vector<CGLVector3D> ppoints;

    for (const auto &ip : intersectPoints_)
      ppoints.push_back(CGLVector3D(ip.getX(), ip.getY(), ip.getZ()));

    intersectParticles_->setPoints(ppoints);
    intersectParticles_->setVisible(true);
  }
}

void
Canvas3D::
wheelEvent(QWheelEvent *e)
{
  auto dw = e->angleDelta().y()/250.0;

#if 0
  auto type = this->type();

  if (type == Type::CAMERA)
    camera_->processMouseScroll(dw);
#else
  auto d = bbox_.getMaxSize()/100.0;

  camera_->setDistance(camera_->distance() - dw*d);
#endif

  update();
}

void
Canvas3D::
keyPressEvent(QKeyEvent *e)
{
  auto keyStr = getKeyString(e);

  keyPressed_[keyStr] = true;

  mouseData_.isControl = (e->modifiers() & Qt::ControlModifier);
  mouseData_.isShift   = (e->modifiers() & Qt::ShiftModifier);

  auto k = e->key();

  //---

  auto bbox = this->bbox();

  auto d  = bbox.getMaxSize()/100.0;
  auto da = M_PI/180.0;

  //---

  auto type = this->type();

  if (k == Qt::Key_Escape) {
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

    if      (k == Qt::Key_Left ) text = "left";
    else if (k == Qt::Key_Right) text = "right";
    else if (k == Qt::Key_Up   ) text = "up";
    else if (k == Qt::Key_Down ) text = "down";
    else if (k == Qt::Key_Space) text = "space";
    else                                text = e->text();

    if (text == "")
      text = QString("key.%1").arg(k);

    app_->runTclCmd(QString("keyPress {%1}").arg(text));

    update();

    return;
  }

  if (type == Type::CAMERA) {
    if      (k == Qt::Key_Left) {
      camera_->moveRight(-d);
    }
    else if (k == Qt::Key_Right) {
      camera_->moveRight(d);
    }
    else if (k == Qt::Key_Up) {
      camera_->moveUp(d);
    }
    else if (k == Qt::Key_Down) {
      camera_->moveUp(-d);
    }
    else if (k == Qt::Key_Plus) {
      camera_->moveFront(d);
    }
    else if (k == Qt::Key_Minus) {
      camera_->moveFront(-d);
    }
    else if (k == Qt::Key_W) {
      camera_->rotateX(da);
    }
    else if (k == Qt::Key_S) {
      camera_->rotateX(-da);
    }
    else if (k == Qt::Key_A) {
      camera_->rotateY(da);
    }
    else if (k == Qt::Key_D) {
      camera_->rotateY(-da);
    }
    else if (k == Qt::Key_Q) {
      camera_->rotateZ(-da);
    }
    else if (k == Qt::Key_E) {
      camera_->rotateZ(da);
    }
    else if (k == Qt::Key_Space) {
      camera_->printMatrices();
    }
  }

  if (type == Type::LIGHT) {
    auto *light = currentLight();

    if      (k == Qt::Key_W) {
      light->setPosition(light->getPosition() + CPoint3D(0.0, 0.1, 0.0));
    }
    else if (k == Qt::Key_S) {
      light->setPosition(light->getPosition() - CPoint3D(0.0, 0.1, 0.0));
    }
    else if (k == Qt::Key_A) {
      light->setPosition(light->getPosition() - CPoint3D(0.1, 0.0, 0.0));
    }
    else if (k == Qt::Key_D) {
      light->setPosition(light->getPosition() + CPoint3D(0.1, 0.0, 0.0));
    }
    else if (k == Qt::Key_L) {
      setLightNum(lightNum() + 1);
    }
    else if (k == Qt::Key_Up) {
      light->setPosition(light->getPosition() + CPoint3D(0.0, 0.0, 0.1));
    }
    else if (k == Qt::Key_Down) {
      light->setPosition(light->getPosition() - CPoint3D(0.0, 0.0, 0.1));
    }
  }

  if (type == Type::MODEL) {
    auto *obj = getCurrentObject();
    if (! obj) return;

    if      (k == Qt::Key_X) {
      obj->setXAngle(obj->xAngle() + da);
    }
    else if (k == Qt::Key_Y) {
      obj->setYAngle(obj->yAngle() + da);
    }
    else if (k == Qt::Key_Z) {
      obj->setZAngle(obj->zAngle() + da);
    }
  }

  update();
}

void
Canvas3D::
keyReleaseEvent(QKeyEvent *e)
{
  auto keyStr = getKeyString(e);

  keyPressed_[keyStr] = false;
}

bool
Canvas3D::
getKeyPressed(const std::string &key) const
{
  auto p = keyPressed_.find(key);

  if (p == keyPressed_.end())
    return false;

  return (*p).second;
}

std::string
Canvas3D::
getKeyString(QKeyEvent *e) const
{
  std::string keyStr;

  if      (e->key() == Qt::Key_Left ) keyStr = "left";
  else if (e->key() == Qt::Key_Right) keyStr = "right";
  else if (e->key() == Qt::Key_Up   ) keyStr = "up";
  else if (e->key() == Qt::Key_Down ) keyStr = "down";
  else if (e->key() == Qt::Key_Space) keyStr = "space";
  else                                keyStr = e->text().toStdString();

  if (keyStr == "")
    keyStr = "key." + std::to_string(e->key());

  return keyStr;
}

//---

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
