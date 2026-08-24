#include <CQSandboxQuadTree3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
QuadTree3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  if (args.size() != 0)
    return nullptr;

  auto *tcl = canvas->app()->tcl();

  auto *obj = new QuadTree3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

QuadTree3DObj::
QuadTree3DObj(Canvas3D *canvas) :
 Group3DObj(canvas, Type::QUAD_TREE)
{
}

void
QuadTree3DObj::
init()
{
  Object3D::init();
}

bool
QuadTree3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *tcl = canvas()->app()->tcl();

  if      (name == "object.in_rect") {
    if (args.size() < 1)
      return false;

    auto rect = Util::stringToRect(tcl, args[0]);

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

    auto p = Util::stringToPoint(tcl, args[0]);

    QuadTree::DataList dataList;
    quadTree_.getDataAtPoint(p.x.value, p.y.value, dataList);

    QStringList names;

    for (auto *obj : dataList)
      names.push_back(obj->getCommandName());

    value = names;
  }
  else
    return Group3DObj::getValue(name, args, value);

  return true;
}

bool
QuadTree3DObj::
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
    return Group3DObj::setValue(name, value, args);

  return true;
}

}
