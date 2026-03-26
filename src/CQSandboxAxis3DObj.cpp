#include <CQSandboxAxis3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxPath3DObj.h>
#include <CQSandboxText3DObj.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CInterval.h>

namespace CQSandbox {

bool
Axis3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Axis3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Axis3DObj::
Axis3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
  path_ = new Path3DObj(canvas);

  (void) canvas_->addNewObject(path_);

  path_->init();
}

void
Axis3DObj::
init()
{
  Object3D::init();
}

QVariant
Axis3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Axis3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

  if      (name == "start") {
    start_ = Util::stringToVector3D(tcl, value);

    setNeedsUpdate();
  }
  else if (name == "end") {
    end_ = Util::stringToVector3D(tcl, value);

    setNeedsUpdate();
  }
  else if (name == "min") {
    min_ = Util::stringToReal(value);

    setNeedsUpdate();
  }
  else if (name == "max") {
    max_ = Util::stringToReal(value);

    setNeedsUpdate();
  }
  else if (name == "auto_range") {
    if      (value == "x") {
      const auto &xrange = canvas_->xrange();

      start_ = CGLVector3D(-1, -0.5, -0.5);
      end_   = CGLVector3D( 1, -0.5, -0.5);
      min_   = xrange.min();
      max_   = xrange.max();
    }
    else if (value == "y") {
      const auto &yrange = canvas_->yrange();

      start_ = CGLVector3D(-0.5, -1, -0.5);
      end_   = CGLVector3D(-0.5,  1, -0.5);
      min_   = yrange.min();
      max_   = yrange.max();
    }
    else if (value == "z") {
      const auto &zrange = canvas_->zrange();

      start_ = CGLVector3D(-0.5, -0.5,  1);
      end_   = CGLVector3D(-0.5, -0.5, -1);
      min_   = zrange.min();
      max_   = zrange.max();
    }

    setNeedsUpdate();
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Axis3DObj::
tick()
{
  updateObjects();

  Object3D::tick();
}

void
Axis3DObj::
updateObjects()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  //---

  path_->setLine(start_, end_);

  CInterval interval(min_, max_);

  ticks_.clear();

  for (uint i = 0; i <= interval.calcNumMajor(); ++i) {
    double x = interval.interval(i);

    ticks_.push_back(x);
  }

  auto nt = ticks_.size();

  while (textObjs_.size() > nt) {
    auto *text = textObjs_.back();

    textObjs_.pop_back();

    canvas_->removeObject(text);

    delete text;
  }

  while (textObjs_.size() < nt) {
    auto *text = new Text3DObj(canvas_);

    (void) canvas_->addNewObject(text);

    text->init();

    textObjs_.push_back(text);
  }

  for (uint i = 0; i < nt; ++i)
    textObjs_[i]->setText(QString::number(ticks_[i]));

  updateModelMatrix();
}

void
Axis3DObj::
updateModelMatrix()
{
  setModelMatrix();

  //---

  auto setTextPos = [&](uint i, const CGLVector3D &p) {
    float x, y, z;
    modelMatrix_.multiplyPoint(p.x(), p.y(), p.z(), &x, &y, &z);

    auto p1 = CPoint3D(x, y, z);

    textObjs_[i]->setPosition(p1);
  };

  auto nt = ticks_.size();

  for (uint i = 0; i < nt; ++i) {
    auto r = CMathUtil::map(ticks_[i], min_, max_, 0.0, 1.0);

    auto x = CMathUtil::map(r, 0.0, 1.0, start_.getX(), end_.getX());
    auto y = CMathUtil::map(r, 0.0, 1.0, start_.getY(), end_.getY());
    auto z = CMathUtil::map(r, 0.0, 1.0, start_.getZ(), end_.getZ());

    setTextPos(i, CGLVector3D(x, y, z));
  }
}

void
Axis3DObj::
render()
{
}

}
