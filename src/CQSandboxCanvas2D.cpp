#include <CQSandboxCanvas2D.h>

#include <CQSandboxArray2DObj.h>
#include <CQSandboxArrow2DObj.h>
#include <CQSandboxAStar2DObj.h>
#include <CQSandboxAxis2DObj.h>
#include <CQSandboxCircle2DObj.h>
#include <CQSandboxCsv2DObj.h>
#include <CQSandboxGroup2DObj.h>
#include <CQSandboxImage2DObj.h>
#include <CQSandboxLine2DObj.h>
#include <CQSandboxPalette2DObj.h>
#include <CQSandboxParticle2DObj.h>
#include <CQSandboxPath2DObj.h>
#include <CQSandboxPointList2DObj.h>
#include <CQSandboxRect2DObj.h>
#include <CQSandboxShlib2DObj.h>
#include <CQSandboxQuadTree2DObj.h>
#include <CQSandboxRenderer2DObj.h>
#include <CQSandboxText2DObj.h>
#include <CQSandboxVector2DObj.h>

#include <CQSandboxClass2DObj.h>
#include <CQSandboxInstance2DObj.h>

#include <CQSandboxParticleSystem.h>
#include <CQSandboxControl2D.h>
#include <CQSandboxViewport.h>
#include <CQSandboxToolbar2D.h>
#include <CQSandboxStatus.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQRubberBand.h>
#include <CQTclUtil.h>
#include <CQUtil.h>

#ifdef CQSANDBOX_CIRCLES
#include <CCircleFactor.h>
#endif

#include <CFile.h>

#include <QFile>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

namespace CQSandbox {

template<typename T>
int createObjectProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv) {
  auto *th = static_cast<Canvas2D *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);

  if (! T::create(th, args))
    return TCL_ERROR;

  return TCL_OK;
}

template<typename T>
int createTclObjectProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv) {
  auto *th = static_cast<Canvas2D *>(clientData);
  assert(th);

  if (! T::create(th, objc, objv))
    return TCL_ERROR;

  return TCL_OK;
}

}

//---

namespace CQSandbox {

Canvas2D::
Canvas2D(App *app) :
 QFrame(app), app_(app)
{
  setFocusPolicy(Qt::StrongFocus);

  setMouseTracking(true);

  //---

  tcl_ = new CQTcl;

  tcl_->init();

  //---

  psys_ = new ParticleSystem;

  //---

  rubberBand_ = new CQRubberBand(this);
}

CQTcl *
Canvas2D::
tcl() const
{
  return tcl_;
}

void
Canvas2D::
init(const QStringList &tclArgs)
{
  assert(! initialized_);

  initialized_ = true;

  //---

  tclArgs_ = tclArgs;

  //---

  addCommands();

  runTclCmd("proc init { args } { }");
  runTclCmd("proc resize { args } { }");
  runTclCmd("proc update { args } { }");

  runTclCmd("proc drawBg { args } { }");
  runTclCmd("proc drawFg { args } { }");

  runTclCmd("proc keyPress { args } { }");
  runTclCmd("proc keyRelease { args } { }");

  runTclCmd("proc mousePress { args } { }");
  runTclCmd("proc mouseMove { args } { }");
  runTclCmd("proc mouseRelease { args } { }");

  runTclCmd("proc setMode { args } { }");

  runTclCmd("proc rubberBandRelease { args } { }");

  //---

  addViewport();

  //---

  timer_ = new QTimer;
  connect(timer_, &QTimer::timeout, this, &Canvas2D::timerSlot);

  stepTimer_ = new QTimer;
  stepTimer_->setSingleShot(true);
  connect(stepTimer_, &QTimer::timeout, this, &Canvas2D::stepTimerSlot);

  drawTimer_ = new QTimer;
  connect(drawTimer_, &QTimer::timeout, this, &Canvas2D::drawTimerSlot);

  //---

  stylePen_   = QPen(Qt::black);
  styleBrush_ = QBrush(Qt::white);
}

void
Canvas2D::
setType(const Type &type)
{
  if (type != type_) {
    type_ = type;

    QString mode;

    switch (type_) {
      case Type::CAMERA: mode = "camera"; break;
      case Type::MODEL : mode = "model" ; break;
      case Type::GAME  : mode = "game"  ; break;
    }

    runTclCmd(QString("setMode {%1}").arg(mode));

    Q_EMIT typeChanged();
  }
}

void
Canvas2D::
addCommands()
{
  auto *tcl = this->tcl();

  tcl->createAlias("echo", "puts");

  tcl->setVar("PI", M_PI);

  // global
  tcl->createObjCommand("sb::canvas",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas2D::canvasProc),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::palette",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas2D::paletteProc),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::style",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas2D::styleProc),
    static_cast<CQTcl::ObjCmdData>(this));

  // viewport
  tcl->createObjCommand("sb::viewport",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas2D::viewportProc),
    static_cast<CQTcl::ObjCmdData>(this));

  // objects
  tcl->createObjCommand("sb::group",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Group2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::circle",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Circle2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::rect",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Rect2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::text",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Text2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::line",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Line2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::image",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Image2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::path",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Path2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::point_list",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<PointList2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::arrow",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Arrow2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::axis",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Axis2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::particle",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Particle2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  //---

  // data
  tcl->createObjCommand("sb::vector",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Vector2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::array",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Array2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::csv",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Csv2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::astar",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<AStar2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::color_range",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Palette2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  //---

  // ui
  tcl->createObjCommand("sb::real_edit",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<RealEdit>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::integer_edit",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<IntegerEdit>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::button",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<ButtonObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::ui",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas2D::uiProc),
    static_cast<CQTcl::ObjCmdData>(this));

  //---

  // graphics
  tcl->createObjCommand("sb::renderer",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Renderer2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

#if 0
  // TODO: remove
  tcl->createObjCommand("sb::draw_point",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas2D::drawPointProc),
    static_cast<CQTcl::ObjCmdData>(this));
#endif

  //---

  // layout
#ifdef CQSANDBOX_CIRCLES
  tcl->createObjCommand("sb::circles_group",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<CirclesGroupObj>),
    static_cast<CQTcl::ObjCmdData>(this));
#endif

  tcl->createObjCommand("sb::quad_tree",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<QuadTree2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  //---

  tcl->createObjCommand("sb::shlib",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Shlib2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  //---

  // math

#if 0
  tcl->createObjCommand("sb::fmul",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas2D::fmulProc),
    static_cast<CQTcl::ObjCmdData>(this));
  tcl->createObjCommand("sb::fma", // fused multiply and add (A*B) + C
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas2D::fmaProc),
    static_cast<CQTcl::ObjCmdData>(this));
#endif

  tcl->createObjCommand("sb::hypot",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas2D::hypotProc),
    static_cast<CQTcl::ObjCmdData>(this));

  //---

  tcl->createObjCommand("sb::help",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas2D::helpProc),
    static_cast<CQTcl::ObjCmdData>(this));

  //---

  tcl->createObjCommand("sb::class",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<Class2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));
  tcl->createObjCommand("sb::method",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas2D::methodProc),
    static_cast<CQTcl::ObjCmdData>(this));
  tcl->createObjCommand("sb::instance",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createTclObjectProc<Instance2DObj>),
    static_cast<CQTcl::ObjCmdData>(this));
  tcl->createObjCommand("sb::invoke",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas2D::invokeProc),
    static_cast<CQTcl::ObjCmdData>(this));
}

void
Canvas2D::
createObjCommand(Object2D *obj)
{
  auto *tcl = this->tcl();

  auto name = obj->getCommandName();

  tcl->createObjCommand(name,
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas2D::objectCommandProc),
    static_cast<CQTcl::ObjCmdData>(obj));
}

void
Canvas2D::
createObjTclCommand(Object2D *obj)
{
  auto *tcl = this->tcl();

  auto name = obj->getCommandName();

  tcl->createObjCommand(name,
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas2D::objectTclCommandProc),
    static_cast<CQTcl::ObjCmdData>(obj));
}

//---

