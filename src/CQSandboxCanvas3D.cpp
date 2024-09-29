#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>
#include <CQGLUtil.h>
#include <CQGLBuffer.h>
#include <CQGLTexture.h>
#include <CQGLCubemap.h>

#include <CQCsvModel.h>

#include <CImportBase.h>
#include <CGeomScene3D.h>
#include <CGeomTexture.h>
#include <CLorenzCalc.h>
#include <CForceDirected3D.h>
#include <CDotParse.h>
#include <CInterval.h>

#ifdef CQSANDBOX_OTHELLO
#include <COthello.h>
#endif

#ifdef CQSANDBOX_FIELD_RUNNERS
#include <CFieldRunners.h>
#endif

#include <CGLTexture.h>
#include <CGLCamera.h>

#ifdef CQSANDBOX_WATER_SURFACE
#include <CWaterSurface.h>
#endif

#ifdef CQSANDBOX_FLAG
#include <CFlag.h>
#endif

#ifdef CQSANDBOX_FLOCKING
#include <CFlocking.h>
#endif

#ifdef CQSANDBOX_FIREWORKS
#include <CFireworks.h>
#endif

#include <CSphere3D.h>
#include <CCone3D.h>
#include <CCylinder3D.h>
#include <CCenteredBox3D.h>

#include <CFile.h>
#include <CMinMax.h>

#include <QFileInfo>
#include <QMouseEvent>
#include <QTimer>
#include <QPainter>

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

CPoint2D
stringToPoint2D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CPoint2D p;

  if (strs.size() >= 2) {
    bool ok;
    auto x = strs[0].toDouble(&ok);
    auto y = strs[1].toDouble(&ok);

    p.x = x;
    p.y = y;
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
  auto astr = QString::number(c.a);

  return rstr + " " + gstr + " " + bstr + " " + astr;
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
    auto a = 1.0;

    if (strs.size() >= 4)
      a = strs[3].toDouble(&ok);

    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
  }

  return c;
}

std::vector<Color>
stringToColors(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  std::vector<Color> colors;

  for (const auto &str : strs) {
    auto c = stringToColor(tcl, str);

    colors.push_back(c);
  }

  return colors;
}

CGLVector3D
pointToVector(const CPoint3D &p)
{
  return CGLVector3D(p.x, p.y, p.z);
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

  timer_ = new QTimer;

  connect(timer_, &QTimer::timeout, this, &Canvas3D::timerSlot);

  timer_->start(redrawTimeOut_);
}

void
Canvas3D::
setType(const Type &type)
{
  if (type != type_) {
    type_ = type;

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

  tcl->createObjCommand("sb3d::cube",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Cube3DObj>),
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

  if      (name == "bg")
    setBgColor(Util::stringToColor(value));
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
  else if (name == "position")
    return vector3DToString(camera()->position());
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
  else if (name == "position")
    camera()->setPosition(stringToVector3D(tcl, value));
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

  //---

  intersectParticles_ = new ParticleList3DObj(this);

  (void) addNewObject(intersectParticles_);

  intersectParticles_->init();
  intersectParticles_->setVisible(false);

  //---

  app_->runTclCmd("init");
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
  auto numLights = numDirectionalLights_ + numPointLights_ + numSpotLights_;
  if (numLights == lights_.size()) return;

  while (lights_.size() < numLights)
    lights_.push_back(new Light3D(this, Light3D::Type::DIRECTIONAL));

  int il = 0;

  lights_[il]->setType     (Light3D::Type::DIRECTIONAL);
  lights_[il]->setEnabled  (true);
  lights_[il]->setDirection(CGLVector3D(1, -1, -1));
  lights_[il]->setColor    (CGLVector3D(1, 1, 0));
//lights_[il]->setIntensity(1.0f);

  ++il;

  lights_[il]->setType     (Light3D::Type::POINT);
  lights_[il]->setEnabled  (true);
  lights_[il]->setPosition (CGLVector3D(-0.5, 0.35, 0.25));
  lights_[il]->setColor    (CGLVector3D(1, 0, 0));
  lights_[il]->setRadius   (1.0f);
//lights_[il]->setIntensity(1.0f);

  ++il;

  lights_[il]->setType     (Light3D::Type::POINT);
  lights_[il]->setEnabled  (true);
  lights_[il]->setPosition (CGLVector3D(0.5, 0.35, 0.25));
  lights_[il]->setColor    (CGLVector3D(0, 1, 0));
  lights_[il]->setRadius   (1.0f);
//lights_[il]->setIntensity(1.0f);

  ++il;

  auto cpos = camera()->position();

  lights_[il]->setType     (Light3D::Type::SPOT);
  lights_[il]->setEnabled  (true);
  lights_[il]->setPosition (cpos + CGLVector3D(0.0, 0.5, 0));
  lights_[il]->setDirection(CGLVector3D(0, 0, -1));
  lights_[il]->setColor    (CGLVector3D(0, 0, 1));
//lights_[il]->setIntensity(1.0);
  lights_[il]->setCutoff   (cos(35.0f));

  ++il;

  lights_[il]->setType     (Light3D::Type::SPOT);
  lights_[il]->setEnabled  (true);
  lights_[il]->setPosition (cpos + CGLVector3D(0.0, -0.5, 0));
  lights_[il]->setDirection(CGLVector3D(0, 0, -1));
  lights_[il]->setColor    (CGLVector3D(0, 1, 1));
//lights_[il]->setIntensity(1.5);
  lights_[il]->setCutoff   (cos(15.0f));

  //++il;
}

void
Canvas3D::
setProgramLights(QOpenGLShaderProgram *program)
{
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
  updateLights();

  //---

  const qreal retinaScale = devicePixelRatio();

  pixelWidth_  = width ()*retinaScale;
  pixelHeight_ = height()*retinaScale;

  glViewport(0, 0, pixelWidth_, pixelHeight_);

  glClearColor(bgColor_.redF(), bgColor_.greenF(), bgColor_.blueF(), 1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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

  aspect_ = float(width())/float(height());

  projectionMatrix_ = camera_->getPerspectiveMatrix(aspect_);
  viewMatrix_       = camera_->getViewMatrix();

  //---

  viewPos_ = camera_->position();

  //---

  for (auto *obj : objects_) {
    if (obj && obj->isVisible())
      obj->render();
  }

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

  if (clickObjs.size() == 1)
    app_->runTclCmd(QString("click {%1}").arg(clickObjs[0]->id()));
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

  app_->control3D()->setPos(QString("%1 %2 %3").arg(xv1).arg(yv1).arg(zv1));

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

  setNeedsUpdate();
}

void
Object3D::
init()
{
  modelMatrix_ = CGLMatrix3D::identity();
}

void
Object3D::
setModelMatrix(uint matrixFlags)
{
  modelMatrix_ = CGLMatrix3D::identity();

  if (matrixFlags & ModelMatrixFlags::TRANSLATE)
    modelMatrix_.translated(float(position().getX()),
                            float(position().getY()),
                            float(position().getZ()));

  if (matrixFlags & ModelMatrixFlags::SCALE)
    modelMatrix_.scaled(xscale(), yscale(), zscale());

  if (matrixFlags & ModelMatrixFlags::ROTATE) {
    modelMatrix_.rotated(xAngle(), CGLVector3D(1.0, 0.0, 0.0));
    modelMatrix_.rotated(yAngle(), CGLVector3D(0.0, 1.0, 0.0));
    modelMatrix_.rotated(zAngle(), CGLVector3D(0.0, 0.0, 1.0));
  }
}

QVariant
Object3D::
getValue(const QString &name, const QStringList &)
{
  auto *app = canvas()->app();

  if      (name == "id")
    return id();
  else if (name == "visible")
    return QString(isVisible() ? "1" : "0");
  else if (name == "position")
    return Util::point3DToString(position());
  else if (name == "x_angle")
    return Util::realToString(xAngle());
  else if (name == "y_angle")
    return Util::realToString(yAngle());
  else if (name == "z_angle")
    return Util::realToString(zAngle());
  else if (name == "group")
    return (group() ? group()->calcId() : "");
  else {
    app->errorMsg(QString("Invalid get name '%1'").arg(name));
    return QVariant();
  }
}

bool
Object3D::
setValue(const QString &name, const QString &value, const QStringList &)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  if      (name == "id")
    setId(value);
  else if (name == "visible") {
    setVisible(Util::stringToBool(value));

    setNeedsUpdate();
  }
  else if (name == "position") {
    setPosition(stringToPoint3D(tcl, value));

    setNeedsUpdate();
  }
  else if (name == "x_angle") {
    setXAngle(Util::stringToReal(value));

    setNeedsUpdate();
  }
  else if (name == "y_angle") {
    setYAngle(Util::stringToReal(value));

    setNeedsUpdate();
  }
  else if (name == "z_angle") {
    setZAngle(Util::stringToReal(value));

    setNeedsUpdate();
  }
  else if (name == "scale") {
    setScale(Util::stringToReal(value));

    setNeedsUpdate();
  }
  else if (name == "group") {
    auto *group = dynamic_cast<Group3DObj *>(canvas()->getObjectByName(value));

    if (group != group_) {
      if (group_)
        group_->removeObject(this);
      else
        canvas()->removeObject(this);

      if (group)
        group->addObject(this);
      else
        canvas()->addObject(this);
    }
  }
  else {
    app->errorMsg(QString("Invalid set name '%1'").arg(name));
    return false;
  }

  return true;
}

void
Object3D::
tick()
{
  auto *app = canvas()->app();

  ticks_ += dt_;

  app->runTclCmd("tick");
}

void
Object3D::
render()
{
}

//---

QOpenGLShaderProgram* Light3D::s_shaderProgram;

Light3D::
Light3D(Canvas3D *canvas, const Type &type) :
 canvas_(canvas), type_(type)
{
}

void
Light3D::
initBuffer()
{
  initShader();

  // set up vertex data (and buffer(s)) and configure vertex attributes
  buffer_ = new CQGLBuffer(s_shaderProgram);

  buffer_->addPoint(-0.5f, -0.5f, -0.5f);
  buffer_->addPoint( 0.5f, -0.5f, -0.5f);
  buffer_->addPoint( 0.5f,  0.5f, -0.5f);

  buffer_->addPoint( 0.5f,  0.5f, -0.5f);
  buffer_->addPoint(-0.5f,  0.5f, -0.5f);
  buffer_->addPoint(-0.5f, -0.5f, -0.5f);

  buffer_->addPoint(-0.5f, -0.5f,  0.5f);
  buffer_->addPoint( 0.5f, -0.5f,  0.5f);
  buffer_->addPoint( 0.5f,  0.5f,  0.5f);

  buffer_->addPoint( 0.5f,  0.5f,  0.5f);
  buffer_->addPoint(-0.5f,  0.5f,  0.5f);
  buffer_->addPoint(-0.5f, -0.5f,  0.5f);

  buffer_->addPoint(-0.5f,  0.5f,  0.5f);
  buffer_->addPoint(-0.5f,  0.5f, -0.5f);
  buffer_->addPoint(-0.5f, -0.5f, -0.5f);

  buffer_->addPoint(-0.5f, -0.5f, -0.5f);
  buffer_->addPoint(-0.5f, -0.5f,  0.5f);
  buffer_->addPoint(-0.5f,  0.5f,  0.5f);

  buffer_->addPoint( 0.5f,  0.5f,  0.5f);
  buffer_->addPoint( 0.5f,  0.5f, -0.5f);
  buffer_->addPoint( 0.5f, -0.5f, -0.5f);

  buffer_->addPoint( 0.5f, -0.5f, -0.5f);
  buffer_->addPoint( 0.5f, -0.5f,  0.5f);
  buffer_->addPoint( 0.5f,  0.5f,  0.5f);

  buffer_->addPoint(-0.5f, -0.5f, -0.5f);
  buffer_->addPoint( 0.5f, -0.5f, -0.5f);
  buffer_->addPoint( 0.5f, -0.5f,  0.5f);

  buffer_->addPoint( 0.5f, -0.5f,  0.5f);
  buffer_->addPoint(-0.5f, -0.5f,  0.5f);
  buffer_->addPoint(-0.5f, -0.5f, -0.5f);

  buffer_->addPoint(-0.5f,  0.5f, -0.5f);
  buffer_->addPoint( 0.5f,  0.5f, -0.5f);
  buffer_->addPoint( 0.5f,  0.5f,  0.5f);

  buffer_->addPoint( 0.5f,  0.5f,  0.5f);
  buffer_->addPoint(-0.5f,  0.5f,  0.5f);
  buffer_->addPoint(-0.5f,  0.5f, -0.5f);

  buffer_->load();
}

void
Light3D::
initShader()
{
  if (! s_shaderProgram) {
    QString buildDir = QUOTE(BUILD_DIR);

    s_shaderProgram = new QOpenGLShaderProgram;

    if (! s_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                   buildDir + "/shaders/light.vs"))
      std::cerr << s_shaderProgram->log().toStdString() << "\n";

    if (! s_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                                   buildDir + "/shaders/light.fs"))
      std::cerr << s_shaderProgram->log().toStdString() << "\n";

    s_shaderProgram->link();
  }
}

