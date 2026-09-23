#include <CQSandboxQuadTree2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
QuadTree2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  if (args.size() != 0)
    return false;

  auto *tcl = canvas->tcl();

  auto *obj = new QuadTree2DObj(canvas);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

QuadTree2DObj::
QuadTree2DObj(Canvas2D *canvas) :
 Group2DObj(canvas, Rect2D())
{
}

bool
QuadTree2DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *tcl = canvas()->tcl();

  if      (name == "object.in_rect") {
    if (args.size() < 1)
      return false;

    Rect2D rect;
    if (! Util::stringToRect2D(tcl, args[0], rect))
      return false;

    QuadTree::DataList dataList;
    quadTree_.getDataInsideBBox(rect, dataList);

    QStringList names;

    for (auto *obj : dataList)
      names.push_back(obj->getCommandName());

    value = names;
  }
  else if (name == "object.at_point") {
    if (args.size() < 1)
      return false;

    Point2D p;
    if (! Util::stringToPoint2D(tcl, args[0], p))
      return false;

    QuadTree::DataList dataList;
    quadTree_.getDataAtPoint(p.x.value, p.y.value, dataList);

    QStringList names;

    for (auto *obj : dataList)
      names.push_back(obj->getCommandName());

    value = names;
  }
  else
    return Group2DObj::getValue(name, args, value);

  return true;
}

bool
QuadTree2DObj::
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
    return Group2DObj::setValue(name, value, args);

  return true;
}

}