Rect2D
Canvas2D::
rectToPixel(const Rect2D &rect) const
{
  auto p1 = pointToPixel(rect.ll);
  auto p2 = pointToPixel(rect.ur);

  return Rect2D(p1, p2);
}

Point2D
Canvas2D::
pointToPixel(const Point2D &p) const
{
  if (p.x.units == Units::PIXEL)
    return p;

  auto *viewport = currentViewport();

  double px, py;
  if (viewport->hasRange)
    viewport->displayRange.windowToPixel(p.x.value, p.y.value, &px, &py);
  else {
    px = p.x.value;
    py = p.y.value;
  }

  return Point2D::makePixel(px, py);
}

Point2D
Canvas2D::
pointToWindow(const Point2D &p) const
{
  if (p.x.units == Units::WINDOW)
    return p;

  auto *viewport = currentViewport();

  double x, y;
  if (viewport->hasRange)
    viewport->displayRange.pixelToWindow(p.x.value, p.y.value, &x, &y);
  else {
    x = p.x.value;
    y = p.y.value;
  }

  return Point2D::makeWindow(x, y);
}

QSizeF
Canvas2D::
pixelSizeToWindow(const QSizeF &psize) const
{
  auto *viewport = currentViewport();

  if (viewport->hasRange) {
    double x1, y1, x2, y2;
    viewport->displayRange.pixelToWindow(0.0          , 0.0           , &x1, &y1);
    viewport->displayRange.pixelToWindow(psize.width(), psize.height(), &x2, &y2);

    return QSizeF(std::abs(x2 - x1), std::abs(y2 - y1));
  }
  else
    return QSizeF(psize.width(), psize.height());
}

void
Canvas2D::
play()
{
  if (! running_) {
    step();

    timer_->start(timerTicks_);

    running_ = true;

    Q_EMIT runStateChanged();
  }
}

void
Canvas2D::
pause()
{
  if (running_) {
    timer_->stop();

    running_ = false;

    Q_EMIT runStateChanged();
  }
}

void
Canvas2D::
step()
{
  ++ticks_;

  //---

  bool buffered;
  stepInit(buffered);

  //---

  psys_->tick(0.01);

  for (auto *viewport : viewports_) {
    for (auto *obj : viewport->objects) {
      if (obj->isAnimating()) {
        if (! obj->step())
          obj->setAnimating(false);
      }
    }
  }

  runTclCmd("update");

  //---

  if (buffered_) {
    if (drawing_)
      return;

    drawBufferedNeeded_ = true;
  }
  else
    update();
}

void
Canvas2D::
stepInit(bool &buffered)
{
  buffered = false;

  if (! inited_) {
    app_->control2D()->setActive(false);

    inited_ = true;

    auto initCmd = QString("init");

    for (const auto &arg : tclArgs_)
      initCmd += " {" + arg + "}";

    runTclCmd(initCmd);

    initRun_ = true;

    app_->control2D()->setActive(true);

    buffered = buffered_;

    drawTimer_->start(100);
  }
}

void
Canvas2D::
drawBuffered()
{
  if (blend_)
    painter_ = new QPainter(&bufferImage2_);
  else
    painter_ = new QPainter(&bufferImage1_);

  drawStep();

  delete painter_;

  painter_ = nullptr;

  if (blend_)
    fadeImage(bufferImage1_, bufferImage2_, blendFactor_);

  update();
}

void
Canvas2D::
timerSlot()
{
  step();
}

void
Canvas2D::
stepTimerSlot()
{
  step();
}

void
Canvas2D::
drawTimerSlot()
{
  if (drawBufferedNeeded_) {
    drawBufferedNeeded_ = false;

    drawBuffered();
  }
}

void
Canvas2D::
fadeImage(QImage &image1, QImage &image2, double f)
{
#if 1
  // blend image2 into faded image1
  for (int y = 0; y < pixelHeight_; ++y) {
    for (int x = 0; x < pixelWidth_; ++x) {
      QRgb pixel1 = image1.pixel(x, y);
      QRgb pixel2 = image2.pixel(x, y);

      auto a1 = qAlpha(pixel1)/255.0;

      QRgb pixel3;

      if (a1 > 0) {
        auto r1 = qRed  (pixel1)/255.0;
        auto g1 = qGreen(pixel1)/255.0;
        auto b1 = qBlue (pixel1)/255.0;

        auto r2 = qRed  (pixel2)/255.0;
        auto g2 = qGreen(pixel2)/255.0;
        auto b2 = qBlue (pixel2)/255.0;

        pixel3 = qRgb(int(255*std::min(r1*f + r2, 1.0)),
                      int(255*std::min(g1*f + g2, 1.0)),
                      int(255*std::min(b1*f + b2, 1.0)));
      }
      else
        pixel3 = pixel2;

      if (pixel3 != pixel1)
        image1.setPixel(x, y, pixel3);
    }
  }
#else
  if (f < 0) return;

  for (int y = 0; y < pixelHeight_; ++y) {
    for (int x = 0; x < pixelWidth_; ++x) {
      QRgb pixel2 = image2.pixel(x, y);

      image1.setPixel(x, y, pixel2);
    }
  }
#endif
}

void
Canvas2D::
resizeEvent(QResizeEvent *)
{
  bool running = running_;

  if (running)
    pause();

  pixelWidth_  = width();
  pixelHeight_ = height();

  if (buffered_) {
    bufferImage1_ = QImage(pixelWidth_, pixelHeight_, QImage::Format_ARGB32);
    bufferImage2_ = QImage(pixelWidth_, pixelHeight_, QImage::Format_ARGB32);

    bufferImage1_.fill(QColor(0, 0, 0, 0).rgba());
    bufferImage2_.fill(QColor(0, 0, 0, 0).rgba());
  }

  updatePixelRanges();

  if (initRun_)
    runTclCmd(QString("resize %1 %2").arg(pixelWidth_).arg(pixelHeight_));

  if (buffered_ && initRun_)
    drawBufferedNeeded_ = true;

  if (running)
    play();
}

void
Canvas2D::
updatePixelRanges()
{
  auto mapX = [&](double x) { return x*(pixelWidth_  - 1); };
  auto mapY = [&](double y) { return y*(pixelHeight_ - 1); };

  for (auto *viewport : viewports_) {
    auto x1 = mapX(viewport->rect.ll.x.value);
    auto y1 = mapY(viewport->rect.ll.y.value);
    auto x2 = mapX(viewport->rect.ur.x.value);
    auto y2 = mapY(viewport->rect.ur.y.value);

    viewport->displayRange.setPixelRange(x1, y1, x2, y2);
  }
}

void
Canvas2D::
paintEvent(QPaintEvent *)
{
  if (buffered_) {
    QPainter painter(this);

    painter.drawImage(0, 0, bufferImage1_);
  }
  else {
    if (drawing_)
      return;

    painter_ = new QPainter(this);

    drawStep();

    delete painter_;

    painter_ = nullptr;
  }
}

void
Canvas2D::
drawStep()
{
  drawing_ = true;

  bool buffered;
  stepInit(buffered);

  if (buffered) {
    drawBufferedNeeded_ = true;

    return;
  }

  for (auto *viewport : viewports_) {
    double x1, y1, x2, y2;
    viewport->displayRange.getPixelRange(&x1, &y1, &x2, &y2);

    if (y1 > y2) std::swap(y1, y2);

    auto rect = QRectF(x1, y1, x2 - x1, y2 - y1);

    painter_->setClipRect(rect);

    painter_->fillRect(rect, viewport->brush.value().color());
  }

  for (auto *viewport : viewports_) {
    currentViewport_ = viewport;

    runTclCmd(QString("drawBg %1").arg(viewport->name));

    std::map<int, Objects> layerObjects;

    for (auto *obj : viewport->objects) {
      if (! obj->isVisible())
        continue;

      layerObjects[obj->layer()].push_back(obj);
    }

    for (const auto &po : layerObjects) {
      for (auto *obj : po.second) {
        obj->draw(painter_);
      }
    }

    auto np = psys_->numberOfParticles();

    for (uint i = 0; i < np; ++i) {
      auto *particle = psys_->getParticle(i);
      assert(particle);

      auto *particle1 = dynamic_cast<Particle *>(particle);

      if (! particle1 || particle1->isDead())
        continue;

      drawParticle(painter_, particle1);
    }

    runTclCmd(QString("drawFg %1").arg(viewport->name));
  }

  currentViewport_ = nullptr;

  drawing_ = false;
}