void
Light3D::
render()
{
  initBuffer();

  // setup light shader
  buffer_->bind();

  s_shaderProgram->bind();

  s_shaderProgram->setUniformValue("projection",
    CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_shaderProgram->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  auto lightMatrix = CGLMatrix3D::translation(position());
  lightMatrix.scaled(0.01f, 0.01f, 0.01f);
  s_shaderProgram->setUniformValue("model", CQGLUtil::toQMatrix(lightMatrix));

  // draw light
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  //buffer_->drawTriangles();

  buffer_->unbind();
}

//---

QOpenGLShaderProgram* Model3DObj::s_modelShaderProgram;

bool
Model3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Model3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  if (args.size() >= 1) {
    auto filename = args[0];

    obj->load(filename);
  }

  tcl->setResult(name);

  return true;
}

Model3DObj::
Model3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
  QString buildDir = QUOTE(BUILD_DIR);

  vertShaderFile_ = buildDir + "/shaders/model.vs";
  fragShaderFile_ = buildDir + "/shaders/model.fs";
}

void
Model3DObj::
initShader()
{
  if (s_modelShaderProgram)
    return;

  s_modelShaderProgram = new QOpenGLShaderProgram;

  if (! s_modelShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, vertShaderFile_))
    std::cerr << s_modelShaderProgram->log().toStdString() << "\n";

  if (! s_modelShaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, fragShaderFile_))
    std::cerr << s_modelShaderProgram->log().toStdString() << "\n";

  s_modelShaderProgram->link();
}

QVariant
Model3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Model3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto fileToTexture = [&](const QString filename, bool flipY=true) {
    CFile imageFile(filename.toStdString());

    if (! imageFile.exists())
      return static_cast<CGLTexture *>(nullptr);

    CImageFileSrc src(imageFile);

    auto image = CImageMgrInst->createImage(src);

    if (flipY)
      image = image->flippedH();

    return new CGLTexture(image);
  };

  auto resetShader = [&]() {
    delete s_modelShaderProgram;

    s_modelShaderProgram = nullptr;
  };

  //---

  if      (name == "diffuse_texture") {
    diffuseTexture_ = fileToTexture(value);

    needsUpdate_ = true;
  }
  else if (name == "specular_texture") {
    specularTexture_ = fileToTexture(value);

    needsUpdate_ = true;
  }
  else if (name == "normal_texture") {
    normalTexture_ = fileToTexture(value);

    needsUpdate_ = true;
  }
  else if (name == "vert_shader") {
    vertShaderFile_ = value;

    resetShader();
  }
  else if (name == "frag_shader") {
    fragShaderFile_ = value;

    resetShader();
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

bool
Model3DObj::
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

  needsUpdate_ = true;

  return true;
}

void
Model3DObj::
tick()
{
  Object3D::tick();

  canvas_->update();
}

