#include <CQSandboxCanvas.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>
#include <CQSandboxControl2D.h>
#include <CQSandboxViewport.h>
#include <CQSandboxToolbar2D.h>

#include <CQSVGUtil.h>
#include <CQTclUtil.h>
#include <CQUtil.h>

#include <CQHtmlTextPainter.h>
#include <CQArrow.h>
#include <CQAxis.h>
#include <CQCsvModel.h>
#include <CCircleFactor.h>
#include <CFile.h>

#include <QFile>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

namespace CQSandbox {

template<typename T>
int createObjectProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv) {
  auto *th = static_cast<Canvas *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);

  if (! T::create(th, args))
    return TCL_ERROR;

  return TCL_OK;
}

QString
coordToString(const Coord &coord) {
  auto str = QString::number(coord.value);

  if (coord.units == Units::PIXEL)
    str += "px";

  return str;
}

Coord
stringToCoord(const QString &str) {
  Coord coord;

  auto str1 = str;

  if (str1.right(2) == "px") {
    coord.units = Units::PIXEL;

    str1 = str1.mid(0, str1.length() - 2);
  }

  coord.value = Util::stringToReal(str1);

  return coord;
}

QString
rectToString(const Rect &r) {
  auto x1str = QString::number(r.ll.x.value);
  auto y1str = QString::number(r.ll.y.value);
  auto x2str = QString::number(r.ur.x.value);
  auto y2str = QString::number(r.ur.y.value);

  auto str = x1str + " " + y1str + " " + x2str + " " + y2str;

  if (r.ll.x.units == Units::PIXEL)
    str += " px";

  return str;
}

QString
pointToString(const Point &p) {
  auto xstr = QString::number(p.x.value);
  auto ystr = QString::number(p.y.value);

  auto str = xstr + " " + ystr;

  if (p.x.units == Units::PIXEL)
    str += " px";

  return str;
}

bool
stringToIntArray(CQTcl *tcl, const QString &str, std::vector<int> &a) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  a.resize(strs.size());

  for (int i = 0; i < strs.size(); ++i) {
    int ii;
    if (! Util::stringToInt(strs[i], ii))
      return false;

    a[i] = ii;
  }

  return true;
}

Point
stringToPoint(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  Point p;

  if (strs.size() > 2 && strs[2] == "px") {
    p.x.units = Units::PIXEL;
    p.y.units = Units::PIXEL;
  }

  if (strs.size() >= 2) {
    auto x = Util::stringToReal(strs[0]);
    auto y = Util::stringToReal(strs[1]);

    p.x.value = x;
    p.y.value = y;
  }

  return p;
}

Rect
stringToRect(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  Point ll, ur;

  if (strs.size() > 4 && strs[2] == "px") {
    ll.x.units = Units::PIXEL;
    ll.y.units = Units::PIXEL;
    ur.x.units = Units::PIXEL;
    ur.y.units = Units::PIXEL;
  }

  if (strs.size() >= 4) {
    auto x1 = Util::stringToReal(strs[0]);
    auto y1 = Util::stringToReal(strs[1]);
    auto x2 = Util::stringToReal(strs[2]);
    auto y2 = Util::stringToReal(strs[3]);

    ll.x.value = std::min(x1, x2);
    ll.y.value = std::min(y1, y2);
    ur.x.value = std::max(x1, x2);
    ur.y.value = std::max(y1, y2);
  }

  Rect rect;

  rect.ll = ll;
  rect.ur = ur;

  return rect;
}

QString
rangeToString(CQTcl *tcl, const CDisplayRange2D &range) {
  double x1, y1, x2, y2;
  range.getWindowRange(&x1, &y1, &x2, &y2);

  QStringList strs;
  strs << Util::realToString(x1);
  strs << Util::realToString(y1);
  strs << Util::realToString(x2);
  strs << Util::realToString(y2);

  return tcl->mergeList(strs);
}

void
stringToRange(CQTcl *tcl, CDisplayRange2D &range, const QString &str)
{
  QStringList strs;
  (void) tcl->splitList(str, strs);

  if (strs.size() == 4) {
    auto x1 = Util::stringToReal(strs[0]);
    auto y1 = Util::stringToReal(strs[1]);
    auto x2 = Util::stringToReal(strs[2]);
    auto y2 = Util::stringToReal(strs[3]);

    range.setWindowRange(x1, y1, x2, y2);
  }
}

bool
stringToImage(const QString &str, QImage &image) {
  auto cstr = str.toStdString();

  CFile file(cstr);

  if (! file.exists())
    return false;

  image = QImage(str);

  image.setText("name", str);

  return true;
}

QString
imageToString(const QImage &image) {
  return image.text("name");
}

QPainterPath
stringToPath(const QString &str) {
  QPainterPath path;
  (void) CQSVGUtil::stringToPath(str, path);
  return path;
}

QString
pathToString(const QPainterPath &path) {
  return CQSVGUtil::pathToString(path);
}

QString
alignToString(Qt::Alignment align)
{
  QString str;

  if (align == Qt::AlignCenter)
    return "Center";

  if      (align & Qt::AlignLeft   ) str += "Left";
  else if (align & Qt::AlignRight  ) str += "Right";
  else if (align & Qt::AlignHCenter) str += "HCenter";

  if      (align & Qt::AlignTop    ) str += "Top";
  else if (align & Qt::AlignBottom ) str += "Bottom";
  else if (align & Qt::AlignVCenter) str += "VCenter";

  return str;
}

Qt::Alignment
stringToAlign(const QString &str)
{
  Qt::Alignment align = Qt::AlignmentFlag(0);

  auto str1 = str.toLower();

  if (str1 == "center")
    return Qt::AlignCenter;

  if      (str1.left(4) == "left"   ) align |= Qt::AlignLeft;
  else if (str1.left(5) == "right"  ) align |= Qt::AlignRight;
  else if (str1.left(7) == "hcenter") align |= Qt::AlignHCenter;
  else                                align |= Qt::AlignHCenter;

  if      (str1.right(3) == "top"    ) align |= Qt::AlignTop;
  else if (str1.right(6) == "bottom" ) align |= Qt::AlignBottom;
  else if (str1.right(7) == "vcenter") align |= Qt::AlignVCenter;
  else                                 align |= Qt::AlignVCenter;

  return align;
}

QVector<qreal>
stringToDashes(CQTcl *tcl, const QString &str)
{
  QStringList strs;
  (void) tcl->splitList(str, strs);

  QVector<qreal> dashes;
  for (const auto &str : strs) {
    auto r = Util::stringToReal(str);
    dashes << r;
  }
  return dashes;
}

QColor RGBAToQColor(const CRGBA &c) {
  return QColor(c.getRed()*255, c.getGreen()*255, c.getBlue()*255, c.getAlpha()*255);
}

CRGBA QColorToRGBA(const QColor &c) {
  return CRGBA(c.redF(), c.greenF(), c.blueF(), c.alphaF());
}

}

//---

namespace CQSandbox {

Canvas::
Canvas(App *app) :
 QFrame(app), app_(app)
{
  setFocusPolicy(Qt::StrongFocus);

  setMouseTracking(true);

  psys_ = new ParticleSystem;
}

void
Canvas::
init()
{
  if (initialized_)
    return;

  initialized_ = true;

  addCommands();

  app_->runTclCmd("proc init { args } { }");
  app_->runTclCmd("proc resize { args } { }");
  app_->runTclCmd("proc update { args } { }");
  app_->runTclCmd("proc drawBg { args } { }");
  app_->runTclCmd("proc drawFg { args } { }");
  app_->runTclCmd("proc keyPress { args } { }");
  app_->runTclCmd("proc mousePress { args } { }");
  app_->runTclCmd("proc mouseMove { args } { }");
  app_->runTclCmd("proc mouseRelease { args } { }");

  //---

  addViewport();

  //---

  timer_ = new QTimer;
  connect(timer_, &QTimer::timeout, this, &Canvas::timerSlot);

  stepTimer_ = new QTimer;
  stepTimer_->setSingleShot(true);
  connect(stepTimer_, &QTimer::timeout, this, &Canvas::stepTimerSlot);

  drawTimer_ = new QTimer;
  connect(drawTimer_, &QTimer::timeout, this, &Canvas::drawTimerSlot);

  //---

  stylePen_   = QPen(Qt::black);
  styleBrush_ = QBrush(Qt::white);
}

void
Canvas::
addCommands()
{
  auto *tcl = app_->tcl();

  tcl->createAlias("echo", "puts");

  // global
  tcl->createObjCommand("sb::canvas",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas::canvasProc),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::palette",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas::paletteProc),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::style",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas::styleProc),
    static_cast<CQTcl::ObjCmdData>(this));

  // viewport
  tcl->createObjCommand("sb::viewport",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas::viewportProc),
    static_cast<CQTcl::ObjCmdData>(this));

  // objects
  tcl->createObjCommand("sb::group",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<GroupObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::circle",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<CircleObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::rect",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<RectObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::text",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<TextObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::line",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<LineObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::image",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<ImageObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::path",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<PathObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::point_list",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<PointListObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::arrow",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<ArrowObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::axis",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<AxisObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::particle",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<ParticleObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  //---

  // data
  tcl->createObjCommand("sb::vector",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<VectorObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::array",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<ArrayObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::csv",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<CsvObj>),
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

  //---

  // graphics
  tcl->createObjCommand("sb::renderer",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<RendererObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::draw_point",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas::drawPointProc),
    static_cast<CQTcl::ObjCmdData>(this));

  //---

  // layout
  tcl->createObjCommand("sb::circles_group",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<CirclesGroupObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::quad_tree",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<QuadTreeObj>),
    static_cast<CQTcl::ObjCmdData>(this));

  // math
  tcl->createObjCommand("sb::fmul",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas::fmulProc),
    static_cast<CQTcl::ObjCmdData>(this));
  tcl->createObjCommand("sb::fma", // fused multiply and add (A*B) + C
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas::fmaProc),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb::hypot",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas::hypotProc),
    static_cast<CQTcl::ObjCmdData>(this));

  //---

  tcl->createObjCommand("sb::ui",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas::uiProc),
    static_cast<CQTcl::ObjCmdData>(this));
}

