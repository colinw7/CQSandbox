#ifndef CQSandboxParticleList3DObj_H
#define CQSandboxParticleList3DObj_H

#include <CQSandboxObject3D.h>
#include <CQSandboxShaderProgram.h>

#include <CGLColor.h>
#include <CGLVector3D.h>

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
  enum class Shape {
    PLANE,
    CUBE
  };

  using Points  = std::vector<CGLVector3D>;
  using Normals = std::vector<CGLVector3D>;
  using Colors  = std::vector<CGLColor>;

  struct ParticleShape {
    Points  points;
    Normals normals;
    bool    flat { false };
  };

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  ParticleList3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "particle_list"; }

  const Shape &shape() const { return shape_; }
  void setShape(const Shape &v) { shape_ = v; }

  double particleSize() const { return particleSize_; }
  void setParticleSize(double r) { particleSize_ = r; setNeedsUpdate(); }

  double particleAlpha() const { return particleAlpha_; }
  void setParticleAlpha(double r) { particleAlpha_ = r; setNeedsUpdate(); }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  const Points &points() const { return points_; }
  void setPoints(const Points &points);

  const Colors &colors() const { return colors_; }

  const QString &textureFile() const { return textureFile_; }
  void setTextureFile(const QString &filename);

  bool isCullFace() const { return cullFace_; }
  void setCullFace(bool b) { cullFace_ = b; }

  void init() override;

  void initShader();

  const ParticleShape &getParticleShape() const;
  void addParticlePoint(const CGLVector3D &v);

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
    GLint normalAttr   { 0 };
    GLint centerAttr   { 0 };
    GLint colorAttr    { 0 };
  };

  //--

  static size_t s_maxShape;
  static size_t s_maxPoints;

  static ParticleListShaderProgram *s_program;

  Points points_;
  Colors colors_;

  GLuint billboardPointsBuffer_  { 0 };
  GLuint billboardNormalsBuffer_ { 0 };

  GLuint particlesPositionBuffer_ { 0 };
  GLuint particlesColorBuffer_    { 0 };

  QString      textureFile_;
  CQGLTexture *texture_ { nullptr };

#ifdef CQSANDBOX_FLOCKING
  CFlocking* flocking_ { nullptr };
#endif

#ifdef CQSANDBOX_FIREWORKS
  CFireworks* fireworks_ { nullptr };
#endif

  double particleSize_  { 0.05 };
  double particleAlpha_ { 0.5 };
  bool   cullFace_      { false };

  Shape         shape_ { Shape::CUBE };
  ParticleShape particleShape_;
  bool          particleFlat_ { false };
};

}

#endif