void
Model3DObj::
render()
{
  initShader();

  updateObjectData();

  //---

  // lighting
  auto *light = canvas_->currentLight();

  auto lightPos   = light->position();
  auto lightColor = light->color();

  //---

  modelMatrix_ = CGLMatrix3D::identity();
//TODO: ???
//modelMatrix_.translated(float(sceneCenter_.getX()),
//                        float(sceneCenter_.getY()),
//                        float(sceneCenter_.getZ()));
  modelMatrix_.scaled(xscale(), yscale(), zscale());
  modelMatrix_.rotated(xAngle(), CGLVector3D(1.0, 0.0, 0.0));
  modelMatrix_.rotated(yAngle(), CGLVector3D(0.0, 1.0, 0.0));
  modelMatrix_.rotated(zAngle(), CGLVector3D(0.0, 0.0, 1.0));
  modelMatrix_.translated(float(-sceneCenter_.getX()),
                          float(-sceneCenter_.getY()),
                          float(-sceneCenter_.getZ()));

  auto t = 1.0*ticks_/100.0;

  if (t >= 1.0)
    dt_ = -dt_;

  auto *program = s_modelShaderProgram;

  // setup model shader
  for (auto &po : objectDatas_) {
    auto *objectData = po.second;

    objectData->buffer->bind();

    program->bind();

    program->setUniformValue("ticks", float(t));

    canvas_->setProgramLights(program);

    program->setUniformValue("viewPos", CQGLUtil::toVector(canvas_->viewPos()));

    program->setUniformValue("ambientStrength" , float(canvas_->ambient()));
    program->setUniformValue("diffuseStrength" , float(canvas_->diffuse()));
    program->setUniformValue("specularStrength", float(canvas_->specular()));
    program->setUniformValue("shininess"       , float(canvas_->shininess()));

    // pass projection matrix to shader (note that in this case it could change every frame)
    program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));

    // camera/view transformation
    program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

    // model rotation
    program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

    // render model
    for (const auto &faceData : objectData->faceDatas) {
      // diffuse (texture 0)
      auto *diffuseTexture = faceData.diffuseTexture;

      if (! diffuseTexture)
        diffuseTexture = diffuseTexture_;

      bool useDiffuseTexture = !!diffuseTexture;

      program->setUniformValue("diffuseTexture.enabled", useDiffuseTexture);

      if (useDiffuseTexture) {
        glActiveTexture(GL_TEXTURE0);
        diffuseTexture->bind();

        program->setUniformValue("diffuseTexture.texture", 0);
      }

      //---

      // specular (texture 1)
      auto *specularTexture = faceData.specularTexture;

      if (! specularTexture)
        specularTexture = specularTexture_;

      bool useSpecularTexture = !!specularTexture;

      program->setUniformValue("specularTexture.enabled", useSpecularTexture);

      if (useSpecularTexture) {
        glActiveTexture(GL_TEXTURE1);
        specularTexture->bind();

        program->setUniformValue("specularTexture.texture", 1);
      }

      //---

      // normal (texture 2)
      auto *normalTexture = faceData.normalTexture;

      if (! normalTexture)
        normalTexture = normalTexture_;

      bool useNormalTexture = !!normalTexture;

      program->setUniformValue("normalTexture.enabled", useNormalTexture);

      if (useNormalTexture) {
        glActiveTexture(GL_TEXTURE2);
        normalTexture->bind();

        program->setUniformValue("normalTexture.texture", 2);
      }

      //---

      program->setUniformValue("isWireframe", canvas_->isWireframe() ? 1 : 0);

      if (! canvas_->isWireframe()) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glDrawArrays(GL_TRIANGLE_FAN, faceData.pos, faceData.len);
      }

      if (canvas_->isPolygonLine() || canvas_->isWireframe()) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glDrawArrays(GL_TRIANGLE_FAN, faceData.pos, faceData.len);
        //glDrawArrays(GL_TRIANGLES, faceData.pos, faceData.len);
      }
    }

    objectData->buffer->unbind();
  }
}

void
Model3DObj::
updateObjectData()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

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

      buffer->clearAll();

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

          auto vnormal = vertex.getNormal(normal);

          if (! flipYZ_) {
            buffer->addPoint(float(model.x), float(model.y), float(model.z));
            buffer->addNormal(float(vnormal.getX()), float(vnormal.getY()), float(vnormal.getZ()));
          }
          else {
            buffer->addPoint(float(model.x), float(model.z), float(model.y));
            buffer->addNormal(float(vnormal.getX()), float(vnormal.getZ()), float(vnormal.getY()));
          }

          auto vcolor = vertex.getColor(color);

          buffer->addColor(vcolor.getRedF(), vcolor.getGreenF(), vcolor.getBlueF());

          auto *diffuseTexture  = faceData.diffuseTexture;
          auto *specularTexture = faceData.specularTexture;
          auto *normalTexture   = faceData.normalTexture;

          if (! diffuseTexture ) diffuseTexture  = diffuseTexture_;
          if (! specularTexture) specularTexture = specularTexture_;
          if (! normalTexture  ) normalTexture   = normalTexture_;

          if (diffuseTexture || specularTexture || normalTexture) {
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

  auto sceneScale = float(1.0/max3(sceneSize.getX(), sceneSize.getY(), sceneSize.getZ()));
  //std::cerr << "Scene Scale : " << sceneScale << "\n";

  xscale_ = sceneScale;
  yscale_ = sceneScale;
  zscale_ = sceneScale;
}

void
Model3DObj::
calcTangents()
{
  if (! import_)
    return;

  auto &scene = import_->getScene();

  for (auto *object : scene.getObjects()) {
    std::vector<CVector3D> tangents;

    auto nv = object->getNumVertices();

    tangents.resize(nv);

    const auto &faces = object->getFaces();

    for (const auto *face : faces) {
      const auto &vertices = face->getVertices();
      if (vertices.size() < 3) continue;

      const auto &v0 = object->getVertex(vertices[0]);
      const auto &v1 = object->getVertex(vertices[1]);
      const auto &v2 = object->getVertex(vertices[2]);

      auto uv0 = v0.getTextureMap();
      auto uv1 = v1.getTextureMap();
      auto uv2 = v2.getTextureMap();

      auto edge1 = v1.getViewed() - v0.getViewed();
      auto edge2 = v2.getViewed() - v0.getViewed();

      auto dUV1 = uv1 - uv0;
      auto dUV2 = uv2 - uv0;

      double f = 1.0/(dUV1.x*dUV2.y - dUV2.x*dUV1.y);

      auto tx = f*(dUV2.y*edge1.x - dUV1.y*edge2.x);
      auto ty = f*(dUV2.y*edge1.y - dUV1.y*edge2.y);
      auto tz = f*(dUV2.y*edge1.z - dUV1.y*edge2.z);

      auto tan = CVector3D(tx, ty, tz);
      tan.normalize();

      tangents[vertices[0]] += tan;
      tangents[vertices[1]] += tan;
      tangents[vertices[2]] += tan;
    }

    for (uint i = 0; i < tangents.size(); ++i)
      tangents[i].normalize();
  }
}

//---

bool
Csv3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  if (args.size() != 1)
    return false;

  auto *tcl = canvas->app()->tcl();

  auto filename = args[0];

  auto *obj = new Csv3DObj(canvas, filename);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Csv3DObj::
Csv3DObj(Canvas3D *canvas, const QString &filename) :
 Object3D(canvas), filename_(filename)
{
  csv_ = new CQCsvModel;
}

void
Csv3DObj::
init()
{
  Object3D::init();
}

QVariant
Csv3DObj::
getValue(const QString &name, const QStringList &args)
{
  auto *app = canvas_->app();

  if      (name == "filename")
    return filename_;
  else if (name == "comment_header")
    return csv_->isCommentHeader();
  else if (name == "first_line_header")
    return csv_->isFirstLineHeader();
  else if (name == "first_column_header")
    return csv_->isFirstColumnHeader();
  else if (name == "num_rows")
    return csv_->rowCount();
  else if (name == "num_columns" || name == "num_cols")
    return csv_->columnCount();
  else if (name == "data") {
    if (args.size() == 2) {
      auto row = Util::stringToInt(args[0]);
      auto col = Util::stringToInt(args[1]);

      auto ind = csv_->index(row, col, QModelIndex());

      return csv_->data(ind);
    }
    else {
      app->errorMsg("missing row/col for data");
      return QVariant();
    }
  }
  else
    return Object3D::getValue(name, args);
}

bool
Csv3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "filename")
    filename_ = value;
  else if (name == "comment_header")
    csv_->setCommentHeader(Util::stringToBool(value));
  else if (name == "first_line_header")
    csv_->setFirstLineHeader(Util::stringToBool(value));
  else if (name == "first_column_header")
    csv_->setFirstColumnHeader(Util::stringToBool(value));
  else
    return Object3D::setValue(name, value, args);

  return true;
}

QVariant
Csv3DObj::
exec(const QString &op, const QStringList &args)
{
  if (op == "load") {
    if (! csv_->load(filename_))
      return QVariant(0);

    return QVariant(1);
  }
  else
    return Object3D::exec(op, args);
}

//---

bool
Group3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Group3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Group3DObj::
Group3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

void
Group3DObj::
init()
{
  Object3D::init();
}

QVariant
Group3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Group3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  return Object3D::setValue(name, value, args);
}

void
Group3DObj::
addObject(Object3D *obj)
{
  objects_.push_back(obj);

  obj->setGroup(this);

  Q_EMIT objectsChanged();
}