void
Canvas::
createObjCommand(Object *obj)
{
  auto *tcl = app_->tcl();

  auto name = obj->getCommandName();

  tcl->createObjCommand(name,
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas::objectCommandProc),
    static_cast<CQTcl::ObjCmdData>(obj));
}

Rect
Canvas::
rectToPixel(const Rect &rect) const
{
  auto p1 = pointToPixel(rect.ll);
  auto p2 = pointToPixel(rect.ur);

  return Rect(p1, p2);
}

Point
Canvas::
pointToPixel(const Point &p) const
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

  return Point::makePixel(px, py);
}

Point
Canvas::
pointToWindow(const Point &p) const
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

  return Point::makeWindow(x, y);
}

QSizeF
Canvas::
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
Canvas::
play()
{
  step();

  timer_->start(timerTicks_);

  running_ = true;
}

void
Canvas::
pause()
{
  timer_->stop();

  running_ = false;
}

void
Canvas::
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

  app_->runTclCmd("update");

  //---

  if (buffered_) {
    if (drawing_)
      return;

    drawBufferedNeeded_ = true;
  }
}

void
Canvas::
stepInit(bool &buffered)
{
  buffered = false;

  if (! inited_) {
    app_->control2D()->setActive(false);

    inited_ = true;

    app_->runTclCmd("init");

    initRun_ = true;

    app_->control2D()->setActive(true);

    buffered = buffered_;

    drawTimer_->start(100);
  }
}

void
Canvas::
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
Canvas::
timerSlot()
{
  step();
}

void
Canvas::
stepTimerSlot()
{
  step();
}

void
Canvas::
drawTimerSlot()
{
  if (drawBufferedNeeded_) {
    drawBufferedNeeded_ = false;

    drawBuffered();
  }
}

void
Canvas::
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
Canvas::
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
    app_->runTclCmd(QString("resize %1 %2").arg(pixelWidth_).arg(pixelHeight_));

  if (buffered_ && initRun_)
    drawBufferedNeeded_ = true;

  if (running)
    play();
}

void
Canvas::
updatePixelRanges()
{
  auto mapX = [&](double x) { return x*(width () - 1); };
  auto mapY = [&](double y) { return y*(height() - 1); };

  for (auto *viewport : viewports_) {
    auto x1 = mapX(viewport->rect.ll.x.value);
    auto y1 = mapY(viewport->rect.ll.y.value);
    auto x2 = mapX(viewport->rect.ur.x.value);
    auto y2 = mapY(viewport->rect.ur.y.value);

    viewport->displayRange.setPixelRange(x1, y1, x2, y2);
  }
}

void
Canvas::
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
Canvas::
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
    currentViewport_ = viewport;

    double x1, y1, x2, y2;
    viewport->displayRange.getPixelRange(&x1, &y1, &x2, &y2);

    if (y1 > y2) std::swap(y1, y2);

    auto rect = QRectF(x1, y1, x2 - x1, y2 - y1);

    painter_->setClipRect(rect);

    painter_->fillRect(rect, viewport->brush.value().color());

    app_->runTclCmd("drawBg");

    for (auto *obj : viewport->objects) {
      if (obj->isVisible())
        obj->draw(painter_);
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

    app_->runTclCmd("drawFg");

    currentViewport_ = nullptr;
  }

  drawing_ = false;
}

void
Canvas::
drawParticle(QPainter *painter, Particle *particle)
{
  auto *obj = dynamic_cast<const ParticleObj *>(particle->obj());

  painter->setPen(obj->pen());

  auto *position = particle->position();

  auto p = Point(position->x(), position->y());

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
      auto fg = RGBAToQColor(particle->color().value());

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
Canvas::
mousePressEvent(QMouseEvent *e)
{
  pressPos_  = e->pos();
  pressObj_  = getObjectAtPos(pressPos_);
  motionPos_ = pressPos_;

  if (pressObj_)
    pressObj_->press(e->x(), e->y());

  pressed_ = true;

  auto p = pointToWindow(Point::makePixel(e->x(), e->y())).qpoint();

  app_->runTclCmd(QString("mousePress %1 %2").arg(p.x()).arg(p.y()));

  update();
}

void
Canvas::
mouseMoveEvent(QMouseEvent *e)
{
  auto p = pointToWindow(Point::makePixel(e->pos())).qpoint();

  if (pressObj_) {
    auto dx = e->x() - motionPos_.x();
    auto dy = e->y() - motionPos_.y();

    pressObj_->move(dx, dy);

    motionPos_ = e->pos();
  }
  else {
    auto *group = dynamic_cast<GroupObj *>(getObjectAtPos(e->pos()));

    QString name;

    if (group) {
      auto p1 = pointToWindow(Point::makePixel(e->pos())).qpoint();

      auto groupRange = group->displayRange();

      double x, y;
      groupRange.pixelToWindow(p1.x(), p1.y(), &x, &y);

      p    = QPointF(x, y);
      name = group->calcId();
    }
    else {
      name = "canvas";
    }

    app_->setInfo(QString("%1: %2 %3").arg(name).arg(p.x()).arg(p.y()));
  }

  if (pressed_)
    app_->runTclCmd(QString("mouseMove %1 %2").arg(p.x()).arg(p.y()));

  update();
}

void
Canvas::
mouseReleaseEvent(QMouseEvent *e)
{
  if (pressObj_) {
    auto *releaseObj = getObjectAtPos(e->pos());

    if (pressObj_ == releaseObj)
      pressObj_->click(pressPos_.x(), pressPos_.y());
  }

  pressObj_ = nullptr;

  pressed_ = false;

  auto p = pointToWindow(Point::makePixel(e->x(), e->y())).qpoint();

  app_->runTclCmd(QString("mouseRelease %1 %2").arg(p.x()).arg(p.y()));

  update();
}

void
Canvas::
keyPressEvent(QKeyEvent *e)
{
  auto keyStr = getKeyString(e);

  keyPressed_[keyStr] = true;

//bool isControl = (e->modifiers() & Qt::ControlModifier);
//bool isShift   = (e->modifiers() & Qt::ShiftModifier);

  //---

  app_->runTclCmd(QString("keyPress {%1}").arg(keyStr));

  update();

  return;
}

void
Canvas::
keyReleaseEvent(QKeyEvent *e)
{
  auto keyStr = getKeyString(e);

  keyPressed_[keyStr] = false;
}

bool
Canvas::
getKeyPressed(const QString &key) const
{
  auto p = keyPressed_.find(key);

  if (p == keyPressed_.end())
    return false;

  return (*p).second;
}

QString
Canvas::
getKeyString(QKeyEvent *e) const
{
  QString keyStr;

  if      (e->key() == Qt::Key_Left ) keyStr = "left";
  else if (e->key() == Qt::Key_Right) keyStr = "right";
  else if (e->key() == Qt::Key_Up   ) keyStr = "up";
  else if (e->key() == Qt::Key_Down ) keyStr = "down";
  else if (e->key() == Qt::Key_Space) keyStr = "space";
  else                                keyStr = e->text();

  if (keyStr == "")
    keyStr = QString("key.%1").arg(e->key());

  return keyStr;
}

Object *
Canvas::
getObjectAtPos(const QPoint &pos) const
{
  for (auto *viewport : viewports_) {
    for (auto *obj : viewport->objects) {
      auto rect  = obj->calcRect();
      auto prect = rectToPixel(rect).qrect();

      if (prect.contains(pos))
        return obj;
    }
  }

  return nullptr;
}

Object *
Canvas::
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
Canvas::
addNewObject(Object *obj)
{
  addObject(obj);

  allObjects_.push_back(obj);

  obj->setInd(++lastInd_);

  createObjCommand(obj);

  return obj->calcId();
}

void
Canvas::
addObject(Object *obj)
{
  auto *viewport = currentViewport();

  viewport->objects.push_back(obj);

  obj->setGroup(nullptr);

  Q_EMIT objectsChanged();
}

void
Canvas::
removeObject(Object *obj)
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

int
Canvas::
canvasProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);
  if (args.size() < 1) return TCL_ERROR;

  auto *tcl = th->app_->tcl();

  if      (args[0] == "get") {
    if (args.size() < 2) {
      th->app_->errorMsg("Missing args for get");
      return TCL_ERROR;
    }

    QStringList args1;
    for (int i = 2; i < args.size(); ++i)
      args1.push_back(args[i]);

    auto res = th->getValue(args[1], args1);

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
Canvas::
paletteProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);
  if (args.size() < 1) return TCL_ERROR;

  auto *tcl = th->app_->tcl();

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
Canvas::
viewportProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);
  if (args.size() < 1) return TCL_ERROR;

  auto *tcl = th->app_->tcl();

  auto rect = stringToRect(tcl, args[0]);

  auto *viewport = th->addViewport();

  viewport->rect = rect;

  tcl->setResult(viewport->name);

  return TCL_OK;
}

int
Canvas::
styleProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);
  if (args.size() < 1) return TCL_ERROR;

  auto *tcl = th->app_->tcl();

  if (args[0] == "get") {
    if (args.size() >= 2)
      tcl->setResult(th->getStyleValue(args[1]));
  }
  else if (args[0] == "set") {
    if (args.size() >= 3)
      th->setStyleValue(args[1], args[2]);
  }
  else
    return TCL_ERROR;

  return TCL_OK;
}