void
Canvas2D::
drawParticle(QPainter *painter, Particle *particle)
{
  auto *obj = dynamic_cast<const Particle2DObj *>(particle->obj());

  painter->setPen(obj->pen());

  auto *position = particle->position();

  auto p = Point2D(position->x(), position->y());

  auto p1 = pointToPixel(p).qpoint();

  const auto &image = particle->image();

  if (! image.isNull()) {
    auto w = image.width ();
    auto h = image.height();

    QImage image1;
    int    w1, h1;

    if (particle->tpos() && particle->tsize()) {
      auto tpos  = particle->tpos ().value();
      auto tsize = particle->tsize().value();

      auto x1 = w*tpos.getX();
      auto y1 = h*tpos.getY();

      w1 = w*tsize.getWidth();
      h1 = h*tsize.getHeight();

      image1 = image.copy(x1, y1, w1, h1);
    }
    else {
      image1 = image;

      w1 = w;
      h1 = h;
    }

    if (particle->color()) {
      auto fg = Util::RGBAToQColor(particle->color().value());

      CQUtil::recolorImage(image1, fg, fg);
    }

    p1.setX(p1.x() - w1/2);
    p1.setY(p1.y() - h1/2);

    painter->drawImage(p1, image1);
  }
  else
    painter->drawPoint(p1);
}

void
Canvas2D::
mousePressEvent(QMouseEvent *e)
{
  mouseData_.pressed = true;
  mouseData_.button  = e->button();
  mouseData_.press   = e->pos();
  mouseData_.move1   = mouseData_.press;
  mouseData_.move2   = mouseData_.move1;

  mouseData_.isShift   = (e->modifiers() & Qt::ShiftModifier);
  mouseData_.isControl = (e->modifiers() & Qt::ControlModifier);

  //---

  pressObj_ = getObjectAtPos(mouseData_.press);

  if (pressObj_)
    pressObj_->press(e->x(), e->y());

  //---

  if (mouseData_.button == Qt::LeftButton) {
    auto showRubberBand =
      (type() == Type::MODEL || (type() == Type::GAME && tclCallbacks_.rubberBandEvent));

    if (showRubberBand) {
      rubberBand_->setBounds(mouseData_.press, mouseData_.move1);
      rubberBand_->show();
    }
  }

  //---

  auto p = pointToWindow(Point2D::makePixel(e->x(), e->y())).qpoint();

  if (tclCallbacks_.mouseEvent)
    runTclCmd(QString("mousePress %1 %2").arg(p.x()).arg(p.y()));

  //---

  update();
}

void
Canvas2D::
mouseMoveEvent(QMouseEvent *e)
{
  mouseData_.move2 = e->pos();

  mouseData_.isShift   = (e->modifiers() & Qt::ShiftModifier);
  mouseData_.isControl = (e->modifiers() & Qt::ControlModifier);

  //---

  auto p = pointToWindow(Point2D::makePixel(e->pos())).qpoint();

  if (mouseData_.button == Qt::MiddleButton) {
    if (pressObj_) {
      auto dx = mouseData_.move2.x() - mouseData_.move1.x();
      auto dy = mouseData_.move2.y() - mouseData_.move1.y();

      if (pressObj_->isSelected())
        pressObj_->move(dx, dy);
     }
  }

  //---

  auto *group = dynamic_cast<Group2DObj *>(getObjectAtPos(e->pos()));

  QString name;

  if (group) {
    auto p1 = pointToWindow(Point2D::makePixel(e->pos())).qpoint();

    auto groupRange = group->displayRange();

    double x, y;
    groupRange.pixelToWindow(p1.x(), p1.y(), &x, &y);

    p    = QPointF(x, y);
    name = group->calcId();
  }
  else {
    name = "canvas";
  }

  app_->canvasToolbar2D()->setPos(QString("%1: %2 %3").arg(name).arg(p.x()).arg(p.y()));

  //---

  if (mouseData_.pressed) {
    if (mouseData_.button == Qt::LeftButton) {
      auto showRubberBand =
        (type() == Type::MODEL || (type() == Type::GAME && tclCallbacks_.rubberBandEvent));

      if (showRubberBand) {
        rubberBand_->setBounds(mouseData_.press, mouseData_.move1);
        rubberBand_->show();
      }
    }

    if (tclCallbacks_.mouseEvent)
      runTclCmd(QString("mouseMove %1 %2").arg(p.x()).arg(p.y()));
  }

  //---

  mouseData_.move1 = mouseData_.move2;

  update();
}

void
Canvas2D::
mouseReleaseEvent(QMouseEvent *e)
{
  mouseData_.move2 = e->pos();

  //---

  auto showRubberBand =
   (type() == Type::MODEL || (type() == Type::GAME && tclCallbacks_.rubberBandEvent));

  if (mouseData_.button == Qt::LeftButton) {
    if (showRubberBand) {
      if (type() == Type::MODEL) {
        bool clear = ! mouseData_.isShift;

        auto dx = std::abs(mouseData_.move2.x() - mouseData_.press.x());
        auto dy = std::abs(mouseData_.move2.y() - mouseData_.press.y());

        if (dx < 4 && dy < 4) {
          selectObjectAtPoint(mouseData_.press, clear);
        }
        else {
          selectObjectInsideRect(QRect(mouseData_.press, mouseData_.move2), clear);
        }
      }
    }
  }

  //---

  if (pressObj_) {
    auto *releaseObj = getObjectAtPos(e->pos());

    if (pressObj_ == releaseObj)
      pressObj_->click(mouseData_.press.x(), mouseData_.press.y());
  }

  pressObj_ = nullptr;

  //---

  if (mouseData_.button == Qt::LeftButton) {
    if (showRubberBand) {
      rubberBand_->hide();

      if (tclCallbacks_.rubberBandEvent)
        runTclCmd(QString("rubberBandRelease %1 %2 %3 %4").
          arg(mouseData_.press.x()).arg(mouseData_.press.y()).
          arg(mouseData_.move2.x()).arg(mouseData_.move2.y()));
    }
  }

  //---

  mouseData_.pressed = false;
  mouseData_.button  = Qt::NoButton;

  //---

  auto p = pointToWindow(Point2D::makePixel(e->x(), e->y())).qpoint();

  if (tclCallbacks_.mouseEvent)
    runTclCmd(QString("mouseRelease %1 %2").arg(p.x()).arg(p.y()));

  update();
}

void
Canvas2D::
keyPressEvent(QKeyEvent *e)
{
  auto keyStr = getKeyString(e);

  keyPressed_[keyStr] = true;

  //---

  if (tclCallbacks_.keyEvent)
    runTclCmd(QString("keyPress {%1}").arg(keyStr));

  update();
}

void
Canvas2D::
keyReleaseEvent(QKeyEvent *e)
{
  auto keyStr = getKeyString(e);

  keyPressed_[keyStr] = false;

  //---

  if (tclCallbacks_.keyEvent)
    runTclCmd(QString("keyRelease {%1}").arg(keyStr));

  update();
}

bool
Canvas2D::
getKeyPressed(const QString &key) const
{
  auto p = keyPressed_.find(key);

  if (p == keyPressed_.end())
    return false;

  return (*p).second;
}

QString
Canvas2D::
getKeyString(QKeyEvent *e) const
{
  QString keyStr;

  if      (e->key() == Qt::Key_Left ) keyStr = "left";
  else if (e->key() == Qt::Key_Right) keyStr = "right";
  else if (e->key() == Qt::Key_Up   ) keyStr = "up";
  else if (e->key() == Qt::Key_Down ) keyStr = "down";
  else if (e->key() == Qt::Key_Space) keyStr = "space";
  else if (e->key() == Qt::Key_Tab  ) keyStr = "tab";
  else                                keyStr = e->text();

  if (keyStr == "")
    keyStr = QString("key.%1").arg(e->key());

  return keyStr;
}