void
Group3DObj::
removeObject(Object3D *obj)
{
  Objects objects;

  for (auto *obj1 : objects_) {
    if (obj1 != obj)
      objects.push_back(obj1);
  }

  std::swap(objects, objects_);

  Q_EMIT objectsChanged();
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

bool
Shape3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

  if      (name == "points") {
    points_ = stringToVectors3D(tcl, value);

    setNeedsUpdate();
  }
  else if (name == "indices") {
    indices_ = stringToUIntArray(tcl, value);

    setNeedsUpdate();
  }
  else if (name == "colors") {
    colors_ = stringToColors(tcl, value);

    setNeedsUpdate();
  }
  else if (name == "tex_coords") {
    texCoords_ = stringToVectors2D(tcl, value);

    setNeedsUpdate();
  }
  else if (name == "color") {
    setColor(stringToColor(tcl, value));

    setNeedsUpdate();
  }
  else if (name == "texture") {
    setTexture(value);

    setNeedsUpdate();
  }
  else if (name == "normal_texture") {
    setNormalTexture(value);

    setNeedsUpdate();
  }
  else if (name == "angle") {
    auto p = stringToPoint3D(tcl, value);

    xAngle_ = p.getX();
    yAngle_ = p.getY();
    zAngle_ = p.getZ();

    setNeedsUpdate();
  }
  else if (name == "wireframe") {
    wireframe_ = Util::stringToBool(value);

    setNeedsUpdate();
  }
  else if (name == "cone") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() != 2) {
      app->errorMsg("Invalid dimensions for cone");
      return false;
    }

    double r = Util::stringToReal(strs[0]);
    double h = Util::stringToReal(strs[1]);

    addCone(r, h);

    delete geom_;
    geom_ = new CCone3D(r);

    setNeedsUpdate();
  }
  else if (name == "cylinder") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() != 2) {
      app->errorMsg("Invalid dimensions for cylinder");
      return false;
    }

    double r = Util::stringToReal(strs[0]);
    double h = Util::stringToReal(strs[1]);

    addCylinder(r, h);

    delete geom_;
    geom_ = new CCylinder3D(r);

    setNeedsUpdate();
  }
  else if (name == "sphere") {
    double r = Util::stringToReal(value);

    addSphere(r);

    delete geom_;
    geom_ = new CSphere3D(r);

    setNeedsUpdate();
  }
  else if (name == "cube") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    double sx = 1.0, sy = 1.0, sz = 1.0;

    if      (strs.size() == 1) {
      sx = Util::stringToReal(value);
      sy = sx;
      sz = sx;
    }
    else if (strs.size() == 3) {
      sx = Util::stringToReal(strs[0]);
      sy = Util::stringToReal(strs[1]);
      sz = Util::stringToReal(strs[2]);
    }
    else {
      app->errorMsg("bad sizes for cube");
      return false;
    }

    addCube(sx, sy, sz);

    delete geom_;
    geom_ = new CBox3D(sx, sy, sz);

    setNeedsUpdate();
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
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
addCylinder(double r, double h)
{
  uint stacks = 20;
  uint slices = 20;

  std::vector<double> x, y;

  x.resize(stacks);
  y.resize(stacks);

  x[0         ] = r; y[0         ] = 0;
  x[stacks - 1] = r; y[stacks - 1] = h;

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
addCube(double sx, double sy, double sz)
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

  double xs = sx/2.0;
  double ys = sy/2.0;
  double zs = sz/2.0;

  struct Point {
    float x;
    float y;
    float z;
  };

  std::vector<Point> v; v.resize(8);

  v[0].x = v[1].x = v[2].x = v[3].x = -xs;
  v[4].x = v[5].x = v[6].x = v[7].x =  xs;
  v[0].y = v[1].y = v[4].y = v[5].y = -ys;
  v[2].y = v[3].y = v[6].y = v[7].y =  ys;
  v[0].z = v[3].z = v[4].z = v[7].z = -zs;
  v[1].z = v[2].z = v[5].z = v[6].z =  zs;

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
setNormalTexture(const QString &filename)
{
  normalTexture_ = new CQGLTexture;

  if (! normalTexture_->load(filename, /*flip*/true)) {
    delete normalTexture_;
    normalTexture_ = nullptr;
  }
}

void
Shape3DObj::
init()
{
  Object3D::init();

  //---

  if (! s_program) {
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "layout (location = 1) in vec3 aNormal;\n"
      "layout (location = 2) in vec4 aColor;\n"
      "layout (location = 3) in vec2 aTexCoord;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "out vec3 FragPos;\n"
      "out vec3 Normal;\n"
      "out vec4 Color;\n"
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
      "in vec4 Color;\n"
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
      "uniform sampler2D normTex;\n"
      "uniform bool useTexture;\n"
      "uniform bool useNormalTexture;\n"
      "void main() {\n"
      "  vec3 norm;\n"
      "  if (useNormalTexture) {\n"
      "    norm = texture(normTex, TextCoord).rgb();\n"
      "    norm = normalize(norm*2.0 - 1.0).rgb();\n"
      "  } else {\n"
      "    norm = normalize(Normal);\n"
      "  }\n"
      "  vec3 lightDir = normalize(lightPos - FragPos);\n"
      "  float diff = max(dot(norm, lightDir), 0.0);\n"
      "  vec4 diffuseColor = Color;\n"
      "  if (useTexture) {\n"
      "    diffuseColor = texture(textureId, TexCoord);\n"
      "  }\n"
      "  vec3 diffuse = diffuseStrength*diff*vec3(diffuseColor);\n"
      "  vec3 ambient = ambientStrength*vec3(diffuseColor);\n"
      "  vec3 viewDir = normalize(viewPos - FragPos);\n"
      "  vec3 reflectDir = reflect(-lightDir, norm);\n"
      "  float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);\n"
      "  vec3 specColor = lightColor;\n"
      "  vec3 specular = specularStrength*spec*specColor;\n"
      "  vec3 result = ambient + diffuse + specular;\n"
      "  FragColor = vec4(result, diffuseColor.a);\n"
      "}\n";

    s_program = new ProgramData;

    s_program->program = new QOpenGLShaderProgram(this);

    auto *program = s_program->program;

    program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShaderSource);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    program->link();
  }

  //---

  canvas_->glGenVertexArrays(1, &vertexArrayId_);

  canvas_->glGenBuffers(1, &pointsBufferId_);
  canvas_->glGenBuffers(1, &normalsBufferId_);
  canvas_->glGenBuffers(1, &colorsBufferId_);
  canvas_->glGenBuffers(1, &texCoordBufferId_);
  canvas_->glGenBuffers(1, &indBufferId_);
}

bool
Shape3DObj::
intersect(const CGLVector3D &p1, const CGLVector3D &p2, CPoint3D &pi1, CPoint3D &pi2) const
{
  if (! geom_)
    return false;

  CLine3D line(p1.getX(), p1.getY(), p1.getZ(), p2.getX(), p2.getY(), p2.getZ());

  double tmin, tmax;
  if (! geom_->intersect(line, &tmin, &tmax))
    return false;

//if ((tmin < 0.0 || tmin > 1.0) && (tmax < 0.0 || tmax > 1.0))
//  return false;

  pi1 = line.interp(tmin);
  pi2 = line.interp(tmax);

//std::cerr << "Intersect: " << id().toStdString() << " " << tmin << " " <<  tmax << "\n";
//std::cerr << "  " << Util::point3DToString(pi1).toStdString() << " " <<
//                     Util::point3DToString(pi2).toStdString() << "\n";

  return true;
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

  // store point data in array buffer (vec3, location 0)
  uint aPos = 0;
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, pointsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D), &points_[0], GL_STATIC_DRAW);

  // set points attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(aPos);

  // store normal data in array buffer (vec3, location 1)
  uint aNormal = 1;
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, normalsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D), &normals_[0], GL_STATIC_DRAW);

  // set normals attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(aNormal, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(aNormal);

  //---

  int nc = colors_.size();

  // store color data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, colorsBufferId_);
  if (nc > 0) {
    canvas_->glBufferData(GL_ARRAY_BUFFER, nc*sizeof(Color), &colors_[0], GL_STATIC_DRAW);
  }
  else {
    static Colors s_colors_;

    if (int(s_colors_.size()) != np)
      s_colors_.resize(np);

    auto c = this->color();

    if (isInside())
      c = Color(0.8, 0.4, 0.4, 0.5);

    for (int i = 0; i < np; ++i)
      s_colors_[i] = c;

    canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(Color), &s_colors_[0], GL_STATIC_DRAW);
  }

  // set colors attrib data and format (for current buffer) (vec4, location 2)
  uint aColor = 2;
  canvas_->glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(Color), nullptr);
  canvas_->glEnableVertexAttribArray(aColor);

  //---

  int nt = texCoords_.size();

  useTexture_       = (texture_       && nt > 0);
  useNormalTexture_ = (normalTexture_ && nt > 0);

  if (isInside()) {
    useTexture_       = false;
    useNormalTexture_ = false;
  }

  // store texture point data in array buffer
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

  // set texture points attrib data and format (for current buffer) (vec2, location 3)
  uint aTexCoords = 3;
  canvas_->glVertexAttribPointer(aTexCoords, 2, GL_FLOAT, GL_FALSE, sizeof(CGLVector2D), nullptr);
  canvas_->glEnableVertexAttribArray(aTexCoords);

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

  auto *light = canvas_->currentLight();

  auto lightPos   = light->position();
  auto lightColor = light->color();

  auto *program = s_program->program;

  program->bind();

  program->setUniformValue("lightColor", CQGLUtil::toVector(lightColor));
  program->setUniformValue("lightPos"  , CQGLUtil::toVector(lightPos));
  program->setUniformValue("viewPos"   , CQGLUtil::toVector(canvas_->viewPos()));

  program->setUniformValue("ambientStrength" , float(canvas_->ambient()));
  program->setUniformValue("diffuseStrength" , float(canvas_->diffuse()));
  program->setUniformValue("specularStrength", float(canvas_->specular()));
  program->setUniformValue("shininess"       , float(canvas_->shininess()));

  program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  setModelMatrix();
  program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

  //---

  canvas_->glBindVertexArray(vertexArrayId_);

  //---

  program->setUniformValue("useTexture", useTexture_);
  program->setUniformValue("useNormalTexture", useNormalTexture_);
  program->setUniformValue("textureId", 0);

  if (useTexture_ || useNormalTexture_)
    glEnable(GL_TEXTURE_2D);

  if (useTexture_) {
    glActiveTexture(GL_TEXTURE0);
    texture_->bind();
  }

  if (useNormalTexture_) {
    glActiveTexture(GL_TEXTURE1);
    normalTexture_->bind();
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

  if (useTexture_ || useNormalTexture_)
    glDisable(GL_TEXTURE_2D);
}