QVariant
Canvas::
getValue(const QString &name, const QStringList &args)
{
  auto *tcl = app_->tcl();

  auto *viewport = currentViewport();

  if      (name == "brush.color") {
    return Util::colorToString(viewport->brush.value().color());
  }
  else if (name == "brush.color.target") {
    return Util::colorToString(viewport->brush.target().color());
  }
  else if (name == "brush.steps") {
    return Util::colorToString(viewport->brush.steps());
  }
  else if (name == "pen.color") {
    return Util::colorToString(viewport->pen.color());
  }
  else if (name == "pen.width") {
    return Util::realToString(viewport->pen.widthF());
  }
  else if (name == "range") {
    return rangeToString(tcl, viewport->displayRange);
  }
  else if (name == "range.xmin") {
    double x1, y1, x2, y2;
    viewport->displayRange.getWindowRange(&x1, &y1, &x2, &y2);
    return x1;
  }
  else if (name == "range.ymin") {
    double x1, y1, x2, y2;
    viewport->displayRange.getWindowRange(&x1, &y1, &x2, &y2);
    return y1;
  }
  else if (name == "range.xmax") {
    double x1, y1, x2, y2;
    viewport->displayRange.getWindowRange(&x1, &y1, &x2, &y2);
    return x2;
  }
  else if (name == "range.ymax") {
    double x1, y1, x2, y2;
    viewport->displayRange.getWindowRange(&x1, &y1, &x2, &y2);
    return y2;
  }
  else if (name == "equal_scale") {
    auto b = viewport->displayRange.getEqualScale();

    return Util::boolToString(b);
  }
  else if (name == "particles") {
    QStringList ids;

    const auto &particles = psys_->getParticles();

    for (uint i = 0; i < particles.size(); ++i) {
      auto *particle = particles.get(int(i));

      auto *particle1 = dynamic_cast<Particle *>(particle);

      ids.push_back(particle1->obj()->getCommandName());
    }

    return ids;
  }
  else if (name == "ticks") {
    return Util::intToString(ticks_);
  }
  else if (name == "key") {
    if (args.size() < 1) {
      app_->errorMsg(QString("Invalid value name '%1'").arg(name));
      return QVariant();
    }

    return getKeyPressed(args[0]);
  }
  else if (name == "play") {
    return running_;
  }
  else if (name == "buffered") {
    return buffered_;
  }
  else if (name == "pixel_width") {
    return pixelWidth_;
  }
  else if (name == "pixel_height") {
    return pixelHeight_;
  }
  else if (name == "font.height") {
    QFontMetrics fm(font());

    return fm.height();
  }
  else {
    app_->errorMsg(QString("Invalid value name '%1'").arg(name));
    return QVariant();
  }
}

bool
Canvas::
setValue(const QString &name, const QString &value, const QStringList &)
{
  auto *tcl = app_->tcl();

  auto *viewport = currentViewport();

  if      (name == "brush.color") {
    auto b = viewport->brush.value();

    b.setColor(Util::stringToColor(tcl, value));

    viewport->brush = b;
  }
  else if (name == "brush.color.target") {
    auto b = viewport->brush.target();

    b.setColor(Util::stringToColor(tcl, value));

    viewport->brush.setTarget(b);
  }
  else if (name == "brush.steps")
    viewport->brush.setSteps(Util::stringToInt(value));
  else if (name == "pen.color")
    viewport->pen.setColor(Util::stringToColor(tcl, value));
  else if (name == "pen.width")
    viewport->pen.setWidthF(Util::stringToReal(value));
  else if (name == "range") {
    stringToRange(tcl, viewport->displayRange, value);

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
    viewport->rect = stringToRect(tcl, value);

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
    auto size = stringToPoint(tcl, value);

    int w = size.x.value;
    int h = size.y.value;

    h += app_->toolbar2D()->height();

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

    app_->toolbar2D()->showControls(b);
  }
  else
    return app_->errorMsg(QString("Invalid value name '%1'").arg(name));

  return true;
}

bool
Canvas::
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
Canvas::
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
Canvas::
addViewport()
{
  auto *viewport = new Viewport;

  viewport->canvas = this;

  viewport->name = QString("sb::viewport_obj.%1").arg(viewports_.size() + 1);

  viewport->rect = Rect(Point(0, 0), Point(1, 1));

  viewport->pen   = QPen(Qt::black);
  viewport->brush = QBrush(Qt::white);

  viewports_.push_back(viewport);

  updatePixelRanges();

  auto *tcl = app_->tcl();

  tcl->createObjCommand(viewport->name,
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas::viewportCommandProc),
    static_cast<CQTcl::ObjCmdData>(viewport));

  if (currentViewportName_ == "")
    currentViewportName_ = viewport->name;

  return viewport;
}

QVariant
Canvas::
getPaletteValue(const QString &)
{
  return QVariant();
}

void
Canvas::
setPaletteValue(const QString &name, const QString &)
{
  app_->errorMsg(QString("Invalid value name '%1'").arg(name));
}

QVariant
Canvas::
getStyleValue(const QString &name)
{
  if      (name == "pen.color")
    return Util::colorToString(stylePen_.color());
  else if (name == "pen.width")
    return Util::realToString(stylePen_.widthF());
  else {
    app_->errorMsg(QString("Invalid value name '%1'").arg(name));
    return QVariant();
  }
}

void
Canvas::
setStyleValue(const QString &name, const QString &value)
{
  auto *tcl = app_->tcl();

  if      (name == "brush.color")
    styleBrush_.setColor(Util::stringToColor(tcl, value));
  else if (name == "pen.color")
    stylePen_.setColor(Util::stringToColor(tcl, value));
  else if (name == "pen.width")
    stylePen_.setWidthF(Util::stringToReal(value));
}

int
Canvas::
drawPointProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas *>(clientData);
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

int
Canvas::
objectCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *obj = static_cast<Object *>(clientData);
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

      QVariant res;

      if (! obj->exec(op, args1, res))
        return TCL_ERROR;

      tcl->setResult(res);
    }
    else {
      app->errorMsg("Missing args for exec");
    }
  }
  else if (args[0] == "delete") {
    canvas->removeObject(obj);

    delete obj;
  }
  else {
    app->errorMsg(QString("Bad object command '%1'").arg(args[0]));
  }

  return TCL_OK;
}

int
Canvas::
viewportCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *viewport = static_cast<Viewport *>(clientData);
  assert(viewport);

  auto *canvas = viewport->canvas;
  auto *app    = canvas->app();

  auto *tcl = app->tcl();

  auto args = app->getArgs(objc, objv);

  if      (args[0] == "get") {
    if (args.size() > 1) {
      auto name = args[1];

      app->errorMsg("Invalid get name '" + name + "'");
    }
    else {
      app->errorMsg("Missing args for viewport get");
    }
  }
  else if (args[0] == "set") {
    if (args.size() > 2) {
      auto name  = args[1];
      auto value = args[2];

      if      (name == "brush.color") {
        auto b = viewport->brush.value();

        b.setColor(Util::stringToColor(tcl, value));

        viewport->brush = b;
      }
      else if (name == "range") {
        stringToRange(tcl, viewport->displayRange, value);

        viewport->hasRange = true;
      }
      else if (name == "clip") {
        viewport->clip = stringToRect(tcl, value);
      }
      else
        app->errorMsg("Invalid set name '" + name + "' for viewport");
    }
    else {
      app->errorMsg("Missing args for viewport set");
    }
  }
  else if (args[0] == "exec") {
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
  else if (args[0] == "delete") {
    app->errorMsg("Invalid viewport delete");
  }
  else {
    app->errorMsg(QString("Bad viewport command '%1'").arg(args[0]));
  }

  return TCL_OK;
}

int
Canvas::
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
Canvas::
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

int
Canvas::
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

//---

int
Canvas::
uiProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas *>(clientData);
  assert(th);

  auto *app = th->app();

  auto args = app->getArgs(objc, objv);
  if (args.size() != 1) return TCL_ERROR;

  if (! app->control2D()->setUi(args[0]))
    return TCL_ERROR;

  return TCL_OK;
}

//---

bool
GroupObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 1) return false;

  auto *tcl = canvas->app()->tcl();

  auto rect = stringToRect(tcl, args[0]);

  auto *obj = new GroupObj(canvas, rect);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

GroupObj::
GroupObj(Canvas *canvas, const Rect &rect) :
 Object(canvas), rect_(rect)
{
}

QVariant
GroupObj::
getValue(const QString &name, const QStringList &args)
{
  auto *tcl = canvas()->app()->tcl();

  if      (name == "rect")
    return rectToString(calcRect());
  else if (name == "range")
    return rangeToString(tcl, displayRange_);
  else
    return Object::getValue(name, args);
}

bool
GroupObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->app()->tcl();

  if      (name == "rect")
    rect_ = stringToRect(tcl, value);
  else if (name == "range")
    stringToRange(tcl, displayRange_, value);
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
GroupObj::
calcRect() const
{
  return rect_;
}

void
GroupObj::
draw(QPainter *painter)
{
  auto rect  = rectToWindow(rect_);
  auto prect = canvas()->rectToPixel(rect).qrect();

  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  painter->drawRect(prect);

  painter->save();
  painter->setClipRect(prect);

  auto qrect = rect.qrect();

  displayRange_.setPixelRange(qrect.left(), qrect.bottom(), qrect.right(), qrect.top());

  for (auto *obj : objects_) {
    if (obj->isVisible())
      obj->draw(painter);
  }

  painter->restore();
}

void
GroupObj::
addObject(Object *obj)
{
  objects_.push_back(obj);

  obj->setGroup(this);

  Q_EMIT objectsChanged();
}

void
GroupObj::
removeObject(Object *obj)
{
  Objects objects;

  for (auto *obj1 : objects_) {
    if (obj1 != obj)
      objects.push_back(obj1);
  }

  std::swap(objects, objects_);

  Q_EMIT objectsChanged();
}

Rect
GroupObj::
rectToPixel(const Rect &r) const
{
  auto p1 = pointToPixel(r.ll);
  auto p2 = pointToPixel(r.ur);

  return Rect(p1, p2);
}