void
Canvas2D::
selectObjectAtPoint(const QPoint &p, bool clear)
{
  if (clear)
    deselectAllObjects();

  auto *obj = getObjectAtPos(p);

  if (obj)
    obj->setSelected(true);
}

void
Canvas2D::
selectObjectInsideRect(const QRect &r, bool clear)
{
  if (clear)
    deselectAllObjects();

  for (auto *viewport : viewports_) {
    for (auto *obj : viewport->objects) {
      if (! obj->isVisible())
        continue;

      auto rect  = obj->calcRect();
      auto prect = rectToPixel(rect).qrect();

      if (prect.intersects(r))
        obj->setSelected(true);
    }
  }
}

void
Canvas2D::
deselectAllObjects()
{
  for (auto *viewport : viewports_) {
    for (auto *obj : viewport->objects) {
      if (obj->isSelected())
        obj->setSelected(false);
    }
  }
}

void
Canvas2D::
getSelectedObjects(std::vector<Object2D *> &objects) const
{
  for (auto *viewport : viewports_) {
    for (auto *obj : viewport->objects) {
      if (obj->isSelected())
        objects.push_back(obj);
    }
  }
}

Object2D *
Canvas2D::
getObjectAtPos(const QPoint &pos) const
{
  for (auto *viewport : viewports_) {
    for (auto *obj : viewport->objects) {
      if (! obj->isVisible())
        continue;

      auto rect  = obj->calcRect();
      auto prect = rectToPixel(rect).qrect();

      if (prect.contains(pos))
        return obj;
    }
  }

  return nullptr;
}

Object2D *
Canvas2D::
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

QString
Canvas2D::
addNewObject(Object2D *obj)
{
  addObject(obj);

  allObjects_.push_back(obj);

  obj->setInd(++lastInd_);

  if (obj->isTclCmd())
    createObjTclCommand(obj);
  else
    createObjCommand(obj);

  connect(obj, SIGNAL(stateChanged()), this, SLOT(updateStatus()));

  return obj->calcId();
}

void
Canvas2D::
addObject(Object2D *obj)
{
  auto *viewport = currentViewport();

  viewport->objects.push_back(obj);

  obj->setGroup(nullptr);

  Q_EMIT objectsChanged();
}

void
Canvas2D::
removeObject(Object2D *obj)
{
  auto *viewport = currentViewport();

  Objects objects;

  for (auto *obj1 : viewport->objects) {
    if (obj1 != obj)
      objects.push_back(obj1);
  }

  std::swap(objects, viewport->objects);

  Q_EMIT objectsChanged();
}

//---

bool
Canvas2D::
addClass(Class2DObj *obj)
{
  auto pc = classes_.find(obj->name());

  if (pc != classes_.end())
    return false;

  classes_[obj->name()] = obj;

  return true;
}

Class2DObj *
Canvas2D::
getClass(const QString &name) const
{
  auto pc = classes_.find(name);
  if (pc == classes_.end()) return nullptr;

  return (*pc).second;
}

//---

int
Canvas2D::
canvasProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas2D *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);
  if (args.size() < 1) return TCL_ERROR;

  auto *tcl = th->tcl();

  if      (args[0] == "get") {
    if (args.size() < 2) {
      th->app_->errorMsg("Missing args for get");
      return TCL_ERROR;
    }

    QStringList args1;
    for (int i = 2; i < args.size(); ++i)
      args1.push_back(args[i]);

    QVariant res;
    if (! th->getValue(args[1], args1, res))
      return TCL_ERROR;

    tcl->setResult(res);
  }
  else if (args[0] == "set") {
    if (args.size() < 3) {
      th->app_->errorMsg("Missing args for set");
      return TCL_ERROR;
    }

    QStringList args1;
    for (int i = 3; i < args.size(); ++i)
      args1.push_back(args[i]);

    th->setValue(args[1], args[2], args1);
  }
  else if (args[0] == "exec") {
    if (args.size() <= 1) {
      th->app_->errorMsg("Missing args for exec");
      return TCL_ERROR;
    }

    QString op = args[1];

    QStringList args1;
    for (int i = 2; i < args.length(); ++i)
      args1.push_back(args[i]);

    QVariant res;

    if (! th->exec(op, args1, res))
      return TCL_ERROR;

    tcl->setResult(res);
  }
  else if (args[0] == "delete") {
    if (args.size() >= 2) {
      if (args[1] == "all") {
        auto *viewport = th->currentViewport();

        Objects objects;

        std::swap(objects, viewport->objects);

        for (auto *obj : objects)
          delete obj;
      }
    }
    else
      return TCL_ERROR;
  }
  else
    return TCL_ERROR;

  return TCL_OK;
}

int
Canvas2D::
paletteProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas2D *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);
  if (args.size() < 1) return TCL_ERROR;

  auto *tcl = th->tcl();

  if      (args[0] == "hsv") {
    if (args.size() >= 2) {
      QStringList strs;
      (void) tcl->splitList(args[1], strs);

      if (strs.size() != 3)
        return TCL_ERROR;

      auto h = Util::stringToReal(strs[0]);
      auto s = Util::stringToReal(strs[1]);
      auto v = Util::stringToReal(strs[2]);

      auto c = QColor::fromHsvF(h, s, v);

      tcl->setResult(Util::colorToString(c));
    }
  }
  else if (args[0] == "get") {
    if (args.size() >= 2)
      tcl->setResult(th->getPaletteValue(args[1]));
  }
  else if (args[0] == "set") {
    if (args.size() >= 3)
      th->setPaletteValue(args[1], args[2]);
  }
  else
    return TCL_ERROR;

  return TCL_OK;
}

int
Canvas2D::
viewportProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas2D *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);
  if (args.size() < 1) return TCL_ERROR;

  auto *tcl = th->tcl();

  Rect2D rect;
  if (! Util::stringToRect2D(tcl, args[0], rect))
    return TCL_ERROR;

  auto *viewport = th->addViewport();

  viewport->rect = rect;

  tcl->setResult(viewport->name);

  return TCL_OK;
}

int
Canvas2D::
styleProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas2D *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);
  if (args.size() < 1) return TCL_ERROR;

  auto *tcl = th->tcl();

  if (args[0] == "get") {
    if (args.size() < 2)
      return TCL_ERROR;

    QVariant res;
    if (! th->getStyleValue(args[1], res))
      return TCL_ERROR;

    tcl->setResult(res);
  }
  else if (args[0] == "set") {
    if (args.size() < 3)
      return TCL_ERROR;

    if (! th->setStyleValue(args[1], args[2]))
      return TCL_ERROR;
  }
  else
    return TCL_ERROR;

  return TCL_OK;
}

//---

