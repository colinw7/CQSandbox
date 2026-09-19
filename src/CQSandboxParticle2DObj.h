#ifndef CQSandboxParticle2DObj_H
#define CQSandboxParticle2DObj_H

#include <CQSandboxObject2D.h>

#include <QPainterPath>

namespace CQSandbox {

class Particle;

class Particle2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  Particle2DObj(Canvas2D *canvas, const Point2D &pos);

  const char *typeName() const override { return "particle"; }

  const Particle *particle() const { return particle_; }
  void setParticle(Particle *p);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point2D   pos_;
  Particle* particle_ { nullptr };
};

}

#endif
