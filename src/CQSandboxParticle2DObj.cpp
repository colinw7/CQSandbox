#include <CQSandboxParticle2DObj.h>
#include <CQSandboxParticleSystem.h>
#include <CQSandboxImage2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

#include <QPainter>

namespace CQSandbox {

bool
Particle2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  if (args.size() != 1) return false;

  auto *tcl = canvas->tcl();

  Point2D pos;
  if (! Util::stringToPoint2D(tcl, args[0], pos))
    return false;

  auto *obj = new Particle2DObj(canvas, pos);

  double mass = 1.0;

  auto *particle = dynamic_cast<Particle *>(
    canvas->psys()->makeParticle(mass, pos.x.value, pos.y.value));

  particle->setObj(obj);

  obj->setParticle(particle);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

Particle2DObj::
Particle2DObj(Canvas2D *canvas, const Point2D &pos) :
 Object2D(canvas, Type::PARTICLE), pos_(pos)
{
}

void
Particle2DObj::
setParticle(Particle *p)
{
  particle_ = p;

  particle_->setPosition(pos_.x.value, pos_.y.value, 0);
}

bool
Particle2DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "position") {
    auto *position = particle_->position();

    value = Util::point2DToString(Point2D(position->x(), position->y()));
  }
  else if (name == "velocity") {
    auto *velocity = particle_->velocity();

    value = Util::point2DToString(Point2D(velocity->x(), velocity->y()));
  }
  else if (name == "dead") {
    value = Util::boolToString(particle_->isDead());
  }
  else if (name == "age") {
    value = particle_->age();
  }
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
Particle2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (name == "position") {
    Point2D p;
    if (! Util::stringToPoint2D(tcl, value, p))
      return false;

    particle_->setPosition(p.x.value, p.y.value, 0);
  }
  else if (name == "velocity") {
    Point2D p;
    if (! Util::stringToPoint2D(tcl, value, p))
      return false;

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
    Point2D p;
    if (! Util::stringToPoint2D(tcl, value, p))
      return false;

    particle_->setTPos(CPoint2D(p.x.value, p.y.value));
  }
  else if (name == "tsize") {
    Point2D p;
    if (! Util::stringToPoint2D(tcl, value, p))
      return false;

    particle_->setTSize(CSize2D(p.x.value, p.y.value));
  }
  else if (name == "angle") {
    particle_->setAngle(Util::stringToReal(value));
  }
  else if (name == "color") {
    particle_->setColor(Util::QColorToRGBA(Util::stringToColor(tcl, value)));
  }
  else if (name == "alpha") {
    particle_->setAlpha(Util::stringToReal(value));
  }
  else if (name == "image") {
    QImage image;

    if (! Util::stringToImage(value, image)) {
      auto *obj = canvas()->getObjectByName(value);
      if (! obj) return app->errorMsg(QString("Failed to find object '%1'").arg(value));

      auto *imageObj = dynamic_cast<Image2DObj *>(obj);
      if (! obj) return false;

      particle_->setImage(imageObj->image());
    }
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

Rect2D
Particle2DObj::
calcRect() const
{
  auto *position = particle_->position();

  auto p = Point2D(position->x(), position->y());

  return Rect2D(p, p);
}

void
Particle2DObj::
draw(QPainter *)
{
}

}