Point
GroupObj::
pointToPixel(const Point &p) const
{
  if (p.x.units == Units::PIXEL)
    return p;

  double px, py;
  displayRange_.windowToPixel(p.x.value, p.y.value, &px, &py);

  auto p1 = Point::makePixel(px, py);

  if (group_)
    return group_->pointToPixel(p1);
  else
    return canvas_->pointToPixel(p1);
}

//---

bool
RendererObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 0)
    return false;

  auto *tcl = canvas->app()->tcl();

  auto *obj = new RendererObj(canvas);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

RendererObj::
RendererObj(Canvas *canvas) :
 Object(canvas)
{
  font_ = canvas->font();
}

QVariant
RendererObj::
getValue(const QString &name, const QStringList &args)
{
  //auto *tcl = canvas()->app()->tcl();

  if      (name == "brush.color") {
    return Util::colorToString(brush_.color());
  }
  else if (name == "pen.color") {
    return Util::colorToString(pen_.color());
  }
  else if (name == "font.height") {
    QFontMetrics fm(font_);

    return fm.height();
  }
  else
    return Object::getValue(name, args);
}

bool
RendererObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  if      (name == "brush.color")
    brush_.setColor(Util::stringToColor(tcl, value));
  else if (name == "pen.color")
    pen_.setColor(Util::stringToColor(tcl, value));
  else if (name == "font.size") {
    double s = Util::stringToReal(value);

    auto font = font_;

    double scale = 1;

    for (int i = 0; i < 8; ++i) {
      font.setPointSizeF(scale*s);

      QFontMetricsF fm(font);

      double s1 = fm.height();

      scale *= s/s1;
    }

    font_ = font;
  }
  else
    return Object::setValue(name, value, args);

  return true;
}

bool
RendererObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  if      (op == "draw.point") {
    if (args.size() != 1)
      return false;

    auto *painter = canvas()->painter();
    if (! painter) return false;

    auto p = stringToPoint(tcl, args[0]);

    painter->setPen(pen_);

    auto pp = pointToPixel(p);

    painter->drawPoint(pp.x.value, pp.y.value);

    return true;
  }
  else if (op == "draw.rect") {
    if (args.size() != 1)
      return false;

    auto *painter = canvas()->painter();
    if (! painter) return false;

    auto r = stringToRect(tcl, args[0]);

    painter->setPen(pen_);

    auto pr = canvas()->rectToPixel(r).qrect();;

    painter->drawRect(pr);

    return true;
  }
  else if (op == "draw.text") {
    if (args.size() != 2)
      return false;

    auto *painter = canvas()->painter();
    if (! painter) return false;

    auto p    = stringToPoint(tcl, args[0]);
    auto text = args[1];

    painter->setPen(pen_);

    auto pp = pointToPixel(p);

    painter->setFont(font_);

    painter->drawText(pp.x.value, pp.y.value, text);

    return true;
  }
  else
    return Object::exec(op, args, res);

  return false;
}

//---

class CirclesMgr : public CCircleFactor::CircleMgr {
 public:
  CirclesMgr(CirclesGroupObj *group) :
   CCircleFactor::CircleMgr(), group_(group) {
  }

  void place() {
    while (int(circles_.size()) < factor()) {
      Point center(0.5, 0.5);
      Coord radius(0.01);

      auto *circle = new CircleObj(group_->canvas(), center, radius);

      circles_.push_back(circle);

      (void) group_->canvas()->addNewObject(circle);
    }

    ind_ = 0;

    calc();
    generate(1, 1);

    while (ind_ < circles_.size()) {
      auto *circle = circles_[ind_++];

      Point center(0.5, 0.5);
      Coord radius(0.000001);

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

    Point center(xc, yc);
    Coord radius(size/2.0);

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
  using Circles = std::vector<CircleObj *>;

  CirclesGroupObj *group_ { nullptr };
  Circles          circles_;
  size_t           ind_ { 0 };
};

bool
CirclesGroupObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 1) return false;

  auto *tcl = canvas->app()->tcl();

  auto rect = stringToRect(tcl, args[0]);

  auto *obj = new CirclesGroupObj(canvas, rect);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

CirclesGroupObj::
CirclesGroupObj(Canvas *canvas, const Rect &rect) :
 GroupObj(canvas, rect)
{
  mgr_ = new CirclesMgr(this);

  mgr_->place();
}

QVariant
CirclesGroupObj::
getValue(const QString &name, const QStringList &args)
{
  if (name == "n")
    return mgr_->factor();
  else
    return GroupObj::getValue(name, args);
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
    return GroupObj::setValue(name, value, args);

  return true;
}

#if 0
Point
GroupObj::
pointToWindow(const Point &p) const
{
  if (p.x.units == Units::WINDOW)
    return p;

  double x, y;
  displayRange_.pixelToWindow(p.x.value, p.y.value, &x, &y);

  return Point::makeWindow(x, y);
}
#endif

//---

bool
QuadTreeObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 0)
    return false;

  auto *tcl = canvas->app()->tcl();

  auto *obj = new QuadTreeObj(canvas);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

QuadTreeObj::
QuadTreeObj(Canvas *canvas) :
 GroupObj(canvas, Rect())
{
}

QVariant
QuadTreeObj::
getValue(const QString &name, const QStringList &args)
{
  auto *tcl = canvas()->app()->tcl();

  if      (name == "object.in_rect") {
    if (args.size() < 1)
      return false;

    auto rect = stringToRect(tcl, args[0]);

    QuadTree::DataList dataList;
    quadTree_.getDataInsideBBox(rect, dataList);

    QStringList names;

    for (auto *obj : dataList)
      names.push_back(obj->getCommandName());

    return names;
  }
  else if (name == "object.at_point") {
    if (args.size() < 1)
      return false;

    auto p = stringToPoint(tcl, args[0]);

    QuadTree::DataList dataList;
    quadTree_.getDataAtPoint(p.x.value, p.y.value, dataList);

    QStringList names;

    for (auto *obj : dataList)
      names.push_back(obj->getCommandName());

    return names;
  }
  else
    return GroupObj::getValue(name, args);
}

bool
QuadTreeObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();

  if      (name == "reset") {
    quadTree_.reset();
  }
  else if (name == "object.add") {
    if (args.size() != 0)
      return app->errorMsg("Invalid number of args");

    auto *obj = canvas()->getObjectByName(value);
    if (! obj) return app->errorMsg(QString("Failed to find object '%1'").arg(value));

    quadTree_.add(obj);
  }
  else if (name == "object.remove") {
    if (args.size() != 0)
      return app->errorMsg("Invalid number of args");

    auto *obj = canvas()->getObjectByName(value);
    if (! obj) return app->errorMsg(QString("Failed to find object '%1'").arg(value));

    quadTree_.remove(obj);
  }
  else
    return GroupObj::setValue(name, value, args);

  return true;
}

//---

bool
CircleObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 2) return false;

  auto *tcl = canvas->app()->tcl();

  auto center = stringToPoint(tcl, args[0]);
  auto r      = stringToCoord(args[1]);

  auto *obj = new CircleObj(canvas, center, r);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

CircleObj::
CircleObj(Canvas *canvas, const Point &center, const Coord &radius) :
 Object(canvas), center_(center), radius_(radius)
{
}

QVariant
CircleObj::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "rect")
    return rectToString(calcRect());
  else if (name == "center")
    return pointToString(center_.value());
  else if (name == "center.target")
    return pointToString(center_.target());
  else if (name == "center.steps")
    return int(center_.steps());
  else if (name == "radius")
    return coordToString(radius_.value());
  else if (name == "radius.target")
    return coordToString(radius_.target());
  else if (name == "radius.steps")
    return int(radius_.steps());
  else
    return Object::getValue(name, args);
}

bool
CircleObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->app()->tcl();

  if      (name == "center") {
    center_.setValue(stringToPoint(tcl, value));
  }
  else if (name == "center.target") {
    center_.setTarget(stringToPoint(tcl, value));
  }
  else if (name == "center.steps") {
    center_.setSteps(Util::stringToInt(value));
  }
  else if (name == "radius") {
    radius_.setValue(stringToCoord(value));
  }
  else if (name == "radius.target") {
    radius_.setTarget(stringToCoord(value));
  }
  else if (name == "radius.steps") {
    radius_.setSteps(Util::stringToInt(value));
  }
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
CircleObj::
calcRect() const
{
  auto c = pointToWindow(center_.value());

  auto radius = radius_.value();

  double xr = radius.value;
  double yr = xr;

  if (radius.units == Units::PIXEL) {
    auto p1 = canvas()->pointToWindow(Point::makePixel(0.0, 0.0));
    auto p2 = canvas()->pointToWindow(Point::makePixel(xr, yr));

    xr = std::abs(p2.x.value - p1.x.value);
    yr = std::abs(p2.y.value - p1.y.value);
  }

  auto ll = Point::makeWindow(c.x.value - xr, c.y.value - yr);
  auto ur = Point::makeWindow(c.x.value + xr, c.y.value + yr);

  return Rect(ll, ur);
}

bool
CircleObj::
step()
{
  bool b1 = center_.step();
  bool b2 = radius_.step();
  bool b3 = Object::step();

  return (b1 || b2 || b3);
}

void
CircleObj::
draw(QPainter *painter)
{
  auto rect  = this->calcRect();
  auto prect = canvas()->rectToPixel(rect).qrect();

  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  painter->drawEllipse(prect);
}

//---

bool
RectObj::
create(Canvas *canvas, const QStringList &args)
{
  auto *tcl = canvas->app()->tcl();

  auto rect = Rect(Point(0, 0), Point(1, 1));

  if (args.size() >= 1)
    rect = stringToRect(tcl, args[0]);

  auto *obj = new RectObj(canvas, rect);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

RectObj::
RectObj(Canvas *canvas, const Rect &rect) :
 Object(canvas), rect_(rect)
{
}

QVariant
RectObj::
getValue(const QString &name, const QStringList &args)
{
  if (name == "rect")
    return rectToString(calcRect());
  else
    return Object::getValue(name, args);
}

bool
RectObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->app()->tcl();

  if (name == "rect") {
    rect_ = stringToRect(tcl, value);
  }
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
RectObj::
calcRect() const
{
  return rectToWindow(rect_);
}

void
RectObj::
draw(QPainter *painter)
{
  auto rect  = this->calcRect();
  auto prect = canvas()->rectToPixel(rect).qrect();

  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  painter->drawRect(prect);
}

//---

bool
LineObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 2) return false;

  auto *tcl = canvas->app()->tcl();

  auto p1 = stringToPoint(tcl, args[0]);
  auto p2 = stringToPoint(tcl, args[1]);

  auto *obj = new LineObj(canvas, p1, p2);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

