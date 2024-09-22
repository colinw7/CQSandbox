#include <CQSandboxCanvas.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQSVGUtil.h>
#include <CQTclUtil.h>

#include <CQHtmlTextPainter.h>
#include <CQArrow.h>
#include <CQAxis.h>
#include <CQCsvModel.h>
#include <CCircleFactor.h>

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

  bool ok;
  coord.value = str1.toDouble(&ok);

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
    bool ok;
    auto x = strs[0].toDouble(&ok);
    auto y = strs[1].toDouble(&ok);

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
    bool ok;
    auto x1 = strs[0].toDouble(&ok);
    auto y1 = strs[1].toDouble(&ok);
    auto x2 = strs[2].toDouble(&ok);
    auto y2 = strs[3].toDouble(&ok);

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

QImage
stringToImage(const QString &str) {
  auto image = QImage(str);
  image.setText("name", str);
  return image;
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

}

//---

namespace CQSandbox {

Canvas::
Canvas(App *app) :
 QFrame(app), app_(app)
{
  setFocusPolicy(Qt::StrongFocus);

  setMouseTracking(true);
}

void
Canvas::
init()
{
  addCommands();

  app_->runTclCmd("proc init { args } { }");
  app_->runTclCmd("proc update { args } { }");
  app_->runTclCmd("proc drawBg { args } { }");
  app_->runTclCmd("proc drawFg { args } { }");

  //---

  timer_ = new QTimer;

  connect(timer_, &QTimer::timeout, this, &Canvas::timerSlot);

  pen_   = QPen(Qt::black);
  brush_ = QBrush(Qt::white);

  stylePen_   = QPen(Qt::black);
  styleBrush_ = QBrush(Qt::white);
}

void
Canvas::
addCommands()
{
  auto *tcl = app_->tcl();

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

  //---

  // data
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

  // graphics
  tcl->createObjCommand("sb::draw_point",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas::drawPointProc),
    static_cast<CQTcl::ObjCmdData>(this));

  //---

  // layout
  tcl->createObjCommand("sb::circles_group",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<CirclesGroupObj>),
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

  double px, py;

  displayRange_.windowToPixel(p.x.value, p.y.value, &px, &py);

  return Point::makePixel(px, py);
}

Point
Canvas::
pointToWindow(const Point &p) const
{
  if (p.x.units == Units::WINDOW)
    return p;

  double x, y;
  displayRange_.pixelToWindow(p.x.value, p.y.value, &x, &y);

  return Point::makeWindow(x, y);
}

QSizeF
Canvas::
pixelSizeToWindow(const QSizeF &psize) const
{
  double x1, y1, x2, y2;
  displayRange_.pixelToWindow(0.0          , 0.0           , &x1, &y1);
  displayRange_.pixelToWindow(psize.width(), psize.height(), &x2, &y2);

  return QSizeF(std::abs(x2 - x1), std::abs(y2 - y1));
}

void
Canvas::
play()
{
  step();

  timer_->start(100);
}

void
Canvas::
pause()
{
  timer_->stop();
}

void
Canvas::
step()
{
  stepInit();

  for (auto *obj : objects_) {
    if (obj->isAnimating()) {
      if (! obj->step())
        obj->setAnimating(false);
    }
  }

  app_->runTclCmd("update");

  update();
}

void
Canvas::
stepInit()
{
  if (! initialized_) {
    app_->runTclCmd("init");

    initialized_ = true;
  }
}

void
Canvas::
timerSlot()
{
  step();
}

void
Canvas::
resizeEvent(QResizeEvent *)
{
  displayRange_.setPixelRange(0, 0, width() - 1, height() - 1);
}

void
Canvas::
paintEvent(QPaintEvent *)
{
  painter_ = new QPainter(this);

  painter_->fillRect(rect(), brush_.value());

  app_->runTclCmd("drawBg");

  for (auto *obj : objects_) {
    if (obj->isVisible())
      obj->draw(painter_);
  }

  app_->runTclCmd("drawFg");

  delete painter_;

  painter_ = nullptr;
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
}

void
Canvas::
mouseMoveEvent(QMouseEvent *e)
{
  if (pressObj_) {
    auto dx = e->x() - motionPos_.x();
    auto dy = e->y() - motionPos_.y();

    pressObj_->move(dx, dy);

    motionPos_ = e->pos();
  }
  else {
    auto *group = dynamic_cast<GroupObj *>(getObjectAtPos(e->pos()));

    QPointF p;
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
      p    = pointToWindow(Point::makePixel(e->pos())).qpoint();
      name = "canvas";
    }

    app_->setInfo(QString("%1: %2 %3").arg(name).arg(p.x()).arg(p.y()));
  }
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
}

