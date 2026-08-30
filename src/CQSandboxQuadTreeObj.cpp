#include <CQSandboxQuadTreeObj.h>
#include <CQSandboxCanvas.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
QuadTreeObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 0)
    return false;

  auto *tcl = canvas->tcl();

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

bool
QuadTreeObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *tcl = canvas()->tcl();

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
    return GroupObj::getValue(name, args, value);

  return true;
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

}