LineObj::
LineObj(Canvas *canvas, const Point &p1, const Point &p2) :
 Object(canvas), p1_(p1), p2_(p2)
{
}

QVariant
LineObj::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "p1")
    return pointToString(p1_);
  else if (name == "p2")
    return pointToString(p2_);
  else
    return Object::getValue(name, args);
}

bool
LineObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->app()->tcl();

  if      (name == "p1")
    p1_ = stringToPoint(tcl, value);
  else if (name == "p2")
    p2_ = stringToPoint(tcl, value);
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
LineObj::
calcRect() const
{
  auto p1 = pointToWindow(p1_);
  auto p2 = pointToWindow(p2_);

  return Rect(p1, p2);
}

void
LineObj::
draw(QPainter *painter)
{
  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  auto p1 = pointToPixel(p1_).qpoint();
  auto p2 = pointToPixel(p2_).qpoint();

  painter->drawLine(p1, p2);
}

//---

EditObj::
EditObj(Canvas *canvas, const QString &name) :
 Object(canvas), name_(name)
{
}

QVariant
EditObj::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "name")
    return name_;
  else if (name == "proc")
    return proc_;
  else
    return Object::getValue(name, args);
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
    return Object::setValue(name, value, args);

  return true;
}

//---

bool
RealEdit::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 2) return false;

  auto *tcl = canvas->app()->tcl();

  auto pos = stringToPoint(tcl, args[0]);

  auto *obj = new RealEdit(canvas, pos, args[1]);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

RealEdit::
RealEdit(Canvas *canvas, const Point &p, const QString &name) :
 EditObj(canvas, name), p_(p)
{
}

QVariant
RealEdit::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "position")
    return pointToString(p_);
  else if (name == "min_value")
    return minValue_;
  else if (name == "max_value")
    return maxValue_;
  else
    return EditObj::getValue(name, args);
}

bool
RealEdit::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "position") {
    auto *tcl = canvas()->app()->tcl();

    p_ = stringToPoint(tcl, value);
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

Rect
RealEdit::
calcRect() const
{
  QFontMetrics fm(canvas()->font());

  int w = fm.horizontalAdvance(name_) + 4;
  int h = fm.height() + 4;

  auto s = canvas()->pixelSizeToWindow(QSizeF(w, h));

  auto p = pointToWindow(p_);

  auto ll = Point(p.x.value            , p.y.value             );
  auto ur = Point(p.x.value + s.width(), p.y.value + s.height());

  return Rect(ll, ur);
}

void
RealEdit::
draw(QPainter *painter)
{
  auto rect  = calcRect();
  auto prect = canvas()->rectToPixel(rect).qrect();

  painter->fillRect(prect, QBrush(Qt::white));

  auto var  = canvas()->app()->tcl()->getVar(name_);
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
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

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
    app->runTclCmd(proc_);
}

//---

bool
IntegerEdit::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 2) return false;

  auto *tcl = canvas->app()->tcl();

  auto pos = stringToPoint(tcl, args[0]);

  auto *obj = new IntegerEdit(canvas, pos, args[1]);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

IntegerEdit::
IntegerEdit(Canvas *canvas, const Point &p, const QString &name) :
 EditObj(canvas, name), p_(p)
{
}

QVariant
IntegerEdit::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "position")
    return pointToString(p_);
  else if (name == "min_value")
    return minValue_;
  else if (name == "max_value")
    return maxValue_;
  else
    return EditObj::getValue(name, args);
}

bool
IntegerEdit::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "position") {
    auto *tcl = canvas()->app()->tcl();

    p_ = stringToPoint(tcl, value);
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

Rect
IntegerEdit::
calcRect() const
{
  QFontMetrics fm(canvas()->font());

  int w = fm.horizontalAdvance("XXXXXX") + 2*fm.height() + 8;
  int h = fm.height() + 4;

  auto s = canvas()->pixelSizeToWindow(QSizeF(w, h));

  auto p = pointToWindow(p_);

  auto ll = Point(p.x.value            , p.y.value             );
  auto ur = Point(p.x.value + s.width(), p.y.value + s.height());

  return Rect(ll, ur);
}

void
IntegerEdit::
draw(QPainter *painter)
{
  auto rect  = calcRect();
  auto prect = canvas()->rectToPixel(rect).qrect();

  painter->fillRect(prect, QBrush(Qt::white));

  auto var  = canvas()->app()->tcl()->getVar(name_);
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
  auto *tcl = canvas()->app()->tcl();

  auto var  = tcl->getVar(name_);
  auto istr = var.toString();

  return Util::stringToInt(istr);
}

void
IntegerEdit::
setIValue(int i)
{
  int i1 = std::min(std::max(i, minValue_), maxValue_);

  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  tcl->setVar(name_, i1);

  if (proc_ != "")
    app->runTclCmd(proc_);
}

//---

bool
ButtonObj::
create(Canvas *canvas, const QStringList &args)
{
  auto *tcl = canvas->app()->tcl();

  Point   pos;
  QString text;

  if      (args.size() >= 2) {
    pos  = stringToPoint(tcl, args[0]);
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
ButtonObj(Canvas *canvas, const Point &p, const QString &name) :
 Object(canvas), p_(p), name_(name)
{
}

QVariant
ButtonObj::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "position")
    return pointToString(p_);
  else if (name == "name")
    return name_;
  else if (name == "proc")
    return proc_;
  else
    return Object::getValue(name, args);
}

bool
ButtonObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "position") {
    auto *tcl = canvas()->app()->tcl();

    p_ = stringToPoint(tcl, value);
  }
  else if (name == "name")
    name_ = value;
  else if (name == "proc")
    proc_ = value;
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
ButtonObj::
calcRect() const
{
  QFontMetrics fm(canvas()->font());

  int w = fm.horizontalAdvance(name_) + 4;
  int h = fm.height() + 4;

  auto s = canvas()->pixelSizeToWindow(QSizeF(w, h));

  auto p = pointToWindow(p_);

  auto ll = Point(p.x.value            , p.y.value             );
  auto ur = Point(p.x.value + s.width(), p.y.value + s.height());

  return Rect(ll, ur);
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
  auto *app = canvas()->app();

  if (proc_ != "")
    app->runTclCmd(proc_);
}

//---

bool
TextObj::
create(Canvas *canvas, const QStringList &args)
{
  auto *tcl = canvas->app()->tcl();

  Point   pos;
  QString text;

  if      (args.size() >= 2) {
    pos  = stringToPoint(tcl, args[0]);
    text = args[1];
  }
  else if (args.size() >= 1) {
    pos= stringToPoint(tcl, args[0]);
  }

  auto *obj = new TextObj(canvas, pos, text);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

TextObj::
TextObj(Canvas *canvas, const Point &pos, const QString &text) :
 Object(canvas), pos_(pos), text_(text)
{
  font_ = canvas->font();
}

QVariant
TextObj::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "position")
    return pointToString(pos_);
  else if (name == "text")
    return text_;
  else if (name == "align")
    return alignToString(align_);
  else if (name == "html")
    return html_;
  else
    return Object::getValue(name, args);
}

bool
TextObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->app()->tcl();

  if      (name == "position")
    pos_ = stringToPoint(tcl, value);
  else if (name == "text")
    text_ = value;
  else if (name == "align")
    align_ = stringToAlign(value);
  else if (name == "html")
    html_ = Util::stringToBool(value);
  else if (name == "border.color")
    border_.setColor(Util::stringToColor(tcl, value));
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
TextObj::
calcRect() const
{
  QSizeF s;

  if (html_) {
    CQHtmlTextPainter textPainter;

    textPainter.setText(text_);

    textPainter.setMargin(4);
    textPainter.setFont(font_);

    s = canvas()->pixelSizeToWindow(textPainter.textSize());
  }
  else {
    QFontMetrics fm(font_);

    int w = fm.horizontalAdvance(text_);
    int h = fm.height();

    s = canvas()->pixelSizeToWindow(QSizeF(w, h));
  }

  auto p = pointToWindow(pos_);

  double x = p.x.value;
  double y = p.y.value;

  if      (align_ & Qt::AlignRight  ) x -= s.width();
  else if (align_ & Qt::AlignHCenter) x -= s.width()/2.0;

  if      (align_ & Qt::AlignBottom ) y -= s.height();
  else if (align_ & Qt::AlignVCenter) y -= s.height()/2.0;

  auto ll = Point(Coord(x            ), Coord(y             ));
  auto ur = Point(Coord(x + s.width()), Coord(y + s.height()));

  return Rect(ll, ur);
}

void
TextObj::
draw(QPainter *painter)
{
  auto rect  = calcRect();
  auto prect = canvas()->rectToPixel(rect).qrect();

  // draw border
  painter->setPen(border_);
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(prect);

  // draw text
  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  if (isHtml()) {
    CQHtmlTextPainter textPainter;

    textPainter.setText(text_);

    textPainter.setMargin(4);
    textPainter.setAlignment(align_);
    textPainter.setTextColor(brush_.value().color());
    textPainter.setFont(font_);

    textPainter.drawInRect(painter, prect.toRect());
  }
  else {
    QFontMetrics fm(font_);

    painter->setFont(font_);

    painter->drawText(prect.left(), prect.top() + fm.ascent(), text_);
  }
}

//---