Object *
Canvas::
getObjectAtPos(const QPoint &pos) const
{
  for (auto *obj : objects_) {
    auto rect  = obj->calcRect();
    auto prect = rectToPixel(rect).qrect();

    if (prect.contains(pos))
      return obj;
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
  objects_.push_back(obj);

  obj->setGroup(nullptr);

  Q_EMIT objectsChanged();
}

void
Canvas::
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
    if (args.size() >= 2) {
      auto res = th->getValue(args[1]);

      tcl->setResult(res);
    }
  }
  else if (args[0] == "set") {
    if (args.size() >= 3)
      th->setValue(args[1], args[2]);
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
getValue(const QString &name)
{
  auto *tcl = app_->tcl();

  if      (name == "brush.color")
    return Util::colorToString(brush_.value().color());
  else if (name == "brush.color.target")
    return Util::colorToString(brush_.target().color());
  else if (name == "brush.steps")
    return Util::colorToString(brush_.steps());
  else if (name == "pen.color")
    return Util::colorToString(pen_.color());
  else if (name == "pen.width")
    return Util::realToString(pen_.widthF());
  else if (name == "range")
    return rangeToString(tcl, displayRange_);
  else if (name == "equal_scale") {
    auto b = displayRange_.getEqualScale();

    return Util::boolToString(b);
  }
  else {
    app_->errorMsg(QString("Invalid value name '%1'").arg(name));
    return QVariant();
  }
}

void
Canvas::
setValue(const QString &name, const QString &value)
{
  auto *tcl = app_->tcl();

  if      (name == "brush.color") {
    auto b = brush_.value();

    b.setColor(Util::stringToColor(value));

    brush_ = b;
  }
  else if (name == "brush.color.target") {
    auto b = brush_.target();

    b.setColor(Util::stringToColor(value));

    brush_.setTarget(b);
  }
  else if (name == "brush.steps")
    brush_.setSteps(Util::stringToInt(value));
  else if (name == "pen.color")
    pen_.setColor(Util::stringToColor(value));
  else if (name == "pen.width")
    pen_.setWidthF(Util::stringToReal(value));
  else if (name == "range")
    stringToRange(tcl, displayRange_, value);
  else if (name == "equal_scale")
    displayRange_.setEqualScale(Util::stringToBool(value));
  else if (name == "play")
    timer_->start(100);
  else
    app_->errorMsg(QString("Invalid value name '%1'").arg(name));
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
  if      (name == "brush.color")
    styleBrush_.setColor(Util::stringToColor(value));
  else if (name == "pen.color")
    stylePen_.setColor(Util::stringToColor(value));
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

      auto res = obj->exec(op, args1);

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

void
GroupObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->app()->tcl();

  if      (name == "rect")
    rect_ = stringToRect(tcl, value);
  else if (name == "range")
    stringToRange(tcl, displayRange_, value);
  else
    Object::setValue(name, value, args);
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

      bool ok;
      auto f = circle->nameValue("factor").toDouble(&ok);

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

void
CirclesGroupObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if (name == "n") {
    mgr_->setFactor(Util::stringToInt(value));

    mgr_->place();
  }
  else
    GroupObj::setValue(name, value, args);
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

void
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
    Object::setValue(name, value, args);
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
  if (args.size() != 1) return false;

  auto *tcl = canvas->app()->tcl();

  auto rect = stringToRect(tcl, args[0]);

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

void
RectObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->app()->tcl();

  if (name == "rect") {
    rect_ = stringToRect(tcl, value);
  }
  else
    Object::setValue(name, value, args);
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

void
LineObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->app()->tcl();

  if      (name == "p1")
    p1_ = stringToPoint(tcl, value);
  else if (name == "p2")
    p2_ = stringToPoint(tcl, value);
  else
    Object::setValue(name, value, args);
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

void
EditObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "name")
    name_ = value;
  else if (name == "proc") {
    proc_ = value;
  }
  else
    Object::setValue(name, value, args);
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

void
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
    EditObj::setValue(name, value, args);
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

void
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
    EditObj::setValue(name, value, args);
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
  if (args.size() != 2) return false;

  auto *tcl = canvas->app()->tcl();

  auto pos = stringToPoint(tcl, args[0]);

  auto *obj = new ButtonObj(canvas, pos, args[1]);

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

void
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
    Object::setValue(name, value, args);
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
  if (args.size() != 2) return false;

  auto *tcl = canvas->app()->tcl();

  auto pos  = stringToPoint(tcl, args[0]);
  auto text = args[1];

  auto *obj = new TextObj(canvas, pos, text);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

TextObj::
TextObj(Canvas *canvas, const Point &pos, const QString &text) :
 Object(canvas), pos_(pos), text_(text)
{
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

void
TextObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "position") {
    auto *tcl = canvas()->app()->tcl();

    pos_ = stringToPoint(tcl, value);
  }
  else if (name == "text")
    text_ = value;
  else if (name == "align")
    align_ = stringToAlign(value);
  else if (name == "html")
    html_ = Util::stringToBool(value);
  else
    Object::setValue(name, value, args);
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

  painter->setPen(Qt::red);
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(prect);

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

    painter->drawText(prect.left(), prect.top() + fm.ascent(), text_);
  }
}

//---

