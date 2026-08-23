#ifndef CQSandboxParticleSystem_H
#define CQSandboxParticleSystem_H

#include <CPSysSystem.h>
#include <CPSysParticle.h>
#include <CRGBA.h>
#include <CSize2D.h>
#include <CPoint2D.h>

#include <QImage>

#include <optional>

namespace CQSandbox {

class Object;

class ParticleSystem : public CPSysSystem {
 public:
  ParticleSystem();

  CPSysParticle *makeParticle(double mass=1.0, double x=0.0, double y=0.0, double z=0.0) override;

  const Object *particleObj() const { return particleObj_; }
  void setParticleObj(Object *p) { particleObj_ = p; }

 private:
  Object* particleObj_ { nullptr };
};

class Particle : public CPSysParticle {
 public:
  using OptColor = std::optional<CRGBA>;
  using OptPoint = std::optional<CPoint2D>;
  using OptSize  = std::optional<CSize2D>;

 public:
  Particle(double mass=1.0);

  const OptColor &color() const { return color_; }
  void setColor(const OptColor &v) { color_ = v; }

  const QImage &image() const { return image_; }
  void setImage(const QImage &i) { image_ = i; }

  double size() const { return size_; }
  void setSize(double r) { size_ = r; }

  double alpha() const { return alpha_; }
  void setAlpha(double r) { alpha_ = r; }

  double angle() const { return angle_; }
  void setAngle(double r) { angle_ = r; }

  const OptPoint &tpos() const { return tpos_; }
  void setTPos(const OptPoint &v) { tpos_ = v; }

  const OptSize &tsize() const { return tsize_; }
  void setTSize(const OptSize &v) { tsize_ = v; }

  const std::string &meta() const { return meta_; }
  void setMeta(const std::string &s) { meta_ = s; }

  const Object *obj() const { return obj_; }
  void setObj(Object *p) { obj_ = p; }

  void updateParticle() override;

 private:
  OptColor    color_;
  QImage      image_;
  double      size_    { 1.0 };
  double      alpha_   { 1.0 };
  double      angle_   { 0.0 };
  OptPoint    tpos_;
  OptSize     tsize_;
  std::string meta_;

  Object *obj_ { nullptr };
};

}

#endif