bool
ImageObj::
create(Canvas *canvas, const QStringList &args)
{
  auto *tcl = canvas->app()->tcl();

  Point  pos;
  QImage image;

  if (args.size() >= 1)
    pos = stringToPoint(tcl, args[0]);

  if (args.size() >= 2) {
    if (args[1] != "") {
      if (! stringToImage(args[1], image))
        return false;
    }
  }

  //---

  auto *obj = new ImageObj(canvas, pos, image);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

ImageObj::
ImageObj(Canvas *canvas, const Point &pos, const QImage &image) :
 Object(canvas), pos_(pos), image_(image)
{
}

QVariant
ImageObj::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "position")
    return pointToString(pos_);
  else if (name == "center") {
    auto ppos = pointToPixel(pos_);

    ppos.x.value += image_.width ()/2;
    ppos.y.value += image_.height()/2;

    return pointToString(ppos);
  }
  else if (name == "image")
    return imageToString(image_);
  else
    return Object::getValue(name, args);
}

bool
ImageObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  if      (name == "position") {
    pos_     = stringToPoint(tcl, value);
    posType_ = Position::TOP_LEFT;
  }
  else if (name == "center") {
    pos_     = stringToPoint(tcl, value);
    posType_ = Position::CENTER;
  }
  else if (name == "rect") {
    rect_    = stringToRect(tcl, value);
    posType_ = Position::RECT;
  }
  else if (name == "image") {
    if (value != "") {
      if (! stringToImage(value, image_)) {
        auto *obj = canvas()->getObjectByName(value);
        if (! obj) return app->errorMsg(QString("Failed to find object '%1'").arg(value));

        auto *imageObj = dynamic_cast<ImageObj *>(obj);
        if (! obj) return false;

        image_ = imageObj->image();
      }
    }
    else
      image_ = QImage();
  }
  else if (name == "flip_x") {
    image_ = image_.mirrored(true, false);
  }
  else if (name == "flip_y") {
    image_ = image_.mirrored(false, true);
  }
  else if (name == "scale") {
    auto size = stringToPoint(tcl, value);

    image_ = image_.scaled(image_.width()*size.x.value, image_.height()*size.y.value);
  }
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
ImageObj::
calcRect() const
{
  if (posType_ == Position::RECT)
    return rect_;

  int w = image_.width ();
  int h = image_.height();

  auto s = canvas()->pixelSizeToWindow(QSizeF(w, h));

  auto pos = pointToPixel(pos_);

  pos.x.value -= w/2;
  pos.y.value -= h/2;

  auto p = pointToWindow(pos);

  auto ll = Point(p.x.value            , p.y.value             );
  auto ur = Point(p.x.value + s.width(), p.y.value + s.height());

  return Rect(ll, ur);
}

void
ImageObj::
draw(QPainter *painter)
{
  if (posType_ == Position::RECT) {
    auto prect = canvas()->rectToPixel(rect_).qrect();

    painter->drawImage(prect, image_);
  }
  else {
    auto pos = pointToPixel(pos_).qpoint();

    if (posType_ == Position::CENTER) {
      int w = image_.width ();
      int h = image_.height();

      pos.setX(pos.x() - w/2);
      pos.setY(pos.y() - h/2);
    }

    if (! image_.isNull())
      painter->drawImage(pos, image_);
  }
}

//---

bool
PathObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 1) return false;

  auto *tcl = canvas->app()->tcl();

  auto path = stringToPath(args[0]);

  auto *obj = new PathObj(canvas, path);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

PathObj::
PathObj(Canvas *canvas, const QPainterPath &path) :
 Object(canvas), path_(path)
{
}

QVariant
PathObj::
getValue(const QString &name, const QStringList &args)
{
  if (name == "path")
    return pathToString(path_);
  else
    return Object::getValue(name, args);
}

bool
PathObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if (name == "path") {
    path_ = stringToPath(value);
  }
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
PathObj::
calcRect() const
{
  auto r = path_.boundingRect();

  auto tl = r.topLeft();
  auto br = r.bottomRight();

  return Rect(Point(Coord(tl.x()), Coord(tl.y())),
              Point(Coord(br.x()), Coord(br.y())));
}

void
PathObj::
draw(QPainter *painter)
{
  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  painter->drawPath(path_);
}

//---

bool
PointListObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 1) return false;

  auto *tcl = canvas->app()->tcl();

  auto r = stringToCoord(args[0]);

  auto *obj = new PointListObj(canvas, r);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

PointListObj::
PointListObj(Canvas *canvas, const Coord &radius) :
 Object(canvas), radius_(radius)
{
}

QVariant
PointListObj::
getValue(const QString &name, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  if      (name == "radius")
    return coordToString(radius_.value());
  else if (name == "radius.target")
    return coordToString(radius_.target());
  else if (name == "radius.steps")
    return int(radius_.steps());
  else if (name == "size")
    return Util::intToString(int(points_.size()));
  else if (name == "connected")
    return Util::boolToString(isConnected());
  else if (name == "show_points")
    return Util::boolToString(isShowPoints());
  else if (name == "angle")
    return angle();
  else if (name == "scale")
    return scale();
  else if (name == "offset")
    return pointToString(offset());
  else if (name == "fill_under")
    return Util::boolToString(isFillUnder());
  else if (name == "fill_under.y") {
    if (fillUnderY())
      return coordToString(*fillUnderY());
    else
      return QVariant();
  }
  else if (name == "position") {
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(points_.size()))
        return QVariant();

      return pointToString(points_[i]);
    }
    else
      return QVariant();
  }
  else if (name == "intersect") {
    if (args.size() < 0)
      return QVariant();

    auto pos = stringToPoint(tcl, args[0]);

    auto pos1 = canvas()->pointToPixel(pos).qpoint();

    return path_.contains(pos1);
  }
  else if (name == "intersect_obj") {
    if (args.size() < 0)
      return QVariant();

    auto *obj = canvas()->getObjectByName(args[0]);
    if (! obj) {
     app->errorMsg(QString("Failed to find object '%1'").arg(args[0]));
     return QVariant();
    }

    auto path1 = calcPath();
    auto path2 = obj->calcPath();

    return path1.intersects(path2);
  }
  else
    return Object::getValue(name, args);
}

bool
PointListObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  if      (name == "radius")
    radius_.setValue(stringToCoord(value));
  else if (name == "radius.target")
    radius_.setTarget(stringToCoord(value));
  else if (name == "radius.steps")
    radius_.setSteps(Util::stringToInt(value));
  else if (name == "size") {
    auto n = Util::stringToInt(value);

    auto n1 = int(points_.size());

    if      (n > n1) {
      for (int i = 0; i < n - n1; ++i)
        points_.emplace_back();
    }
    else if (n < n1) {
      for (int i = 0; i < n1 - n; ++i)
        points_.pop_back();
    }
  }
  else if (name == "connected")
    setConnected(Util::stringToBool(value));
  else if (name == "show_points")
    setFillUnder(Util::stringToBool(value));
  else if (name == "fill_under")
    setFillUnder(Util::stringToBool(value));
  else if (name == "fill_under.y")
    setFillUnderY(stringToCoord(value));
  else if (name == "angle")
    setAngle(Util::stringToReal(value));
  else if (name == "scale")
    setScale(Util::stringToReal(value));
  else if (name == "offset")
    setOffset(stringToPoint(tcl, value));
  else if (name == "position") {
    // get index from args
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(points_.size()))
        return false;

      points_[i] = stringToPoint(tcl, value);
    }
    else
      return app->errorMsg("Missing index for position");
  }
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
PointListObj::
calcRect() const
{
  QRectF r;
  bool   rset { false };

  for (const auto &point : points_) {
    auto c = pointToWindow(point).qpoint();

    if (! rset) {
      r = QRectF(c.x(), c.y(), 0, 0);

      rset = true;
    }
    else {
      auto x1 = std::min(r.left  (), c.x());
      auto y1 = std::min(r.top   (), c.y());
      auto x2 = std::max(r.right (), c.x());
      auto y2 = std::max(r.bottom(), c.y());

      r = QRectF(x1, y1, x2 - x1, y2 - y1);
    }
  }

  return Rect::makeWindow(r);
}

bool
PointListObj::
step()
{
  bool b1 = radius_.step();
  bool b2 = Object::step();

  return (b1 || b2);
}

void
PointListObj::
draw(QPainter *painter)
{
  auto rect  = this->calcRect();
  auto prect = canvas()->rectToPixel(rect).qrect();

  auto c  = center();
  auto pc = canvas()->pointToPixel(c).qpoint();

  auto po = canvas()->pointToPixel(Point()).qpoint();
  auto pf = canvas()->pointToPixel(offset()).qpoint() - po;

  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  QTransform t;

  t.translate(pc.x() + pf.x(), pc.y() + pf.y());
  t.rotate(-angle());
  t.scale(scale(), scale());
  t.translate(-pc.x(), -pc.y());

#if 0
  painter->setTransform(t);
#endif

  //painter->drawRect(prect);

  //---

  auto radius = radius_.value();

  double xr = radius.value;
  double yr = xr;

  if (radius.units == Units::PIXEL) {
    auto p1 = canvas()->pointToWindow(Point::makePixel(0.0, 0.0));
    auto p2 = canvas()->pointToWindow(Point::makePixel(xr, yr));

    xr = std::abs(p2.x.value - p1.x.value);
    yr = std::abs(p2.y.value - p1.y.value);
  }

  //---

  QPainterPath path;

  int i = 0;

  QPointF p1, p2;

  for (const auto &point : points_) {
    auto p = pointToWindow(point);

    auto pp = canvas()->pointToPixel(p).qpoint();

    if (i == 0) {
      p1 = pp;

      path.moveTo(pp);
    }
    else {
      path.lineTo(pp);

      p2 = pp;
    }

    ++i;
  }

  auto path1 = path;

  if (isConnected())
    path1.closeSubpath();

  path_ = t.map(path1);

  //---

  if (isFillUnder()) {
    auto by = prect.bottom();

    if (fillUnderY()) {
      auto c = *fillUnderY();
      auto p = Point(c, c);

      auto pw = pointToWindow(p);
      auto pp = canvas()->pointToPixel(pw).qpoint();

      by = pp.y();
    }

    auto path2 = path;

    path2.lineTo(p2.x(), by);
    path2.lineTo(p1.x(), by);

    path2.closeSubpath();

    painter->fillPath(t.map(path2), painter->brush());
  }

  if (isFilled())
    painter->fillPath(path_, painter->brush());

  if (isStroked())
    painter->strokePath(path_, painter->pen());

  if (isShowPoints()) {
    for (const auto &point : points_) {
      auto c = pointToWindow(point);

      auto ll = Point::makeWindow(c.x.value - xr, c.y.value - yr);
      auto ur = Point::makeWindow(c.x.value + xr, c.y.value + yr);

      auto rect  = Rect(ll, ur);
      auto prect = canvas()->rectToPixel(rect).qrect();

      painter->drawEllipse(prect);
    }
  }
}

