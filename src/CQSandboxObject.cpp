#include <CQSandboxObject.h>
#include <CQSandboxCanvas.h>
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

bool
Object::
getValue(const QString &name, const QStringList &, QVariant &value)
{
  auto *app = canvas()->app();

  if      (name == "id")
    value = id();
  else if (name == "visible")
    value = isVisible();
  else if (name == "brush.color")
    value = Util::colorToString(brush_.value().color());
  else if (name == "brush.color.target")
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
  else if (name.left(5) == "user.")
    value = nameValue(name.mid(5));
  else if (name.left(8) == "animate.") {
    auto name1 = name.mid(8);

    if (name1 == "animating")
      value = isAnimating();
    else
      return app->errorMsg(QString("Invalid get name '%1' for '%2'").
               arg(name).arg(getCommandName()));
  }
  else if (name == "meta")
    value = meta_;
  else
    return app->errorMsg(QString("Invalid get name '%1' for '%2'").
             arg(name).arg(getCommandName()));

  return true;
}

bool
Object::
setValue(const QString &name, const QString &value, const QStringList &)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (name == "id")
    setId(value);
  else if (name == "visible")
    setVisible(Util::stringToBool(value));
  else if (name == "stroked")
    setStroked(Util::stringToBool(value));
  else if (name == "filled")
    setFilled(Util::stringToBool(value));
  else if (name == "brush.color" || name == "fill.color") {
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

}