bool
Canvas2D::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *tcl = this->tcl();

  auto *viewport = currentViewport();

  // brush (TODO: gradient, ...)
  if      (name == "brush.color") {
    value = Util::colorToString(viewport->brush.value().color());
  }
  else if (name == "brush.target.color") {
    value = Util::colorToString(viewport->brush.target().color());
  }
  else if (name == "brush.steps") {
    value = Util::colorToString(viewport->brush.steps());
  }
  // pen (TODO: dash)
  else if (name == "pen.color") {
    value = Util::colorToString(viewport->pen.color());
  }
  else if (name == "pen.width") {
    value = Util::realToString(viewport->pen.widthF());
  }
  // window range
  else if (name == "range") {
    value = Util::rangeToString(tcl, viewport->displayRange);
  }
  else if (name == "range.xmin") {
    double x1, y1, x2, y2;
    viewport->displayRange.getWindowRange(&x1, &y1, &x2, &y2);
    value = x1;
  }
  else if (name == "range.ymin") {
    double x1, y1, x2, y2;
    viewport->displayRange.getWindowRange(&x1, &y1, &x2, &y2);
    value = y1;
  }
  else if (name == "range.xmax") {
    double x1, y1, x2, y2;
    viewport->displayRange.getWindowRange(&x1, &y1, &x2, &y2);
    value = x2;
  }
  else if (name == "range.ymax") {
    double x1, y1, x2, y2;
    viewport->displayRange.getWindowRange(&x1, &y1, &x2, &y2);
    value = y2;
  }
  else if (name == "equal_scale") {
    auto b = viewport->displayRange.getEqualScale();

    value = Util::boolToString(b);
  }
  // pixel range
  else if (name == "pixel_width") {
    value = pixelWidth_;
  }
  else if (name == "pixel_height") {
    value = pixelHeight_;
  }
  else if (name == "pixel_range") {
    double x1, y1, x2, y2;
    viewport->displayRange.getPixelRange(&x1, &y1, &x2, &y2);

    value = Util::bbox2DToString(CBBox2D(x1, y1, x2, y2));
  }
  // particles (TODO: use particle system object)
  else if (name == "particles") {
    QStringList ids;

    const auto &particles = psys_->getParticles();

    for (uint i = 0; i < particles.size(); ++i) {
      auto *particle = particles.get(int(i));

      auto *particle1 = dynamic_cast<Particle *>(particle);

      ids.push_back(particle1->obj()->getCommandName());
    }

    value = ids;
  }
  // key state
  else if (name == "key") {
    if (args.size() < 1)
      return app_->errorMsg(QString("Invalid value name '%1'").arg(name));

    value = getKeyPressed(args[0]);
  }
  // animation state
  else if (name == "ticks") {
    value = Util::intToString(ticks_);
  }
  else if (name == "play") {
    value = running_;
  }
  // renderer
  else if (name == "buffered") {
    value = buffered_;
  }
  else if (name == "font.height") {
    QFontMetrics fm(font());

    value = fm.height();
  }
  else
    return app_->errorMsg(QString("Invalid value name '%1'").arg(name));

  return true;
}

bool
Canvas2D::
setValue(const QString &name, const QString &value, const QStringList &)
{
  auto *tcl = this->tcl();

  auto *viewport = currentViewport();

  if      (name == "brush.color") {
    auto b = viewport->brush.value();

    QColor c;
    if (! Util::stringToColor(tcl, value, c))
      return false;

    b.setColor(c);

    viewport->brush = b;
  }
  else if (name == "brush.target.color") {
    auto b = viewport->brush.target();

    QColor c;
    if (! Util::stringToColor(tcl, value, c))
      return false;

    b.setColor(c);

    viewport->brush.setTarget(b);
  }
  else if (name == "brush.steps") {
    viewport->brush.setSteps(Util::stringToInt(value));
  }
  else if (name == "pen.color") {
    QColor c;
    if (! Util::stringToColor(tcl, value, c))
      return false;

    viewport->pen.setColor(c);
  }
  else if (name == "pen.width") {
    viewport->pen.setWidthF(Util::stringToReal(value));
  }
  else if (name == "range") {
    if (! Util::stringToRange(tcl, viewport->displayRange, value))
      return false;

    viewport->hasRange = true;
  }
  else if (name == "equal_scale") {
    auto *viewport = currentViewport();

    viewport->displayRange.setEqualScale(Util::stringToBool(value));

    viewport->hasRange = true;
  }
  else if (name == "view") {
    currentViewportName_ = value;
  }
  else if (name == "view.rect") {
    if (! Util::stringToRect2D(tcl, value, viewport->rect))
      return false;

    updatePixelRanges();
  }
  else if (name == "play") {
    if (Util::stringToBool(value))
      play();
    else
      pause();
  }
  else if (name == "gravity") {
    psys_->setGravity(Util::stringToReal(value));
  }
  else if (name == "buffered") {
    buffered_ = Util::stringToBool(value);

    resizeEvent(nullptr);
  }
  else if (name == "blend.enabled") {
    blend_ = Util::stringToBool(value);

    resizeEvent(nullptr);
  }
  else if (name == "blend.factor") {
    blendFactor_ = Util::stringToReal(value);
  }
  else if (name == "window.size") {
    Point2D size;
    if (! Util::stringToPoint2D(tcl, value, size))
      return false;

    int w = size.x.value;
    int h = size.y.value;

    h += app_->canvasToolbar2D()->height();
    h += app_->status()->height();

    app_->resize(w, h);
  }
  else if (name == "font.size") {
    double s = Util::stringToReal(value);

    auto font = this->font();

    double scale = 1;

    for (int i = 0; i < 8; ++i) {
      font.setPointSizeF(scale*s);

      QFontMetricsF fm(font);

      double s1 = fm.height();

      scale *= s/s1;
    }

    setFont(font);
  }
  else if (name == "controls.show") {
    auto b = Util::stringToBool(value);

    app_->control2D()->setShown(b);
  }
  else if (name == "module_dir") {
    moduleDirs_.push_back(value);
  }
  else if (name == "rubberBandEvent") {
    auto b = Util::stringToBool(value);

    tclCallbacks_.rubberBandEvent = b;
  }
  else
    return app_->errorMsg(QString("Invalid value name '%1'").arg(name));

  return true;
}

bool
Canvas2D::
exec(const QString &op, const QStringList &, QVariant &)
{
  if      (op == "update") {
    this->update();
  }
  else if (op == "step") {
    stepTimer_->start(10);
  }
  else if (op == "redraw") {
    if (buffered_)
      drawBufferedNeeded_ = true;
    else
      this->update();
  }
  else
    return false;

  return true;
}

Viewport *
Canvas2D::
currentViewport() const
{
  if (currentViewport_)
    return currentViewport_;

  for (auto *viewport : viewports_) {
    if (viewport->name == currentViewportName_)
      return viewport;
  }

  return nullptr;
}

Viewport *
Canvas2D::
addViewport()
{
  auto *viewport = new Viewport;

  viewport->canvas = this;

  viewport->name = QString("sb::viewport_obj.%1").arg(viewports_.size() + 1);

  viewport->rect = Rect2D(Point2D(0, 0), Point2D(1, 1));

  viewport->pen   = QPen(Qt::black);
  viewport->brush = QBrush(Qt::white);

  viewports_.push_back(viewport);

  updatePixelRanges();

  auto *tcl = this->tcl();

  tcl->createObjCommand(viewport->name,
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas2D::viewportCommandProc),
    static_cast<CQTcl::ObjCmdData>(viewport));

  if (currentViewportName_ == "")
    currentViewportName_ = viewport->name;

  return viewport;
}

QVariant
Canvas2D::
getPaletteValue(const QString &)
{
  return QVariant();
}

void
Canvas2D::
setPaletteValue(const QString &name, const QString &)
{
  app_->errorMsg(QString("Invalid value name '%1'").arg(name));
}

bool
Canvas2D::
getStyleValue(const QString &name, QVariant &res)
{
  if      (name == "pen.color")
    res = Util::colorToString(stylePen_.color());
  else if (name == "pen.width")
    res = Util::realToString(stylePen_.widthF());
  else
    return app_->errorMsg(QString("Invalid value name '%1'").arg(name));

  return true;
}

bool
Canvas2D::
setStyleValue(const QString &name, const QString &value)
{
  auto *tcl = this->tcl();

  if      (name == "brush.color") {
    QColor c;
    if (! Util::stringToColor(tcl, value, c))
      return false;

    styleBrush_.setColor(c);
  }
  else if (name == "pen.color") {
    QColor c;
    if (! Util::stringToColor(tcl, value, c))
      return false;

    stylePen_.setColor(c);
  }
  else if (name == "pen.width") {
    stylePen_.setWidthF(Util::stringToReal(value));
  }

  return true;
}

#if 0
int
Canvas2D::
drawPointProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas2D *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);
  if (args.size() != 5) return TCL_ERROR;

  auto x = Util::stringToReal(args[0]);
  auto y = Util::stringToReal(args[1]);
  auto r = Util::stringToReal(args[2]);
  auto g = Util::stringToReal(args[3]);
  auto b = Util::stringToReal(args[4]);

  if (th->painter_) {
    th->painter_->setPen(QColor(r, g, b));

    th->painter_->drawPoint(x, y);
  }

  return TCL_OK;
}
#endif