//---

bool
Cube3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Cube3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Cube3DObj::
Cube3DObj(Canvas3D *canvas) :
 Shape3DObj(canvas)
{
}

void
Cube3DObj::
init()
{
  Object3D::init();

  //---

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
Shader3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Shader3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Shader3DObj::
Shader3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

void
Shader3DObj::
init()
{
  Object3D::init();

  //---

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
Shader3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Shader3DObj::
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
    return Object3D::setValue(name, value, args);

  return true;
}

void
Shader3DObj::
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
Shader3DObj::
tick()
{
  elapsed_ += canvas_->redrawTimeOut()/1000.0;

  Object3D::tick();
}

void
Shader3DObj::
render()
{
  updateShaders();

  program_->bind();

  //---

  int elapsedLocation = program_->uniformLocation("iTime");
  program_->setUniformValue(elapsedLocation, GLfloat(elapsed_));

  //---

  int frameLocation = program_->uniformLocation("iFrame");
  program_->setUniformValue(frameLocation, GLint(ticks_));

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

size_t                                      ParticleList3DObj::s_maxPoints = 50000;
ParticleList3DObj::ParticleListProgramData *ParticleList3DObj::s_program   = nullptr;

bool
ParticleList3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new ParticleList3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

ParticleList3DObj::
ParticleList3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

QVariant
ParticleList3DObj::
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
  else if (name == "range") {
    CRMinMax xrange, yrange, zrange;

    for (const auto &p : points_) {
      xrange.add(p.x());
      yrange.add(p.y());
      zrange.add(p.z());
    }

    QStringList strs;

    strs << QString::number(xrange.min());
    strs << QString::number(yrange.min());
    strs << QString::number(zrange.min());
    strs << QString::number(xrange.max());
    strs << QString::number(yrange.max());
    strs << QString::number(zrange.max());

    return strs.join(" ");
  }
  else
    return Object3D::getValue(name, args);
}

bool
ParticleList3DObj::
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
        return false;

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
        return false;

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
#ifdef CQSANDBOX_FLOCKING
  else if (name == "flocking") {
    delete flocking_;
#ifdef CQSANDBOX_FIREWORKS
    delete fireworks_;
#endif

    flocking_ = new CFlocking;

    auto n = flocking_->numBoids();

    setNumPoints(n);

    updateFlocking();
  }
#endif
#ifdef CQSANDBOX_FIREWORKS
  else if (name == "fireworks") {
#ifdef CQSANDBOX_FLOCKING
    delete flocking_;
#endif
    delete fireworks_;

    fireworks_ = new CFireworks;

    auto n = 100;

    setNumPoints(n);

    updateFireworks();
  }
#endif
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
ParticleList3DObj::
setPoints(const Points &points)
{
  setNumPoints(points.size());

  points_ = points;
}

void
ParticleList3DObj::
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
ParticleList3DObj::
init()
{
  Object3D::init();

  //---

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
      "  gl_Position = (projection*view*model*center) + 0.005*position;\n"
      "}\n";

    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "varying lowp vec4 col;\n"
      "void main() {\n"
      "  gl_FragColor = col;\n"
      "}\n";

    s_program = new ParticleListProgramData;

    s_program->program = new QOpenGLShaderProgram(this);

    auto *program = s_program->program;

    program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShaderSource);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    program->link();

    // get program variables
    s_program->positionAttr = program->attributeLocation("position");
    Q_ASSERT(s_program->positionAttr != -1);

    s_program->centerAttr = program->attributeLocation("center");
    Q_ASSERT(s_program->centerAttr != -1);

    s_program->colorAttr = program->attributeLocation("color");
    Q_ASSERT(s_program->colorAttr != -1);

    s_program->projectionUniform = program->uniformLocation("projection");
    Q_ASSERT(s_program->projectionUniform != -1);

    s_program->viewUniform = program->uniformLocation("view");
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
ParticleList3DObj::
tick()
{
#ifdef CQSANDBOX_FLOCKING
  if      (flocking_) {
    flocking_->update(0.1);

    updateFlocking();

    setNeedsUpdate();
  }
#endif

#ifdef CQSANDBOX_FIREWORKS
  if (fireworks_) {
    fireworks_->step();

    fireworks_->updateParticles();

    fireworks_->updateCurrentParticles();

    updateFireworks();

    setNeedsUpdate();
  }
#endif

  Object3D::tick();
}

#ifdef CQSANDBOX_FLOCKING
void
ParticleList3DObj::
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
#endif

#ifdef CQSANDBOX_FIREWORKS
void
ParticleList3DObj::
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
#endif

void
ParticleList3DObj::
render()
{
  auto *program = s_program->program;

  glDisable(GL_CULL_FACE);

  program->bind();

  program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  setModelMatrix();

  program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

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
   4,
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

  program->release();
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
  Object3D::init();

  //---

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

    auto *program1 = s_program1->program;

    program1->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShader1);
    program1->addShaderFromSourceCode(QOpenGLShader::Geometry, geometryShader1);
    program1->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader1);

    program1->link();

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

    auto *program2 = s_program2->program;

    program2->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShader2);
    program2->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader2);

    program2->link();
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

bool
Graph3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if (name == "dot_file") {
    (void) loadDotFile(value);
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
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

  Object3D::tick();
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
  setModelMatrix();
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
  auto *program1 = s_program1->program;

  program1->bind();

  program1->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  program1->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  program1->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

  canvas_->glBindVertexArray(pointsArrayId_);

  int np = points_.size();

  glDrawArrays(GL_POINTS, 0, np);
//glDrawArrays(GL_TRIANGLES, 0, np);

  program1->release();

  //------

  auto *program2 = s_program2->program;

  program2->bind();

  program2->setUniformValue("lineColor", toVector(lineColor_));

  program2->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  program2->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  program2->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

  canvas_->glBindVertexArray(pointsArrayId_);

  int nl = linePoints_.size();

  glDrawArrays(GL_LINES, 0, nl);

  program2->release();
}

//---

ProgramData *Surface3DObj::s_program   = nullptr;

bool
Surface3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Surface3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Surface3DObj::
Surface3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

QVariant
Surface3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Surface3DObj::
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

    setNeedsUpdate();
  }
#ifdef CQSANDBOX_WATER_SURFACE
  else if (name == "water_surface") {
    bool ok;
    nx_ = std::max(value.toInt(&ok), 1);
    ny_ = nx_;

    resizePoints();

#ifdef CQSANDBOX_FLAG
    delete flag_;
    flag_ = nullptr;
#endif

    delete waterSurface_;
    waterSurface_ = new CWaterSurface(nx_);

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
#endif
#ifdef CQSANDBOX_FLAG
  else if (name == "flag") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() >= 2) {
      bool ok;
      nx_ = std::max(strs[0].toInt(&ok), 0);
      ny_ = std::max(strs[1].toInt(&ok), 0);
    }

    resizePoints();