bool
ImageObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 2) return false;

  auto *tcl = canvas->app()->tcl();

  auto pos   = stringToPoint(tcl, args[0]);
  auto image = stringToImage(args[1]);

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
  else if (name == "image")
    return imageToString(image_);
  else
    return Object::getValue(name, args);
}

void
ImageObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "position") {
    auto *tcl = canvas()->app()->tcl();

    pos_ = stringToPoint(tcl, value);
  }
  else if (name == "image") {
    image_ = stringToImage(value);
  }
  else
    Object::setValue(name, value, args);
}

Rect
ImageObj::
calcRect() const
{
  int w = image_.width ();
  int h = image_.height();

  auto s = canvas()->pixelSizeToWindow(QSizeF(w, h));

  auto p = pointToWindow(pos_);

  auto ll = Point(p.x.value            , p.y.value             );
  auto ur = Point(p.x.value + s.width(), p.y.value + s.height());

  return Rect(ll, ur);
}

void
ImageObj::
draw(QPainter *painter)
{
  auto pos = pointToPixel(pos_).qpoint();

  if (! image_.isNull())
    painter->drawImage(pos, image_);
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

void
PathObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if (name == "path") {
    path_ = stringToPath(value);
  }
  else
    Object::setValue(name, value, args);
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
  else
    return Object::getValue(name, args);
}

void
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
  else if (name == "fill_under")
    setFillUnder(Util::stringToBool(value));
  else if (name == "fill_under.y")
    setFillUnderY(stringToCoord(value));
  else if (name == "position") {
    // get index from args
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(points_.size()))
        return;

      points_[i] = stringToPoint(tcl, value);
    }
    else
      app->errorMsg("Missing index for position");
  }
  else
    Object::setValue(name, value, args);
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

  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  //painter->drawRect(prect);

  auto radius = radius_.value();

  double xr = radius.value;
  double yr = xr;

  if (radius.units == Units::PIXEL) {
    auto p1 = canvas()->pointToWindow(Point::makePixel(0.0, 0.0));
    auto p2 = canvas()->pointToWindow(Point::makePixel(xr, yr));

    xr = std::abs(p2.x.value - p1.x.value);
    yr = std::abs(p2.y.value - p1.y.value);
  }

  if (isConnected() || isFillUnder()) {
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

    if (isFillUnder()) {
      auto by = prect.bottom();

      if (fillUnderY()) {
        auto c = *fillUnderY();
        auto p = Point(c, c);

        auto pw = pointToWindow(p);
        auto pp = canvas()->pointToPixel(pw).qpoint();

        by = pp.y();
      }

      auto path1 = path;

      path1.lineTo(p2.x(), by);
      path1.lineTo(p1.x(), by);

      path1.closeSubpath();

      painter->fillPath(path1, painter->brush());
    }

    if (isConnected())
      painter->strokePath(path, painter->pen());
  }

  for (const auto &point : points_) {
    auto c = pointToWindow(point);

    auto ll = Point::makeWindow(c.x.value - xr, c.y.value - yr);
    auto ur = Point::makeWindow(c.x.value + xr, c.y.value + yr);

    auto rect  = Rect(ll, ur);
    auto prect = canvas()->rectToPixel(rect).qrect();

    painter->drawEllipse(prect);
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

void
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
    Object::setValue(name, value, args);
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

void
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
    Object::setValue(name, value, args);
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

void
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
    Object::setValue(name, value, args);
}

QVariant
CsvObj::
exec(const QString &op, const QStringList &args)
{
  if (op == "load") {
    if (! csv_->load(filename_))
      return QVariant(0);

    return QVariant(1);
  }
  else
    return Object::exec(op, args);
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
  return QString("object.%1").arg(ind_);
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
    const auto &range = canvas()->displayRange();

    double x, y;
    range.pixelToWindow(p1.x.value, p1.y.value, &x, &y);

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
      app->errorMsg(QString("Invalid get name '%1'").arg(name));
      return QVariant();
    }
  }
  else {
    app->errorMsg(QString("Invalid get name '%1'").arg(name));
    return QVariant();
  }
}

void
Object::
setValue(const QString &name, const QString &value, const QStringList &)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  if      (name == "id")
    setId(value);
  else if (name == "visible")
    setVisible(Util::stringToBool(value));
  else if (name == "brush.color") {
    auto b = brush_.value();

    b.setColor(Util::stringToColor(value));

    brush_ = b;
  }
  else if (name == "brush.color.target") {
    auto b = brush_.target();

    b.setColor(Util::stringToColor(value));

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
    if (strs.size() != 4) return;

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
    pen_.setColor(Util::stringToColor(value));
  else if (name == "pen.width")
    pen_.setWidthF(Util::stringToReal(value));
  else if (name == "pen.dash")
    pen_.setDashPattern(stringToDashes(tcl, value));
  else if (name == "group") {
    auto *group = dynamic_cast<GroupObj *>(canvas()->getObjectByName(value));

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
      app->errorMsg(QString("Invalid set name '%1'").arg(name));
  }
  else
    app->errorMsg(QString("Invalid set name '%1'").arg(name));
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

}