int
Canvas2D::
objectCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *obj = static_cast<Object2D *>(clientData);
  assert(obj);

  auto *canvas = obj->canvas();
  auto *app    = canvas->app();

  if (objc < 2) {
    (void) app->errorMsg("Missing args for object command");
    return TCL_ERROR;
  }

  auto *tcl = canvas->tcl();

  auto args = app->getArgs(objc, objv);

  auto cmd = args[0];

  if      (cmd == "get") {
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
  else if (cmd == "set") {
    if (args.size() > 2) {
      auto name  = args[1];
      auto value = args[2];

      QStringList args1;
      for (int i = 3; i < args.length(); ++i)
        args1.push_back(args[i]);

      if (! obj->setValue(name, value, args1))
        return TCL_ERROR;
    }
    else {
      app->errorMsg("Missing args for set");
      return TCL_ERROR;
    }
  }
  else if (cmd == "exec") {
    if (args.size() > 1) {
      auto op = args[1];

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
  else if (cmd == "delete") {
    canvas->removeObject(obj);

    delete obj;
  }
  else {
    (void) app->errorMsg(QString("Bad object command '%1'").arg(cmd));
    return TCL_ERROR;
  }

  return TCL_OK;
}

int
Canvas2D::
objectTclCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *obj = static_cast<Object2D *>(clientData);
  assert(obj);

  auto *canvas = obj->canvas();
  auto *app    = canvas->app();

  if (objc < 2) {
    (void) app->errorMsg("Missing args for object command");
    return TCL_ERROR;
  }

  auto *tcl = canvas->tcl();

  auto cmd = tcl->qstringFromObj(objv[1]);

  if      (cmd == "get") {
    if (objc > 2) {
      auto name = tcl->qstringFromObj(objv[2]);

      Object2D::TclObjs objs;
      for (int i = 3; i < objc; ++i)
        objs.push_back(const_cast<Tcl_Obj *>(objv[i]));

      Tcl_Obj *res;
      if (! obj->getTclValue(name, objs, res))
        return TCL_ERROR;

      tcl->setResult(res);
    }
    else {
      (void) app->errorMsg("Missing args for get");
      return TCL_ERROR;
    }
  }
  else if (cmd == "set") {
    if (objc > 3) {
      auto name = tcl->qstringFromObj(objv[2]);

      Object2D::TclObjs objs;
      for (int i = 4; i < objc; ++i)
        objs.push_back(const_cast<Tcl_Obj *>(objv[i]));

      if (! obj->setTclValue(name, const_cast<Tcl_Obj *>(objv[3]), objs))
        return TCL_ERROR;
    }
    else {
      (void) app->errorMsg("Missing args for set");
      return TCL_ERROR;
    }
  }
  else if (cmd == "exec") {
    if (objc > 2) {
      auto op = tcl->qstringFromObj(objv[2]);

      Object2D::TclObjs objs;
      for (int i = 3; i < objc; ++i)
        objs.push_back(const_cast<Tcl_Obj *>(objv[i]));

      Tcl_Obj *res;
      if (! obj->execTcl(op, objs, res))
        return TCL_ERROR;

      tcl->setResult(res);
    }
    else {
      (void) app->errorMsg("Missing args for exec");
      return TCL_ERROR;
    }
  }
  else {
    (void) app->errorMsg(QString("Bad object command '%1'").arg(cmd));
    return TCL_ERROR;
  }

  return TCL_OK;
}

int
Canvas2D::
viewportCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *viewport = static_cast<Viewport *>(clientData);
  assert(viewport);

  auto *canvas = viewport->canvas;
  auto *app    = canvas->app();

  if (objc < 2) {
    (void) app->errorMsg("Missing args for viewport command");
    return TCL_ERROR;
  }

  auto *tcl = canvas->tcl();

  auto args = app->getArgs(objc, objv);

  auto cmd = args[0];

  if      (cmd == "get") {
    if (args.size() > 1) {
      auto name = args[1];

      app->errorMsg("Invalid get name '" + name + "'");
    }
    else {
      app->errorMsg("Missing args for viewport get");
    }
  }
  else if (cmd == "set") {
    if (args.size() > 2) {
      auto name  = args[1];
      auto value = args[2];

      if      (name == "brush.color") {
        auto b = viewport->brush.value();

        QColor c;
        if (! Util::stringToColor(tcl, value, c))
          return false;

        b.setColor(c);

        viewport->brush = b;
      }
      else if (name == "range") {
        if (! Util::stringToRange(tcl, viewport->displayRange, value))
          return false;

        viewport->hasRange = true;
      }
      else if (name == "clip") {
        if (! Util::stringToRect2D(tcl, value, viewport->clip))
          return false;
      }
      else
        app->errorMsg("Invalid set name '" + name + "' for viewport");
    }
    else {
      app->errorMsg("Missing args for viewport set");
    }
  }
  else if (cmd == "exec") {
    if (args.size() > 1) {
      QString op = args[1];

#if 0
      QStringList args1;
      for (int i = 2; i < args.length(); ++i)
        args1.push_back(args[i]);
#endif

      app->errorMsg("Invalid exec op '" + op + "' for viewport");
    }
    else {
      app->errorMsg("Missing args for viewport exec");
    }
  }
  else if (cmd == "delete") {
    app->errorMsg("Invalid viewport delete");
  }
  else {
    app->errorMsg(QString("Bad viewport command '%1'").arg(cmd));
  }

  return TCL_OK;
}

#if 0
int
Canvas2D::
fmulProc(void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv)
{
  if (objc != 3) return TCL_ERROR;

  double r1, r2;
  if (Tcl_GetDoubleFromObj(interp, const_cast<Tcl_Obj *>(objv[1]), &r1) != TCL_OK ||
      Tcl_GetDoubleFromObj(interp, const_cast<Tcl_Obj *>(objv[2]), &r2) != TCL_OK)
    return TCL_ERROR;

  auto *resObj = Tcl_NewDoubleObj(r1*r2);

  Tcl_SetObjResult(interp, resObj);

  return TCL_OK;
}

int
Canvas2D::
fmaProc(void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv)
{
  if (objc != 4) return TCL_ERROR;

  double r1, r2, r3;
  if (Tcl_GetDoubleFromObj(interp, const_cast<Tcl_Obj *>(objv[1]), &r1) != TCL_OK ||
      Tcl_GetDoubleFromObj(interp, const_cast<Tcl_Obj *>(objv[2]), &r2) != TCL_OK ||
      Tcl_GetDoubleFromObj(interp, const_cast<Tcl_Obj *>(objv[3]), &r3) != TCL_OK)
    return TCL_ERROR;

  auto *resObj = Tcl_NewDoubleObj(r1*r2 + r3);

  Tcl_SetObjResult(interp, resObj);

  return TCL_OK;
}
#endif

int
Canvas2D::
hypotProc(void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv)
{
  if (objc != 3) return TCL_ERROR;

  double dx, dy;
  if (Tcl_GetDoubleFromObj(interp, const_cast<Tcl_Obj *>(objv[1]), &dx) != TCL_OK ||
      Tcl_GetDoubleFromObj(interp, const_cast<Tcl_Obj *>(objv[2]), &dy) != TCL_OK)
    return TCL_ERROR;

  auto *resObj = Tcl_NewDoubleObj(std::hypot(dx, dy));

  Tcl_SetObjResult(interp, resObj);

  return TCL_OK;
}

int
Canvas2D::
helpProc(void *clientData, Tcl_Interp *, int, const Tcl_Obj **)
{
  auto *th = static_cast<Canvas2D *>(clientData);
  assert(th);

  auto *tcl = th->tcl();

  const auto &names = tcl->commandNames();

  tcl->setResult(names);

  return TCL_OK;
}