#ifdef CQSANDBOX_WATER_SURFACE
    delete waterSurface_;
    waterSurface_ = nullptr;
#endif

    delete flag_;
    flag_ = new CFlag(-0.5, -0.5, 1, 1, nx_, ny_);

    flag_->setWind(true);
    flag_->setWindForce(3.0);

    //---

    updateFlag();
  }
#endif
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
        return false;

      int ixy = iy*nx_ + ix;

      auto z = Util::stringToReal(value);

      points_[ixy].setZ(z);
    }
    else
      app->errorMsg("Missing index for point");

    setNeedsUpdate();
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
        return false;

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

    setNeedsUpdate();
  }
  else if (name == "wireframe") {
    wireframe_ = Util::stringToBool(value);

    setNeedsUpdate();
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Surface3DObj::
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
Surface3DObj::
init()
{
  Object3D::init();

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

    auto *program = s_program->program;

    program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShaderSource);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    program->link();
  }

  //---

  canvas_->glGenVertexArrays(1, &vertexArrayId_);

  canvas_->glGenBuffers(1, &pointsBufferId_);
  canvas_->glGenBuffers(1, &normalsBufferId_);
  canvas_->glGenBuffers(1, &colorsBufferId_);
  canvas_->glGenBuffers(1, &indBufferId_);
}

void
Surface3DObj::
tick()
{
#ifdef CQSANDBOX_WATER_SURFACE
  if (waterSurface_) {
    waterSurface_->step(0.1);

    updateWaterSurface();

    setNeedsUpdate();
  }
#endif

#ifdef CQSANDBOX_FLAG
  if (flag_) {
    flag_->step(0.0005);

    updateFlag();

    setNeedsUpdate();
  }
#endif

  Object3D::tick();
}

#ifdef CQSANDBOX_WATER_SURFACE
void
Surface3DObj::
updateWaterSurface()
{
  uint nxy = nx_*ny_;

  for (uint i = 0; i < nxy; ++i) {
    points_[i].setZ(waterSurface_->getZ(i));
  }
}
#endif

#ifdef CQSANDBOX_FLAG
void
Surface3DObj::
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
#endif

void
Surface3DObj::
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
Surface3DObj::
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
Surface3DObj::
render()
{
  updateGL();

  if (wireframe_)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  //---

  auto *light = canvas_->currentLight();

  auto lightPos   = light->position();
  auto lightColor = light->color();

  auto *program = s_program->program;

  program->bind();

  program->setUniformValue("lightColor", CQGLUtil::toVector(lightColor));
  program->setUniformValue("lightPos"  , CQGLUtil::toVector(lightPos));
  program->setUniformValue("viewPos"   , CQGLUtil::toVector(canvas_->viewPos()));

  program->setUniformValue("ambientStrength" , float(canvas_->ambient()));
  program->setUniformValue("diffuseStrength" , float(canvas_->diffuse()));
  program->setUniformValue("specularStrength", float(canvas_->specular()));
  program->setUniformValue("shininess"       , float(canvas_->shininess()));

  program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  setModelMatrix();
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
  Object3D::init();

  //---

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
      "uniform sampler2D textureId;\n"
      "void main() {\n"
       " vec4 tc = texture(textureId, uv0);\n"
      "  gl_FragColor = vec4(col.r, col.g, col.b, tc.r);\n"
      "}\n";

    s_program = new TextProgramData;

    s_program->program = new QOpenGLShaderProgram(this);

    auto *program = s_program->program;

    program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShaderSource);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    program->link();

    //---

    s_program->posAttr = program->attributeLocation("position");
    Q_ASSERT(s_program->posAttr != -1);

    s_program->colAttr = program->attributeLocation("color");
  //Q_ASSERT(s_program->colAttr != -1);

    s_program->texPosAttr = program->attributeLocation("texCoord0");
    Q_ASSERT(s_program->texPosAttr != -1);

    s_program->projectionUniform = program->uniformLocation("projection");
    Q_ASSERT(s_program->projectionUniform != -1);

    s_program->viewUniform = program->uniformLocation("view");
    Q_ASSERT(s_program->viewUniform != -1);

    s_program->textureUniform = program->uniformLocation("textureId");
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

bool
Text3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if (name == "text")
    setText(value);
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Text3DObj::
updateTextData()
{
  vertices_.clear();
  uvs_     .clear();
  indexes_ .clear();
  colors_  .clear();

  const auto &color = this->color();

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

  xscale_ = size_;
  yscale_ = size_;
  zscale_ = 1.0;

  auto matrixFlags = ModelMatrixFlags::TRANSLATE | ModelMatrixFlags::SCALE;

  if (isRotated())
    matrixFlags |= ModelMatrixFlags::ROTATE;

  setModelMatrix(matrixFlags);

  //------

  auto *program = s_program->program;

  program->bind();

  program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

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

  // color attribute populated from 3 floats (r, g, b, a)
  canvas_->glVertexAttribPointer(s_program->colAttr, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
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

  program->setUniformValue(s_program->textureUniform, GL_TEXTURE0);

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

  program->release();

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
  Object3D::init();

  //---

  canvas_->glGenVertexArrays(1, &vertexArrayId_);

  canvas_->glGenBuffers(1, &pointsBufferId_);
}

void
Path3DObj::
initShader()
{
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

    auto *program = s_program->program;

    program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShaderSource);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    program->link();
  }
}

void
Path3DObj::
setLine(const CGLVector3D &p1, const CGLVector3D &p2)
{
  path_.clear();

  path_.moveTo(p1);
  path_.lineTo(p2);

  updatePoints();

  setNeedsUpdate();
}

QVariant
Path3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
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

    setNeedsUpdate();
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
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
  initShader();

  updateGL();

  //---

  auto *program = s_program->program;

  program->bind();

  program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  setModelMatrix();
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
  Object3D::init();
}

