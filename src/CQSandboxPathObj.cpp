#include <CQSandboxPathObj.h>
#include <CQSandboxCanvas.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

#include <QPainter>

namespace CQSandbox {

bool
PathObj::
create(Canvas *canvas, const QStringList &args)
{
  auto *tcl = canvas->tcl();

  QPainterPath path;

  if (args.size() >= 1)
    path = Util::stringToPath(args[0]);

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

bool
PathObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if (name == "path")
    value = Util::pathToString(path_);
  else
    return Object::getValue(name, args, value);

  return true;
}

bool
PathObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if (name == "path") {
    path_ = Util::stringToPath(value);
  }
  else
    return Object::setValue(name, value, args);

  return true;
}

bool
PathObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  auto *tcl = canvas()->tcl();

  if      (op == "moveTo") {
    if (args.size() < 1)
      return false;

    CPoint2D p;
    if (! Util::stringToPoint2D(tcl, args[0], p))
      return false;

    path_.moveTo(p.x, p.y);
  }
  else if (op == "lineTo") {
    if (args.size() < 1)
      return false;

    CPoint2D p;
    if (! Util::stringToPoint2D(tcl, args[0], p))
      return false;

    path_.lineTo(p.x, p.y);
  }
  else if (op == "curveTo") {
    if (args.size() < 2)
      return false;

    CPoint2D p1;
    if (! Util::stringToPoint2D(tcl, args[0], p1))
      return false;

    CPoint2D p2;
    if (! Util::stringToPoint2D(tcl, args[1], p2))
      return false;

    if (args.size() > 2) {
      CPoint2D p3;
      if (! Util::stringToPoint2D(tcl, args[2], p3))
        return false;

      path_.cubicTo(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
    }
    else
      path_.quadTo(p1.x, p1.y, p2.x, p2.y);
  }
  else
    return Object::exec(op, args, res);

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
  auto pointToPixel = [&](const QPointF &p) {
    auto p1 = canvas_->pointToPixel(Point(p.x(), p.y()));
    return QPointF(p1.x.value, p1.y.value);
  };

  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  QPainterPath path;

  int n = path_.elementCount();

  QPointF lastP, nextP;

  for (int i = 0; i < n; ++i) {
    auto e = path_.elementAt(i);

    if      (e.isMoveTo()) {
      QPointF p(e.x, e.y);

      if (i < n - 1) {
        auto e1 = path_.elementAt(i + 1);

        nextP = QPointF(e1.x, e1.y);
      }
      else
        nextP = p;

      path.moveTo(pointToPixel(p));

      lastP = p;
    }
    else if (e.isLineTo()) {
      QPointF p(e.x, e.y);

      if (i < n - 1) {
        auto e1 = path_.elementAt(i + 1);

        nextP = QPointF(e1.x, e1.y);
      }
      else
        nextP = p;

      path.lineTo(pointToPixel(p));

      lastP = p;
    }
    else if (e.isCurveTo()) {
      QPointF p(e.x, e.y);

      QPointF p1, p2;

      QPainterPath::ElementType e1t { QPainterPath::MoveToElement };
      QPainterPath::ElementType e2t { QPainterPath::MoveToElement };

      if (i < n - 1) {
        auto e1 = path_.elementAt(i + 1);

        e1t = e1.type;

        p1 = QPointF(e1.x, e1.y);
      }

      if (i < n - 2) {
        auto e2 = path_.elementAt(i + 2);

        e2t = e2.type;

        p2 = QPointF(e2.x, e2.y);
      }

      if (e1t == QPainterPath::CurveToDataElement) {
        ++i;

        if (e2t == QPainterPath::CurveToDataElement) {
          ++i;

          if (i < n - 1) {
            auto e3 = path_.elementAt(i + 1);

            nextP = QPointF(e3.x, e3.y);
          }
          else
            nextP = p;

          path.cubicTo(pointToPixel(p), pointToPixel(p1), pointToPixel(p2));

          lastP = p;
        }
        else {
          if (i < n - 1) {
            auto e3 = path_.elementAt(i + 1);

            nextP = QPointF(e3.x, e3.y);
          }
          else
            nextP = p;

          path.quadTo(pointToPixel(p), pointToPixel(p1));

          lastP = p;
        }
      }
    }
  }

  painter->drawPath(path);
}

}
