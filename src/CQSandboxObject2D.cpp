#include <CQSandboxObject2D.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxViewport.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

namespace CQSandbox {

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

namespace CQSandbox {

Object2D::
Object2D(Canvas2D *canvas, Type type) :
 canvas_(canvas), type_(type)
{
  pen_   = canvas->stylePen();
  brush_ = canvas->styleBrush();
}

QString
Object2D::
getCommandName() const
{
  return QString("sb2d::%1.%2").arg(typeName()).arg(ind_);
}

//---

const AnimatePoint2D &
Object2D::
position() const
{
  return position_;
}

void
Object2D::
setPosition(const AnimatePoint2D &p)
{
  position_ = p;

  canvas_->update();
}

//---

Point2D
Object2D::
pointToWindow(const Point2D &p) const
{
  auto p1 = p;

  auto *group = this->group();

  while (group) {
    const auto &range = group->displayRange();

    double x, y;
    range.windowToPixel(p1.x.value, p1.y.value, &x, &y);

    p1 = Point2D::makeWindow(x, y);

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

    return Point2D::makeWindow(x, y);
  }
  else
    return Point2D::makeWindow(p1.point());
}

Point2D
Object2D::
pointToPixel(const Point2D &p) const
{
  if (group_) {
    double px, py;
    group_->displayRange().windowToPixel(p.x.value, p.y.value, &px, &py);

    return canvas()->pointToPixel(Point2D::makeWindow(px, py));
  }
  else
    return canvas()->pointToPixel(p);
}

Rect2D
Object2D::
rectToWindow(const Rect2D &r) const
{
  auto p1 = pointToWindow(r.ll);
  auto p2 = pointToWindow(r.ur);

  return Rect2D(p1, p2);
}

QString
Object2D::
calcId() const
{
  auto id = this->id();

  if (id == "")
    id = getCommandName();

  return id;
}

//---

void
Object2D::
init()
{
}

bool
Object2D::
getValue(const QString &name, const QStringList &, QVariant &value)
{
  auto *app = canvas()->app();

  if      (name == "id")
    value = id();

  // state
  else if (name == "visible") {
    value = isVisible();
  }
  else if (name == "selected") {
    value = isSelected();
  }
  else if (name == "layer") {
    value = layer();
  }

  // brush/pen
  else if (name == "brush.color")
    value = Util::colorToString(brush_.value().color());
  else if (name == "brush.target.color")
    value = Util::colorToString(brush_.target().color());
  else if (name == "brush.alpha")
    value = Util::realToString(brush_.value().color().alphaF());
  else if (name == "brush.steps")
    value = Util::colorToString(brush_.steps());
  else if (name == "pen.color")
    value = Util::colorToString(pen_.color());
  else if (name == "pen.width")
    value = Util::realToString(pen_.widthF());

  else if (name == "group")
    value = (group() ? group()->calcId() : "");

  else if (name == "meta")
    value = meta_;
  else if (name.left(5) == "user.")
    value = nameValue(name.mid(5));

  // animate
  else if (name.left(8) == "animate.") {
    auto name1 = name.mid(8);

    if (name1 == "animating")
      value = isAnimating();
    else
      return app->errorMsg(QString("Invalid get name '%1' for '%2'").
               arg(name).arg(getCommandName()));
  }

  else if (name == "position") {
    value = Util::point2DToString(Point2D::makeWindow(position().value()));
  }

  else
    return app->errorMsg(QString("Invalid get name '%1' for '%2'").
             arg(name).arg(getCommandName()));

  return true;
}

bool
Object2D::
setValue(const QString &name, const QString &value, const QStringList &)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (name == "id")
    setId(value);

  // state
  else if (name == "visible") {
    bool b;
    if (! Util::stringToBool(value, b))
      return false;

    setVisible(b);
  }
  else if (name == "selected") {
    bool b;
    if (! Util::stringToBool(value, b))
      return false;

    setSelected(b);
  }
  else if (name == "layer") {
    int i;
    if (! Util::stringToInt(value, i))
      return false;

    setLayer(i);
  }

  else if (name == "stroked") {
    setStroked(Util::stringToBool(value));
  }
  else if (name == "filled") {
    setFilled(Util::stringToBool(value));
  }

  // brush/pen
  else if (name == "brush.color" || name == "fill.color") {
    auto b = brush_.value();

    b.setColor(Util::stringToColor(tcl, value));

    brush_ = b;
  }
  else if (name == "brush.target.color") {
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
    auto *group = dynamic_cast<Group2DObj *>(canvas()->getObjectByName(value));
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

  else if (name == "meta") {
    meta_ = value;
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

  else if (name == "position") {
    Point2D p;
    if (! Util::stringToPoint2D(tcl, value, p))
      return false;

    setPosition(canvas()->pointToWindow(p).point());
  }

  else
    return app->errorMsg(QString("Invalid set name '%1' for '%2'").
               arg(name).arg(getCommandName()));

  return true;
}

bool
Object2D::
exec(const QString &, const QStringList &, QVariant &)
{
  return false;
}

bool
Object2D::
getTclValue(const QString &name, const TclObjs &args, Tcl_Obj* &res)
{
  auto *tcl = canvas()->tcl();

  QStringList args1;
  for (auto *arg : args)
    args1.push_back(tcl->variantFromObj(arg).toString());

  QVariant res1;
  if (! getValue(name, args1, res1))
    return false;

  res = tcl->variantToObj(res1);

  return true;
}

bool
Object2D::
setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &args)
{
  auto *tcl = canvas()->tcl();

  QStringList args1;
  for (auto *arg : args)
    args1.push_back(tcl->variantFromObj(arg).toString());

  if (! setValue(name, tcl->qstringFromObj(value), args1))
    return false;

  return true;
}

bool
Object2D::
execTcl(const QString &op, const TclObjs &args, Tcl_Obj* &res)
{
  auto *tcl = canvas()->tcl();

  QStringList args1;
  for (auto *arg : args)
    args1.push_back(tcl->variantFromObj(arg).toString());

  QVariant res1;
  if (! exec(op, args1, res1))
    return false;

  res = tcl->variantToObj(res1);

  return true;
}

//---

bool
Object2D::
step()
{
  return brush_.step();
}

void
Object2D::
move(int dx, int dy)
{
  auto ppos = canvas()->pointToPixel(Point2D::makeWindow(position().value())).point();

  ppos += CPoint2D(dx, dy);

  setPosition(canvas()->pointToWindow(Point2D::makePixel(ppos)).point());
}

void
Object2D::
press(int, int)
{
  //std::cerr << "Press: " << calcId().toStdString() << "\n";
}

void
Object2D::
click(int, int)
{
  //std::cerr << "Click: " << calcId().toStdString() << "\n";
}

}