QVariant
Axis3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Axis3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

  if      (name == "start") {
    start_ = stringToVector3D(tcl, value);

    setNeedsUpdate();
  }
  else if (name == "end") {
    end_ = stringToVector3D(tcl, value);

    setNeedsUpdate();
  }
  else if (name == "min") {
    min_ = Util::stringToReal(value);

    setNeedsUpdate();
  }
  else if (name == "max") {
    max_ = Util::stringToReal(value);

    setNeedsUpdate();
  }
  else if (name == "auto_range") {
    if      (value == "x") {
      const auto &xrange = canvas_->xrange();

      start_ = CGLVector3D(-1, -0.5, -0.5);
      end_   = CGLVector3D( 1, -0.5, -0.5);
      min_   = xrange.min();
      max_   = xrange.max();
    }
    else if (value == "y") {
      const auto &yrange = canvas_->yrange();

      start_ = CGLVector3D(-0.5, -1, -0.5);
      end_   = CGLVector3D(-0.5,  1, -0.5);
      min_   = yrange.min();
      max_   = yrange.max();
    }
    else if (value == "z") {
      const auto &zrange = canvas_->zrange();

      start_ = CGLVector3D(-0.5, -0.5,  1);
      end_   = CGLVector3D(-0.5, -0.5, -1);
      min_   = zrange.min();
      max_   = zrange.max();
    }

    setNeedsUpdate();
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Axis3DObj::
tick()
{
  updateObjects();

  Object3D::tick();
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
  setModelMatrix();

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

ProgramData *Sprite3DObj::s_program = nullptr;

bool
Sprite3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Sprite3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Sprite3DObj::
Sprite3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

void
Sprite3DObj::
init()
{
  Object3D::init();

  //---

  if (! s_program) {
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "layout (location = 1) in vec2 aTexCoord;\n"
      "out vec2 TexCoord;\n"
      "uniform mat4 model;\n"
      "uniform mat4 view;\n"
      "void main() {\n"
      "  TexCoord = aTexCoord;\n"
      "  //gl_Position = vec4(aPos, 1.0);\n"
      "  gl_Position = view * model * vec4(aPos, 1.0);\n"
      "}\n";
    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "out vec4 FragColor;\n"
      "in vec2 TexCoord;\n"
      "uniform sampler2D textureId;\n"
      "void main() {\n"
      "  vec4 texColor = texture(textureId, TexCoord);\n"
      "  if (texColor.a < 0.1) {\n"
      "    discard;\n"
      "  }\n"
      " FragColor = texColor;\n"
      "}\n";

    s_program = new ProgramData;

    s_program->program = new QOpenGLShaderProgram(this);

    auto *program = s_program->program;

    program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShaderSource);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    program->link();
  }

  //---

  updateBuffer();
}

void
Sprite3DObj::
updateBuffer()
{
  auto *program = s_program->program;

  delete buffer_;

  buffer_ = new CQGLBuffer(program);

  auto z = position().z;

  buffer_->addPoint(-1.0f,  1.0f, float(z)); buffer_->addTexturePoint(0.0f, 0.0f);
  buffer_->addPoint(-1.0f, -1.0f, float(z)); buffer_->addTexturePoint(0.0f, 1.0f);
  buffer_->addPoint( 1.0f,  1.0f, float(z)); buffer_->addTexturePoint(1.0f, 0.0f);
  buffer_->addPoint( 1.0f, -1.0f, float(z)); buffer_->addTexturePoint(1.0f, 1.0f);

  buffer_->load();
}

void
Sprite3DObj::
setTexture(CQGLTexture *texture)
{
  textures_.clear();

  textures_.push_back(texture);
}

QVariant
Sprite3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Sprite3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

  if      (name == "add_image") {
    auto *texture = new CQGLTexture;

    if (texture->load(value, /*flip*/false))
      textures_.push_back(texture);
    else
      delete texture;
  }
  else if (name == "image_start") {
    textureStart_ = Util::stringToInt(value);
  }
  else if (name == "image_end") {
    textureEnd_ = Util::stringToInt(value);
  }
  else if (name == "velocity") {
    auto v = stringToPoint2D(tcl, value);

    xv_ = v.x;
    yv_ = v.y;
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Sprite3DObj::
tick()
{
  updateObjects();

  Object3D::tick();

  position_ = CPoint3D(position_.x + xv_, position_.y + yv_, position_.z);

  if (ticks_ % 100) {
    int textureEnd = textureEnd_;

    if (textureEnd < 0)
      textureEnd = int(textures_.size()) - 1;

    int numTextures = std::max(textureEnd - textureStart_ + 1, 0);

    ++textureNum_;

    if (textureNum_ >= numTextures)
      textureNum_ = 0;
  }
}

void
Sprite3DObj::
updateObjects()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  //---

  updateModelMatrix();
}

void
Sprite3DObj::
updateModelMatrix()
{
  xAngle_ = 2.0;
  yAngle_ = 0.0;
  zscale_ = 1.0;

  setModelMatrix();
}

void
Sprite3DObj::
render()
{
  buffer_->bind();

  //---

  auto *program = s_program->program;

  program->bind();

  program->setUniformValue("textureId", 0);

  program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  updateModelMatrix();

  program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

  glActiveTexture(GL_TEXTURE0);

  int textureNum = textureNum_ + textureStart_;

  if (textureNum >= 0 && textureNum < int(textures_.size()))
    textures_[textureNum]->bind();

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  buffer_->unbind();
}

//---

ProgramData *Skybox3DObj::s_program = nullptr;

bool
Skybox3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Skybox3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Skybox3DObj::
Skybox3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
  (void) load();
}

void
Skybox3DObj::
init()
{
  Object3D::init();
}

void
Skybox3DObj::
initShader()
{
  if (! s_program) {
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "layout (location = 0) in vec3 pos;\n"
      "out vec3 fragPos;\n"
      "uniform mat4 projection;\n"
      "uniform mat4 view;\n"
      "uniform mat4 model;\n"
      "void main() {\n"
      "  fragPos = pos;\n"
      "  //gl_Position = projection * view * model * vec4(pos, 1.0);\n"
      "  gl_Position = (projection * view * model * vec4(pos, 1.0)).xyww;\n"
      "  //gl_Position = view * model * vec4(pos, 1.0);\n"
      "}\n";
    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "uniform samplerCube textureId;\n"
      "//uniform sampler2D textureId;\n"
      "uniform bool isWireframe;\n"
      "in vec3 fragPos;\n"
      "out vec4 outCol;\n"
      "void main() {\n"
      "  if (isWireframe) {\n"
      "    outCol = vec4(fragPos, 1.0);\n"
      "  } else {\n"
      "    outCol = texture(textureId, fragPos);\n"
      "    //outCol = texture(textureId, 0.5*(fragPos.xy + 1.0));\n"
      "    //outCol = vec4(fragPos, 1.0);\n"
      "  }\n"
      "}\n";

    s_program = new ProgramData;

    s_program->program = new QOpenGLShaderProgram(this);

    auto *program = s_program->program;

    program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShaderSource);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    program->link();
  }
}

bool
Skybox3DObj::
load()
{
  QString buildDir = QUOTE(BUILD_DIR);

  auto filename = buildDir + "/models/ply/cube.ply";

  QFileInfo fi(filename);

  auto suffix = fi.suffix().toLower();
  auto type   = CImportBase::suffixToType(suffix.toStdString());

  import_ = CImportBase::createModel(type);

  if (! import_) {
    canvas_->app()->errorMsg(QString("Invalid model type for '%1'").arg(filename));
    return false;
  }

  CFile file(filename.toStdString());

  if (! import_->read(file)) {
    canvas_->app()->errorMsg(QString("Failed to load file '%1'").arg(filename));
    return false;
  }

  needsUpdate_ = true;

  return true;
}

QVariant
Skybox3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Skybox3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  if      (name == "images") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.length() != 6)
      return false;

    images_.resize(6);

    for (int i = 0; i < 6; ++i) {
      images_[i] = QImage(strs[i]);

      if (images_[i].isNull())
        std::cerr << "Invalid image '" << strs[i].toStdString() << "'\n";
    }

    delete cubemap_;

    cubemap_ = new CQGLCubemap;
    cubemap_->setImages(images_);

    canvas_->glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    delete texture_;

    texture_ = new CQGLTexture;
    texture_->setImage(images_[0]);

    needsUpdate_ = true;
  }
  else if (name == "cubemap") {
    useCubemap_ = Util::stringToBool(value);
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Skybox3DObj::
updateObjectData()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  auto *program = s_program->program;

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
        objectData->buffer = new CQGLBuffer(program);

      //---

      auto *buffer = objectData->buffer;

      buffer->clearAll();

      objectData->faceDatas.clear();

      //---

      const auto &faces = object->getFaces();

      int pos = 0;

      for (const auto *face : faces) {
        FaceData faceData;

        //---

        const auto &vertices = face->getVertices();

        faceData.pos = pos;
        faceData.len = int(vertices.size());

        for (const auto &v : vertices) {
          auto &vertex = object->getVertex(v);

          const auto &model = vertex.getModel();

          if (! flipYZ_)
            buffer->addPoint(float(model.x), float(model.y), float(model.z));
          else
            buffer->addPoint(float(model.x), float(model.z), float(model.y));
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

  auto sceneScale = float(1.0/max3(sceneSize.getX(), sceneSize.getY(), sceneSize.getZ()));
  //std::cerr << "Scene Scale : " << sceneScale << "\n";

  xscale_ = sceneScale;
  yscale_ = sceneScale;
  zscale_ = sceneScale;
}

void
Skybox3DObj::
render()
{
  initShader();

  updateObjectData();

  //---

//glDepthMask(GL_FALSE);
  glDepthFunc(GL_LEQUAL);

  auto *program = s_program->program;

  // setup model shader
  for (auto &po : objectDatas_) {
    auto *objectData = po.second;

    objectData->buffer->bind();

    program->bind();

    if (useCubemap_) {
      if (cubemap_) {
        cubemap_->enable(/*enable*/true);

        glActiveTexture(GL_TEXTURE0);

        cubemap_->bind();

        //cubemap_->setParameters();
      }
    }
    else {
      if (texture_) {
        texture_->enable(/*enable*/true);

        glActiveTexture(GL_TEXTURE0);

        texture_->bind();
      }
    }

    program->setUniformValue("textureId", 0);

    // pass projection matrix to shader (note that in this case it could change every frame)
    program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));

    // camera/view transformation
    program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

    // model rotation
    program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix_));

    program->setUniformValue("isWireframe", canvas_->isWireframe() ? 1 : 0);

#if 0
    // render model
    for (const auto &faceData : objectData->faceDatas) {
      if (! canvas_->isWireframe()) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glDrawArrays(GL_TRIANGLE_FAN, faceData.pos, faceData.len);
      }
      else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glDrawArrays(GL_TRIANGLE_FAN, faceData.pos, faceData.len);
      }
    }
#else
    if (! canvas_->isWireframe()) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
