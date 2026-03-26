#ifndef CQSandboxParticleList3DObj_H
#define CQSandboxParticleList3DObj_H

#include <CQSandboxObject3D.h>
#include <CQSandboxShaderProgram.h>

#include <CGLColor.h>

#ifdef CQSANDBOX_FLOCKING
class CFlocking;
#endif

#ifdef CQSANDBOX_FIREWORKS
class CFireworks;
#endif

class CQGLTexture;

namespace CQSandbox {

class ParticleList3DObj : public Object3D {
  Q_OBJECT

  Q_PROPERTY(double particleSize READ particleSize WRITE setParticleSize)

 public:
  using Points = std::vector<CGLVector3D>;

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  ParticleList3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "ParticleList"; }

  double particleSize() const { return particleSize_; }
  void setParticleSize(double r) { particleSize_ = r; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void setPoints(const Points &points);

  const QString &textureFile() const { return textureFile_; }
  void setTextureFile(const QString &filename);

  void init() override;

  void tick() override;

  void render() override;

  CBBox3D calcBBox() override;

 protected:
  void setNumPoints(int n);

#ifdef CQSANDBOX_FLOCKING
  void updateFlocking();
#endif

#ifdef CQSANDBOX_FIREWORKS
  void updateFireworks();
#endif

 protected:
  class ParticleListShaderProgram : public ShaderProgram {
   public:
    ParticleListShaderProgram(QObject *parent) :
     ShaderProgram(parent) {
    }

    GLint positionAttr { 0 };
    GLint centerAttr   { 0 };
    GLint colorAttr    { 0 };
  };

  static size_t s_maxPoints;

  static ParticleListShaderProgram *s_program;

  using Colors = std::vector<CGLColor>;

  Points points_;
  Colors colors_;

  GLuint particlesPositionBuffer_ { 0 };
  GLuint particlesColorBuffer_    { 0 };
  GLuint billboardVertexBuffer_   { 0 };

  QString      textureFile_;
  CQGLTexture *texture_ { nullptr };

#ifdef CQSANDBOX_FLOCKING
  CFlocking* flocking_ { nullptr };
#endif

#ifdef CQSANDBOX_FIREWORKS
  CFireworks* fireworks_ { nullptr };
#endif

  double particleSize_ { 0.05 };
};

}

#endif