//---

bool
ArrowObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 2) return false;

  auto *tcl = canvas->app()->tcl();

  auto p1 = stringToPoint(tcl, args[0]);
  auto p2 = stringToPoint(tcl, args[1]);

  auto *obj = new ArrowObj(canvas, p1, p2);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

ArrowObj::
ArrowObj(Canvas *canvas, const Point &p1, const Point &p2) :
 Object(canvas), p1_(p1), p2_(p2)
{
  arrow_ = new CQArrow;
}

QVariant
ArrowObj::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "p1")
    return pointToString(p1_);
  else if (name == "p2")
    return pointToString(p2_);
  else
    return Object::getValue(name, args);
}

bool
ArrowObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->app()->tcl();

  if      (name == "p1")
    p1_ = stringToPoint(tcl, value);
  else if (name == "p2")
    p2_ = stringToPoint(tcl, value);
  else if (name == "lineWidth")
    arrow_->setLineWidth(Util::stringToReal(value));
  else if (name == "front.visible")
    arrow_->setFHead(Util::stringToBool(value));
  else if (name == "front.angle")
    arrow_->setFrontAngle(Util::stringToReal(value));
  else if (name == "front.backAngle")
    arrow_->setFrontBackAngle(Util::stringToReal(value));
  else if (name == "front.length")
    arrow_->setFrontLength(Util::stringToReal(value));
  else if (name == "front.lineEnds")
    arrow_->setFrontLineEnds(Util::stringToBool(value));
  else if (name == "tail.visible")
    arrow_->setTHead(Util::stringToBool(value));
  else if (name == "tail.angle")
    arrow_->setTailAngle(Util::stringToReal(value));
  else if (name == "tail.backAngle")
    arrow_->setTailBackAngle(Util::stringToReal(value));
  else if (name == "tail.length")
    arrow_->setTailLength(Util::stringToReal(value));
  else if (name == "tail.lineEnds")
    arrow_->setTailLineEnds(Util::stringToBool(value));
  else if (name == "filled")
    arrow_->setFilled(Util::stringToBool(value));
  else if (name == "stroked")
    arrow_->setStroked(Util::stringToBool(value));
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
ArrowObj::
calcRect() const
{
  return Rect(p1_, p2_);
}

void
ArrowObj::
draw(QPainter *painter)
{
  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  class Device : public CQArrowDevice {
   public:
    Device(Canvas *canvas) :
     canvas_(canvas) {
    }

    QPointF windowToPixel(const QPointF &w) override {
      return canvas_->pointToPixel(Point::makeWindow(w)).qpoint();
    }

    QPointF pixelToWindow(const QPointF &p) override {
      return canvas_->pointToWindow(Point::makePixel(p)).qpoint();
    }

   private:
    Canvas *canvas_ { nullptr };
  };

  Device device(canvas());

  auto p1 = pointToWindow(p1_).qpoint();
  auto p2 = pointToWindow(p2_).qpoint();

  arrow_->setFrom(p1);
  arrow_->setTo  (p2);

  arrow_->draw(painter, &device);
}

//---

bool
AxisObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 2) return false;

  auto *tcl = canvas->app()->tcl();

  auto pos = stringToPoint(tcl, args[0]);
  auto len = stringToCoord(args[1]);

  auto *obj = new AxisObj(canvas, pos, len);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

AxisObj::
AxisObj(Canvas *canvas, const Point &pos, const Coord &len) :
 Object(canvas), pos_(pos), len_(len)
{
  axis_ = new CQAxis;
}

QVariant
AxisObj::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "pos")
    return pointToString(pos_);
  else if (name == "p2")
    return coordToString(len_);
  else
    return Object::getValue(name, args);
}

bool
AxisObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->app()->tcl();

  if      (name == "pos")
    pos_ = stringToPoint(tcl, value);
  else if (name == "p2")
    len_ = stringToCoord(value);
  else if (name == "direction") {
    auto lstr = value.toLower();

    if      (lstr == "horizontal")
      axis_->setDirection(CQAxis::DIR_HORIZONTAL);
    else if (lstr == "vertical")
      axis_->setDirection(CQAxis::DIR_VERTICAL);
  }
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
AxisObj::
calcRect() const
{
  Point pos1 = pointToWindow(pos_);
  Point pos2;

  if (len_.units == Units::PIXEL) {
    auto ppos1 = pointToPixel(pos_).qpoint();

    QPointF ppos2;

    if (axis_->getDirection() == CQAxis::DIR_HORIZONTAL)
      ppos2 = QPointF(ppos1.x() + len_.value, ppos1.y());
    else
      ppos2 = QPointF(ppos1.x(), ppos1.y() + len_.value);

    pos2 = pointToWindow(Point::makePixel(ppos2));
  }
  else {
    if (axis_->getDirection() == CQAxis::DIR_HORIZONTAL)
      pos2 = Point::makeWindow(pos1.x.value + len_.value, pos1.y.value);
    else
      pos2 = Point::makeWindow(pos1.x.value, pos1.y.value + len_.value);
  }

  return Rect(pos1, pos2);
}

void
AxisObj::
draw(QPainter *painter)
{
  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  auto rect  = calcRect();
  auto prect = canvas()->rectToPixel(rect).qrect();

  if (axis_->getDirection() == CQAxis::DIR_HORIZONTAL)
    axis_->draw(painter, prect.left(), prect.bottom(), prect.width());
  else
    axis_->draw(painter, prect.left(), prect.bottom(), -prect.height());
}

//---

bool
ParticleObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 1) return false;

  auto *tcl = canvas->app()->tcl();

  auto pos = stringToPoint(tcl, args[0]);

  auto *obj = new ParticleObj(canvas, pos);

  double mass = 1.0;

  auto *particle = dynamic_cast<Particle *>(
    canvas->psys()->makeParticle(mass, pos.x.value, pos.y.value));

  particle->setObj(obj);

  obj->setParticle(particle);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

ParticleObj::
ParticleObj(Canvas *canvas, const Point &pos) :
 Object(canvas), pos_(pos)
{
}

void
ParticleObj::
setParticle(Particle *p)
{
  particle_ = p;

  particle_->setPosition(pos_.x.value, pos_.y.value, 0);
}

QVariant
ParticleObj::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "position") {
    auto *position = particle_->position();

    return pointToString(Point(position->x(), position->y()));
  }
  else if (name == "velocity") {
    auto *velocity = particle_->velocity();

    return pointToString(Point(velocity->x(), velocity->y()));
  }
  else if (name == "dead") {
    return Util::boolToString(particle_->isDead());
  }
  else if (name == "age") {
    return particle_->age();
  }
  else
    return Object::getValue(name, args);
}

bool
ParticleObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  if      (name == "position") {
    auto p = stringToPoint(tcl, value);

    particle_->setPosition(p.x.value, p.y.value, 0);
  }
  else if (name == "velocity") {
    auto p = stringToPoint(tcl, value);

    particle_->setVelocity(p.x.value, p.y.value, 0);
  }
  else if (name == "dead") {
    particle_->setDead(Util::stringToBool(value));
  }
  else if (name == "age") {
    particle_->setAge(Util::stringToReal(value));
  }
  else if (name == "size") {
    particle_->setSize(Util::stringToReal(value));
  }
  else if (name == "tpos") {
    auto p = stringToPoint(tcl, value);

    particle_->setTPos(CPoint2D(p.x.value, p.y.value));
  }
  else if (name == "tsize") {
    auto p = stringToPoint(tcl, value);

    particle_->setTSize(CSize2D(p.x.value, p.y.value));
  }
  else if (name == "angle") {
    particle_->setAngle(Util::stringToReal(value));
  }
  else if (name == "color") {
    particle_->setColor(QColorToRGBA(Util::stringToColor(tcl, value)));
  }
  else if (name == "alpha") {
    particle_->setAlpha(Util::stringToReal(value));
  }
  else if (name == "image") {
    QImage image;

    if (! stringToImage(value, image)) {
      auto *obj = canvas()->getObjectByName(value);
      if (! obj) return app->errorMsg(QString("Failed to find object '%1'").arg(value));

      auto *imageObj = dynamic_cast<ImageObj *>(obj);
      if (! obj) return false;

      particle_->setImage(imageObj->image());
    }
  }
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
ParticleObj::
calcRect() const
{
  auto *position = particle_->position();

  auto p = Point(position->x(), position->y());

  return Rect(p, p);
}

void
ParticleObj::
draw(QPainter *)
{
}

//---

bool
VectorObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() > 1)
    return false;

  auto *tcl = canvas->app()->tcl();

  auto *obj = new VectorObj(canvas);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

VectorObj::
VectorObj(Canvas *canvas) :
 Object(canvas)
{
}

QVariant
VectorObj::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "x")
    return v_.x();
  else if (name == "y")
    return v_.y();
  else
    return Object::getValue(name, args);
}