#endif

    if (useCubemap_) {
      if (cubemap_) {
        cubemap_->enable(/*enable*/false);

        cubemap_->unbind();

        //cubemap_->setParameters();
      }
    }
    else {
      if (texture_) {
        texture_->enable(/*enable*/false);

        texture_->unbind();
      }
    }

    objectData->buffer->unbind();
  }

//glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);
}

//---

#ifdef CQSANDBOX_OTHELLO
bool
Othello3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Othello3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Othello3DObj::
Othello3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
  board_ = new COthelloBoard;
}

void
Othello3DObj::
init()
{
  Object3D::init();
}

QVariant
Othello3DObj::
getValue(const QString &name, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  struct Index {
    int ix { -1 };
    int iy { -1 };

    bool isValid() { return (ix >= 0 && iy >= 0); }
  };

  auto argsToIndex = [&]() {
    Index ind;

    if (args.size() < 1) {
      app->errorMsg("Missing index for " + name);
      return ind;
    }

    QStringList strs;
    (void) tcl->splitList(args[0], strs);

    if (strs.size() != 2) {
      app->errorMsg("Missing index for " + name);
      return ind;
    }

    ind.ix = Util::stringToInt(strs[0]);
    ind.iy = Util::stringToInt(strs[1]);

    return ind;
  };

  auto stringToPiece = [](const QString &str) {
    if      (str.toLower() == "white")
      return COTHELLO_PIECE_WHITE;
    else if (str.toLower() == "black")
      return COTHELLO_PIECE_BLACK;
    else
      return COTHELLO_PIECE_NONE;
  };

  if      (name == "init_board") {
  }
  else if (name == "board_piece") {
  }
  else if (name == "can_move_anywhere") {
  }
  else if (name == "can_move") {
    if (args.size() != 2) {
      app->errorMsg("Invalid args for " + name);
      return QVariant();
    }

    auto ind = argsToIndex();
    if (! ind.isValid()) return QVariant();

    auto b = board_->canMove(ind.ix, ind.iy, stringToPiece(args[1]));

    return QVariant(b);
  }
  else if (name == "do_move") {
    if (args.size() != 2) {
      app->errorMsg("Invalid args for " + name);
      return QVariant();
    }

    auto ind = argsToIndex();
    if (! ind.isValid()) return QVariant();

    board_->doMove(ind.ix, ind.iy, stringToPiece(args[1]));

    return QVariant();
  }
  else if (name == "is_white_piece") {
    auto ind = argsToIndex();
    if (! ind.isValid()) return QVariant();

    return QVariant(board_->getPiece(ind.ix, ind.iy) == COTHELLO_PIECE_WHITE ? 1 : 0);
  }
  else if (name == "is_black_piece") {
    auto ind = argsToIndex();
    if (! ind.isValid()) return QVariant();

    return QVariant(board_->getPiece(ind.ix, ind.iy) == COTHELLO_PIECE_BLACK ? 1 : 0);
  }
  else if (name == "num_white") {
  }
  else if (name == "num_black") {
  }
  else if (name == "num") {
  }
  else if (name == "best_move") {
    if (args.size() != 1) {
      app->errorMsg("Invalid args for " + name);
      return QVariant();
    }

    int depth = 1;

    int ix, iy;

    auto b = board_->getBestMove(stringToPiece(args[0]), depth, &ix, &iy);

    QString res;

    if (b)
      res = QString("%1 %2").arg(ix).arg(iy);
    else
      res = QString("-1 -1");

    return QVariant(res);
  }
  return Object3D::getValue(name, args);
}

bool
Othello3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  return Object3D::setValue(name, value, args);
}

void
Othello3DObj::
render()
{
}
#endif

//---

#ifdef CQSANDBOX_FIELD_RUNNERS
bool
FieldRunners3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new FieldRunners3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

FieldRunners3DObj::
FieldRunners3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
  enum { NUM_ROWS = 15 };
  enum { NUM_COLS = 26 };

  runners_ = new CFieldRunners;

  runners_->init();

  runners_->build(NUM_ROWS, NUM_COLS);
}

void
FieldRunners3DObj::
init()
{
  Object3D::init();
}

void
FieldRunners3DObj::
tick()
{
  runners_->update();
}

QVariant
FieldRunners3DObj::
getValue(const QString &name, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  struct Index {
    int ix { -1 };
    int iy { -1 };

    bool isValid() { return (ix >= 0 && iy >= 0); }
  };

  auto argsToIndex = [&]() {
    Index ind;

    if (args.size() < 1) {
      app->errorMsg("Missing index for " + name);
      return ind;
    }

    QStringList strs;
    (void) tcl->splitList(args[0], strs);

    if (strs.size() != 2) {
      app->errorMsg("Missing index for " + name);
      return ind;
    }

    ind.ix = Util::stringToInt(strs[0]);
    ind.iy = Util::stringToInt(strs[1]);

    return ind;
  };

  if (name == "cell_bg") {
    (void) argsToIndex();
  }
  return Object3D::getValue(name, args);
}

bool
FieldRunners3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto setTexture = [&](const QString &id, const QString &filename) {
    auto p = textures_.find(id);

    if (p != textures_.end()) {
      delete (*p).second;

      textures_.erase(p);
    }

    auto *texture = new CQGLTexture;

    if (! texture->load(filename, /*flip*/false)) {
      delete texture;
      return;
    }

    textures_[id] = texture;
  };

  if      (name == "map") {
    runners_->loadMap(value.toStdString());
  }
  else if (name.left(8) == "texture.") {
    auto id = name.mid(8);

    setTexture(id, value);
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
FieldRunners3DObj::
render()
{
  auto resizeSprites = [&](Sprites &sprites, uint n) {
    while (sprites.size() > n) {
      auto *obj = sprites.back();

      sprites.pop_back();

      canvas_->removeObject(obj);

      delete obj;
    }

    while (sprites.size() < n) {
      auto *obj = new Sprite3DObj(canvas_);

      (void) canvas_->addNewObject(obj);

      obj->init();

      sprites.push_back(obj);
    }
  };

  //---

  auto nr = runners_->getNumRows();
  auto nc = runners_->getNumCols();

  auto dx = 1.0/nc;
  auto dy = 1.0/nr;

  auto mapX = [&](int ix) { return CMathUtil::map(ix, 0, nc - 1, -1, 1); };
  auto mapY = [&](int iy) { return CMathUtil::map(iy, 0, nr - 1, 1, -1); };

  //---

  {
  auto n = uint(std::max(nr*nc, 0));

  resizeSprites(bgSprites_, n);

  int i = 0;

  for (int r = 0; r < nr; ++r) {
    auto y = mapY(r);

    for (int c = 0; c < nc; ++c, ++i) {
      auto x = mapX(c);

      CFieldRunners::FieldCell *cell;
      runners_->getCell(CFieldRunners::CellPos(r, c), &cell);

      auto *sprite = bgSprites_[i];

      sprite->setPosition(CPoint3D(x, y, 1.0));
      sprite->setScale   (dx, dy, 1.0);

      auto cellType    = CFieldRunners::CellType::EMPTY;
      auto cellSubType = CFieldRunners::CellSubType::NONE;

      if (cell) {
        cellType    = cell->type();
        cellSubType = cell->subType();
      }

      QString id;

      if      (cellType == CFieldRunners::CellType::BORDER)
        id = "border";
      else if (cellType == CFieldRunners::CellType::BLOCK)
        id = "block";
      else if (cellType == CFieldRunners::CellType::GUN)
        id = "gun";
      else if (cellType == CFieldRunners::CellType::EMPTY) {
        if      (cellSubType == CFieldRunners::CellSubType::GRASS)
          id = "grass";
        else if (cellSubType == CFieldRunners::CellSubType::STONE)
          id = "stone";
        else
          id = "grass";
      }

      auto p = textures_.find(id);

      if (p != textures_.end())
        sprite->setTexture((*p).second);
    }
  }
  }

  //---

  {
  uint nr = runners_->getNumRunners();

  resizeSprites(runnerSprites_, nr);

  for (uint i = 0; i < nr; ++i) {
    auto *runner = runners_->getRunner(i);

    const auto &pos = runner->getPos();

    auto *sprite = runnerSprites_[i];

    auto x = mapX(pos.col);
    auto y = mapY(pos.row);

    sprite->setPosition(CPoint3D(x, y, 0.5));
    sprite->setScale   (dx, dy, 1.0);

    QString id;

    if (runner->type() == CFieldRunners::RunnerType::SOLDIER)
      id = "soldier";

    auto p = textures_.find(id);

    if (p != textures_.end())
      sprite->setTexture((*p).second);
  }
  }
}
#endif

//---

}
