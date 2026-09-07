#ifndef CQSandboxPointList3DObj_H
#define CQSandboxPointList3DObj_H

#include <CQSandboxObject3D.h>
#include <CQSandboxShaderProgram.h>

#include <CGLColor.h>
#include <CGLVector3D.h>

class CQGLTexture;

namespace CQSandbox {

class PointList3DObj : public Object3D {
  Q_OBJECT

  Q_PROPERTY(double pointSize READ pointSize WRITE setPointSize)

 public:
  using Points = std::vector<CGLVector3D>;
  using Colors = std::vector<CGLColor>;

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  PointList3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "point_list"; }

  double pointSize() const { return pointSize_; }
  void setPointSize(double r) { pointSize_ = r; setNeedsUpdate(); }

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

  void initData();

  void render() override;

  CBBox3D calcBBox() override;

 protected:
  void setNumPoints(int n);

 protected:
  class PointListShaderProgram : public ShaderProgram {
   public:
    PointListShaderProgram(QObject *parent) :
     ShaderProgram(parent) {
    }

    GLint positionAttr { 0 };
    GLint centerAttr   { 0 };
    GLint colorAttr    { 0 };
  };

  static size_t s_maxPoints;

  bool dataInit_ { false };

  static PointListShaderProgram *s_program;

  Points points_;
  Colors colors_;

  GLuint pointsPositionBuffer_ { 0 };
  GLuint pointsColorBuffer_    { 0 };
  GLuint billboardVertexBuffer_   { 0 };

  QString      textureFile_;
  CQGLTexture *texture_ { nullptr };

  double pointSize_ { 0.05 };
  bool   cullFace_  { false };
};

}

#endif