bool
VectorObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "x")
    v_.setX(Util::stringToReal(value));
  else if (name == "y")
    v_.setY(Util::stringToReal(value));
  else
    return Object::setValue(name, value, args);

  return true;
}

//---

bool
ArrayObj::
create(Canvas *canvas, const QStringList &args)
{
  auto *tcl = canvas->app()->tcl();

  if (args.size() != 2)
    return false;

  auto dim0 = Util::stringToInt(args[0]);
  auto dim1 = Util::stringToInt(args[1]);

  auto *obj = new ArrayObj(canvas, dim0, dim1);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

ArrayObj::
ArrayObj(Canvas *canvas, uint dim0, uint dim1) :
 Object(canvas), a_(dim0, dim1, 0.0)
{
}

ArrayObj::
ArrayObj(Canvas *canvas, const CArray2D<double> &a) :
 Object(canvas), a_(a)
{
}

QVariant
ArrayObj::
getValue(const QString &name, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  if      (name == "value") {
    uint dim0, dim1;

    if      (args.size() == 2) {
      dim0 = Util::stringToInt(args[0]);
      dim1 = Util::stringToInt(args[1]);
    }
    else if (args.size() == 1) {
      std::vector<int> a;
      if (! stringToIntArray(tcl, args[0], a) || a.size() != 2)
        return false;

      dim0 = a[0];
      dim1 = a[1];
    }
    else
      return QVariant();

    if (! a_.validIndex(dim0, dim1))
      return QVariant();

    return a_.get(dim0, dim1);
  }
  else if (name == "dim0") {
    return int(a_.dim(0));
  }
  else if (name == "dim1") {
    return int(a_.dim(1));
  }
  else if (name == "dup") {
    auto *obj = new ArrayObj(canvas(), a_);

    auto name = canvas()->addNewObject(obj);

    return name;
  }
  else
    return Object::getValue(name, args);
}

bool
ArrayObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  if (name == "value") {
    uint dim0, dim1;

    if      (args.size() == 2) {
      dim0 = Util::stringToInt(args[0]);
      dim1 = Util::stringToInt(args[1]);
    }
    else if (args.size() == 1) {
      std::vector<int> a;
      if (! stringToIntArray(tcl, args[0], a) || a.size() != 2)
        return false;

      dim0 = a[0];
      dim1 = a[1];
    }
    else
      return false;

    if (! a_.validIndex(dim0, dim1))
      return false;

    auto r = Util::stringToReal(value);

    a_.set(dim0, dim1, r);
  }
  else
    return Object::setValue(name, value, args);

  return true;
}

//---

bool
CsvObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 1)
    return false;

  auto *tcl = canvas->app()->tcl();

  auto filename = args[0];

  auto *obj = new CsvObj(canvas, filename);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

CsvObj::
CsvObj(Canvas *canvas, const QString &filename) :
 Object(canvas), filename_(filename)
{
  csv_ = new CQCsvModel;
}

QVariant
CsvObj::
getValue(const QString &name, const QStringList &args)
{
  auto *app = canvas()->app();

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
    return Object::getValue(name, args);
}

bool
CsvObj::
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
    return Object::setValue(name, value, args);

  return true;
}

bool
CsvObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  if (op == "load") {
    if (! csv_->load(filename_))
      return false;

    return true;
  }
  else
    return Object::exec(op, args, res);
}

//---

Object::
Object(Canvas *canvas, size_t ind) :
 canvas_(canvas), ind_(ind)
{
  pen_   = canvas->stylePen();
  brush_ = canvas->styleBrush();
}

QString
Object::
getCommandName() const
{
  return QString("sb::%1.%2").arg(typeName()).arg(ind_);
}

Point
Object::
pointToWindow(const Point &p) const
{
  auto p1 = p;

  auto *group = this->group();

  while (group) {
    const auto &range = group->displayRange();

    double x, y;
    range.windowToPixel(p1.x.value, p1.y.value, &x, &y);

    p1 = Point::makeWindow(x, y);

    group = group->group();
  }

  if (p1.x.units == Units::PIXEL) {
    auto *viewport = canvas()->currentViewport();

    double x, y;

    if (viewport->hasRange) {
      const auto &range = viewport->displayRange;

      range.pixelToWindow(p1.x.value, p1.y.value, &x, &y);
    }
    else {
      x = p1.x.value;
      y = p1.y.value;
    }

    return Point::makeWindow(x, y);
  }
  else
    return Point::makeWindow(p1.x.value, p1.y.value);
}

Point
Object::
pointToPixel(const Point &p) const
{
  if (group_) {
    double px, py;
    group_->displayRange().windowToPixel(p.x.value, p.y.value, &px, &py);

    return canvas()->pointToPixel(Point::makeWindow(px, py));
  }
  else
    return canvas()->pointToPixel(p);
}

Rect
Object::
rectToWindow(const Rect &r) const
{
  auto p1 = pointToWindow(r.ll);
  auto p2 = pointToWindow(r.ur);

  return Rect(p1, p2);
}

QString
Object::
calcId() const
{
  auto id = this->id();

  if (id == "")
    id = getCommandName();

  return id;
}

QVariant
Object::
getValue(const QString &name, const QStringList &)
{
  auto *app = canvas()->app();

  if      (name == "id")
    return id();
  else if (name == "visible")
    return isVisible();
  else if (name == "brush.color")
    return Util::colorToString(brush_.value().color());
  else if (name == "brush.color.target")
    return Util::colorToString(brush_.target().color());
  else if (name == "brush.alpha")
    return Util::realToString(brush_.value().color().alphaF());
  else if (name == "brush.steps")
    return Util::colorToString(brush_.steps());
  else if (name == "pen.color")
    return Util::colorToString(pen_.color());
  else if (name == "pen.width")
    return Util::realToString(pen_.widthF());
  else if (name == "group")
    return (group() ? group()->calcId() : "");
  else if (name.left(5) == "user.")
    return nameValue(name.mid(5));
  else if (name.left(8) == "animate.") {
    auto name1 = name.mid(8);

    if (name1 == "animating")
      return isAnimating();
    else {
      app->errorMsg(QString("Invalid get name '%1' for '%2'").arg(name).arg(getCommandName()));
      return QVariant();
    }
  }
  else if (name == "meta") {
    return meta_;
  }
  else {
    app->errorMsg(QString("Invalid get name '%1' for '%2'").arg(name).arg(getCommandName()));
    return QVariant();
  }
}

bool
Object::
setValue(const QString &name, const QString &value, const QStringList &)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  if      (name == "id")
    setId(value);
  else if (name == "visible")
    setVisible(Util::stringToBool(value));
  else if (name == "stroked")
    setStroked(Util::stringToBool(value));
  else if (name == "filled")
    setFilled(Util::stringToBool(value));
  else if (name == "brush.color") {
    auto b = brush_.value();

    b.setColor(Util::stringToColor(tcl, value));

    brush_ = b;
  }
  else if (name == "brush.color.target") {
    auto b = brush_.target();

    b.setColor(Util::stringToColor(tcl, value));

    brush_.setTarget(b);
  }
  else if (name == "brush.steps")
    brush_.setSteps(Util::stringToInt(value));
  else if (name == "brush.alpha") {
    auto b = brush_.value();
    auto c = b.color();

    c.setAlphaF(Util::stringToReal(value));
    b.setColor(c);

    brush_ = b;
  }
  else if (name == "brush.linear_gradient") {
    QStringList strs;
    (void) tcl->splitList(value, strs);
    if (strs.size() != 4) return false;

    auto x1 = Util::stringToReal(strs[0]);
    auto y1 = Util::stringToReal(strs[1]);
    auto x2 = Util::stringToReal(strs[2]);
    auto y2 = Util::stringToReal(strs[3]);

    QLinearGradient lg(x1, y1, x2, y2);

    QGradientStops stops;

    stops.push_back(QGradientStop(0.0, Qt::red));
    stops.push_back(QGradientStop(1.0, Qt::green));

    lg.setStops(stops);

    lg.setCoordinateMode(QGradient::ObjectMode);

    brush_ = QBrush(lg);
  }
  else if (name == "pen.color")
    pen_.setColor(Util::stringToColor(tcl, value));
  else if (name == "pen.width")
    pen_.setWidthF(Util::stringToReal(value));
  else if (name == "pen.dash")
    pen_.setDashPattern(stringToDashes(tcl, value));
  else if (name == "group") {
    auto *group = dynamic_cast<GroupObj *>(canvas()->getObjectByName(value));
    if (! group) return app->errorMsg(QString("Failed to find group '%1'").arg(value));

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
  else if (name.left(5) == "user.") {
    setNameValue(name.mid(5), value);
  }
  else if (name.left(8) == "animate.") {
    auto name1 = name.mid(8);

    if (name1 == "animating")
      setAnimating(Util::stringToBool(value));
    else
      return app->errorMsg(QString("Invalid set name '%1' for '%2'").
               arg(name).arg(getCommandName()));
  }
  else if (name == "meta") {
    meta_ = value;
  }
  else
    return app->errorMsg(QString("Invalid set name '%1' for '%2'").
               arg(name).arg(getCommandName()));

  return true;
}

bool
Object::
step()
{
  return brush_.step();
}

void
Object::
press(int, int)
{
  //std::cerr << "Press: " << calcId().toStdString() << "\n";
}

void
Object::
click(int, int)
{
  //std::cerr << "Click: " << calcId().toStdString() << "\n";
}

//---

ParticleSystem::
ParticleSystem() :
 CPSysSystem(-1.0, 0.1)
{
}

CPSysParticle *
ParticleSystem::
makeParticle(double mass, double x, double y, double z)
{
  auto *particle = new Particle;

#if 0
  if (particleObj_) {
    particle->setObj(particleObj_);

    particleObj_->setParticle(particle);
  }
#endif

  particle->setMass(mass);

  particle->position()->set(x, y, z);

  addParticle(particle);

  return particle;
}

//---

Particle::
Particle(double mass) :
 CPSysParticle(mass)
{
}

void
Particle::
updateParticle()
{
}

}
