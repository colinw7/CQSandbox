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
  if (args.size() != 1) return false;

  auto *tcl = canvas->app()->tcl();

  auto path = Util::stringToPath(args[0]);

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

}