int
Canvas2D::
methodProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas2D *>(clientData);
  assert(th);

  auto *app = th->app();

  if (objc != 5) {
    (void) app->errorMsg("Invalid args for method command");
    return TCL_ERROR;
  }

  auto args = app->getArgs(objc, objv);

  auto className = args[0];

  auto *classObj = th->getClass(className);
  if (! classObj) return TCL_ERROR;

  auto methodName = args[1];
  auto methodArgs = args[2];
  auto methodBody = args[3];

  classObj->addMethod(methodName, methodArgs, methodBody);

  return TCL_OK;
}

int
Canvas2D::
invokeProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas2D *>(clientData);
  assert(th);

  auto *app = th->app();

  if (objc < 4) {
    (void) app->errorMsg("Invalid args for invoke command");
    return TCL_ERROR;
  }

  auto args = app->getArgs(objc, objv);

  auto className = args[0];

  auto *classObj = th->getClass(className);
  if (! classObj) return TCL_ERROR;

  auto methodName   = args[1];
  auto instanceName = args[2];

  std::vector<Tcl_Obj *> args1;
  for (int i = 4; i < objc; ++i)
    args1.push_back(const_cast<Tcl_Obj *>(objv[i]));

  Tcl_Obj *res;
  if (! classObj->invokeMethod(methodName, instanceName, args1, res))
    return TCL_OK;

  auto *tcl = th->tcl();

  tcl->setResult(res);

  return TCL_OK;
}

//---

int
Canvas2D::
uiProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas2D *>(clientData);
  assert(th);

  auto *app = th->app();

  auto args = app->getArgs(objc, objv);
  if (args.size() < 1) return TCL_ERROR;

  auto *tcl = th->tcl();

  if      (args[0] == "create") {
    if (args.size() < 2) return TCL_ERROR;

    if (! app->control2D()->createUi(args[1]))
      return TCL_ERROR;
  }
  else if (args[0] == "get") {
    if (args.size() < 2) return TCL_ERROR;

    QVariant value;
    if (! app->control2D()->getUiValue(args[1], value))
      return TCL_ERROR;

    tcl->setResult(value);
  }
  else if (args[0] == "set") {
    if (args.size() < 3) return TCL_ERROR;

    QVariant value;
    if (! app->control2D()->setUiValue(args[1], args[2]))
      return TCL_ERROR;

    tcl->setResult(value);
  }
  else if (args[0] == "widget.get") {
    if (args.size() < 3) return TCL_ERROR;

    QVariant value;
    if (! app->control2D()->getUiWidgetValue(args[1], args[2], value))
      return TCL_ERROR;

    tcl->setResult(value);
  }
  else if (args[0] == "widget.set") {
    if (args.size() < 4) return TCL_ERROR;

    QVariant value;
    if (! app->control2D()->setUiWidgetValue(args[1], args[2], args[3]))
      return TCL_ERROR;

    tcl->setResult(value);
  }
  else {
    (void) th->app()->errorMsg("Invalid ui command '" + args[0] + "'");
    return TCL_ERROR;
  }

  return TCL_OK;
}

//---

void
Canvas2D::
updateStatus()
{
  std::vector<Object2D *> objects;
  getSelectedObjects(objects);

  auto str = QString("Selected: ");

  if      (objects.empty() == 1) {
    str += "None";
  }
  else if (objects.size() == 1) {
    str += objects[0]->calcId();
  }
  else {
    str += QString::number(objects.size());
  }

  app_->status()->setText(str);
}

//---

bool
Canvas2D::
runTclCmd(const QString &cmd)
{
  auto rc = tcl_->eval(cmd, /*showError*/true, /*showResult*/false);

  if (! rc)
    (void) app_->errorMsg(QString("Command '%1' failed").arg(cmd));

  return rc;
}

//---

#ifdef CQSANDBOX_CIRCLES
class CirclesMgr : public CCircleFactor::CircleMgr {
 public:
  CirclesMgr(CirclesGroupObj *group) :
   CCircleFactor::CircleMgr(), group_(group) {
  }

  void place() {
    while (int(circles_.size()) < factor()) {
      Point2D center(0.5, 0.5);
      Coord   radius(0.01);

      auto *circle = new Circle2DObj(group_->canvas(), center, radius);

      circles_.push_back(circle);

      (void) group_->canvas()->addNewObject(circle);
    }

    ind_ = 0;

    calc();
    generate(1, 1);

    while (ind_ < circles_.size()) {
      auto *circle = circles_[ind_++];

      Point2D center(0.5, 0.5);
      Coord   radius(0.000001);

      circle->setAnimating(true);
      circle->setTargetCenter(center);
      circle->setTargetRadius(radius);
    //circle->setVisible(false);

      auto f = Util::stringToReal(circle->nameValue("factor").toString());

      auto c = circleColor(f);
      c.setAlphaF(0.0);

      circle->setTargetBrush(QBrush(c));
    }
  }

  void addDrawCircle(double xc, double yc, double size, double f) override {
    assert(ind_ < circles_.size());

    auto *circle = circles_[ind_++];

    Point2D center(xc, yc);
    Coord   radius(size/2.0);

    circle->setAnimating(true);
    circle->setTargetCenter(center);
    circle->setTargetRadius(radius);
  //circle->setVisible(true);

    auto c = circleColor(f);

    circle->setNameValue("factor", f);

    circle->setTargetBrush(QBrush(c));
  }

  QColor circleColor(double f) const {
    QColor c;

    double s = 0.6;
    double v = 0.6;

    c.setHsv(int(f*360.0), int(s*255.0), int(v*255.0));

    return c;
  }

 private:
  using Circles = std::vector<Circle2DObj *>;

  CirclesGroupObj *group_ { nullptr };
  Circles          circles_;
  size_t           ind_ { 0 };
};

bool
CirclesGroupObj::
create(Canvas2D *canvas, const QStringList &args)
{
  if (args.size() != 1) return false;

  auto *tcl = canvas->tcl();

  Rect2D rect;
  if (! Util::stringToRect2D(tcl, args[0], rect))
    return false;

  auto *obj = new CirclesGroupObj(canvas, rect);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

CirclesGroupObj::
CirclesGroupObj(Canvas2D *canvas, const Rect2D &rect) :
 Group2DObj(canvas, rect)
{
  mgr_ = new CirclesMgr(this);

  mgr_->place();
}

bool
CirclesGroupObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if (name == "n")
    value = mgr_->factor();
  else
    return Group2DObj::getValue(name, args, value);

  return true;
}

bool
CirclesGroupObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if (name == "n") {
    mgr_->setFactor(Util::stringToInt(value));

    mgr_->place();
  }
  else
    return Group2DObj::setValue(name, value, args);

  return true;
}
#endif

//---

EditObj::
EditObj(Canvas2D *canvas, const QString &name) :
 Object2D(canvas, Type::EDIT), name_(name)
{
}

bool
EditObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "name")
    value = name_;
  else if (name == "proc")
    value = proc_;
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
EditObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "name")
    name_ = value;
  else if (name == "proc") {
    proc_ = value;
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

//---

bool
RealEdit::
create(Canvas2D *canvas, const QStringList &args)
{
  if (args.size() != 2) return false;

  auto *tcl = canvas->tcl();

  Point2D pos;
  if (! Util::stringToPoint2D(tcl, args[0], pos))
    return false;

  auto *obj = new RealEdit(canvas, pos, args[1]);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

RealEdit::
RealEdit(Canvas2D *canvas, const Point2D &p, const QString &name) :
 EditObj(canvas, name), p_(p)
{
}

bool
RealEdit::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "position")
    value = Util::point2DToString(p_);
  else if (name == "min_value")
    value = minValue_;
  else if (name == "max_value")
    value = maxValue_;
  else
    return EditObj::getValue(name, args, value);

  return true;
}

bool
RealEdit::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "position") {
    auto *tcl = canvas()->tcl();

    if (! Util::stringToPoint2D(tcl, value, p_))
      return false;
  }
  else if (name == "min_value") {
    minValue_ = Util::stringToReal(value);
  }
  else if (name == "max_value") {
    maxValue_ = Util::stringToReal(value);
  }
  else
    return EditObj::setValue(name, value, args);

  return true;
}

Rect2D
RealEdit::
calcRect() const
{
  QFontMetrics fm(canvas()->font());

  int w = fm.horizontalAdvance(name_) + 4;
  int h = fm.height() + 4;

  auto s = canvas()->pixelSizeToWindow(QSizeF(w, h));

  auto p = pointToWindow(p_);

  auto ll = Point2D(p.x.value            , p.y.value             );
  auto ur = Point2D(p.x.value + s.width(), p.y.value + s.height());

  return Rect2D(ll, ur);
}

void
RealEdit::
draw(QPainter *painter)
{
  auto rect  = calcRect();
  auto prect = canvas()->rectToPixel(rect).qrect();

  painter->fillRect(prect, QBrush(Qt::white));

  auto var  = canvas()->tcl()->getVar(name_);
  auto rstr = var.toString();
  auto r    = Util::stringToReal(rstr);

  painter->setPen(Qt::black);

  QFontMetrics fm(canvas()->font());

  painter->drawText(prect.left() + 2, prect.top() + fm.ascent() + 2, Util::realToString(r));
}

void
RealEdit::
move(int dx, int)
{
  auto *tcl = canvas()->tcl();

  double d1 = double(dx)/double(canvas()->width());
  double d = d1*(maxValue_ - minValue_);

  auto var  = tcl->getVar(name_);
  auto rstr = var.toString();
  auto r    = Util::stringToReal(rstr);

  r += d;

  r = std::min(std::max(r, minValue_), maxValue_);

  tcl->setVar(name_, r);

  canvas()->step();

  if (proc_ != "")
    canvas()->runTclCmd(proc_);
}

//---

bool
IntegerEdit::
create(Canvas2D *canvas, const QStringList &args)
{
  if (args.size() != 2) return false;

  auto *tcl = canvas->tcl();

  Point2D pos;
  if (! Util::stringToPoint2D(tcl, args[0], pos))
    return false;

  auto *obj = new IntegerEdit(canvas, pos, args[1]);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

IntegerEdit::
IntegerEdit(Canvas2D *canvas, const Point2D &p, const QString &name) :
 EditObj(canvas, name), p_(p)
{
}

bool
IntegerEdit::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "position")
    value = Util::point2DToString(p_);
  else if (name == "min_value")
    value = minValue_;
  else if (name == "max_value")
    value = maxValue_;
  else
    return EditObj::getValue(name, args, value);

  return true;
}

bool
IntegerEdit::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "position") {
    auto *tcl = canvas()->tcl();

    if (! Util::stringToPoint2D(tcl, value, p_))
      return false;
  }
  else if (name == "min_value") {
    minValue_ = Util::stringToInt(value);
  }
  else if (name == "max_value") {
    maxValue_ = Util::stringToInt(value);
  }
  else
    return EditObj::setValue(name, value, args);

  return true;
}

Rect2D
IntegerEdit::
calcRect() const
{
  QFontMetrics fm(canvas()->font());

  int w = fm.horizontalAdvance("XXXXXX") + 2*fm.height() + 8;
  int h = fm.height() + 4;

  auto s = canvas()->pixelSizeToWindow(QSizeF(w, h));

  auto p = pointToWindow(p_);

  auto ll = Point2D(p.x.value            , p.y.value             );
  auto ur = Point2D(p.x.value + s.width(), p.y.value + s.height());

  return Rect2D(ll, ur);
}

void
IntegerEdit::
draw(QPainter *painter)
{
  auto rect  = calcRect();
  auto prect = canvas()->rectToPixel(rect).qrect();

  painter->fillRect(prect, QBrush(Qt::white));

  auto var  = canvas()->tcl()->getVar(name_);
  auto istr = var.toString();
  auto i    = Util::stringToInt(istr);

  painter->setPen(Qt::black);

  QFontMetrics fm(canvas()->font());

  int x1 = prect.left () + 2;
  int x2 = x1 + fm.height();
  int x4 = prect.right() - 2;
  int x3 = x4 - fm.height();
  int y1 = prect.top() + 2;
  int y2 = prect.bottom() - 2;
  int ym = prect.center().y();

  auto drawTriangle = [&](const QPoint &p1, const QPoint &p2, const QPoint &p3) {
    painter->setBrush(Qt::black);

    QPainterPath path;

    path.moveTo(p1);
    path.lineTo(p2);
    path.lineTo(p3);
    path.closeSubpath();

    painter->drawPath(path);
  };

  drawTriangle(QPoint(x1, ym), QPoint(x2, y1), QPoint(x2, y2));
  drawTriangle(QPoint(x4, ym), QPoint(x3, y1), QPoint(x3, y2));

  lrect_ = QRect(x1, y1, x2 - x1, y2 - y1);
  rrect_ = QRect(x3, y1, x4 - x3, y2 - y1);

  painter->drawText(x2 + 2, ym + (fm.ascent() - fm.descent())/2, Util::intToString(i));
}

void
IntegerEdit::
move(int dx, int)
{
  double d1 = double(dx)/double(canvas()->width());
  double d = d1*(maxValue_ - minValue_);

  auto i = int(std::round(getIValue() + d));

  setIValue(i);

  canvas()->step();
}

void
IntegerEdit::
click(int x, int y)
{
  if      (lrect_.contains(QPoint(x, y)))
    setIValue(getIValue() - 1);
  else if (rrect_.contains(QPoint(x, y)))
    setIValue(getIValue() + 1);
}

int
IntegerEdit::
getIValue() const
{
  auto *tcl = canvas()->tcl();

  auto var  = tcl->getVar(name_);
  auto istr = var.toString();

  return Util::stringToInt(istr);
}

void
IntegerEdit::
setIValue(int i)
{
  int i1 = std::min(std::max(i, minValue_), maxValue_);

  auto *tcl = canvas()->tcl();

  tcl->setVar(name_, i1);

  if (proc_ != "")
    canvas()->runTclCmd(proc_);
}

//---

bool
ButtonObj::
create(Canvas2D *canvas, const QStringList &args)
{
  auto *tcl = canvas->tcl();

  Point2D pos;
  QString text;

  if      (args.size() >= 2) {
    if (! Util::stringToPoint2D(tcl, args[0], pos))
      return false;

    text = args[1];
  }
  else if (args.size() >= 1) {
    text = args[1];
  }

  auto *obj = new ButtonObj(canvas, pos, text);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

ButtonObj::
ButtonObj(Canvas2D *canvas, const Point2D &p, const QString &name) :
 Object2D(canvas, Type::BUTTON), p_(p), name_(name)
{
}

bool
ButtonObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "position")
    value = Util::point2DToString(p_);
  else if (name == "name")
    value = name_;
  else if (name == "proc")
    value = proc_;
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
ButtonObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "position") {
    auto *tcl = canvas()->tcl();

    if (! Util::stringToPoint2D(tcl, value, p_))
      return false;
  }
  else if (name == "name")
    name_ = value;
  else if (name == "proc")
    proc_ = value;
  else
    return Object2D::setValue(name, value, args);

  return true;
}

Rect2D
ButtonObj::
calcRect() const
{
  QFontMetrics fm(canvas()->font());

  int w = fm.horizontalAdvance(name_) + 4;
  int h = fm.height() + 4;

  auto s = canvas()->pixelSizeToWindow(QSizeF(w, h));

  auto p = pointToWindow(p_);

  auto ll = Point2D(p.x.value            , p.y.value             );
  auto ur = Point2D(p.x.value + s.width(), p.y.value + s.height());

  return Rect2D(ll, ur);
}

void
ButtonObj::
draw(QPainter *painter)
{
  auto rect  = calcRect();
  auto prect = canvas()->rectToPixel(rect).qrect();

  painter->fillRect(prect, QBrush(Qt::white));

  painter->setPen(Qt::black);

  QFontMetrics fm(canvas()->font());

  painter->drawText(prect.left() + 2, prect.top() + fm.ascent() + 2, name_);
}

void
ButtonObj::
click(int, int)
{
  if (proc_ != "")
    canvas()->runTclCmd(proc_);
}

//---

}
