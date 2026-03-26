#ifndef CQSandboxShape3DObj_H
#define CQSandboxShape3DObj_H

#include <CQSandboxObject3D.h>
#include <CQSandboxShape3DData.h>

#include <CGLVector2D.h>
#include <CGLColor.h>

class CQGLTexture;
class CShape3D;

namespace CQSandbox {

class ShaderProgram;

class Shape3DObj : public Object3D {
  Q_OBJECT

  Q_PROPERTY(QString texture READ textureFile WRITE setTextureFile)

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Shape3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Shape"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  const CGLColor &color() const { return color_; }
  void setColor(const CGLColor &c) { color_ = c; }

  const QString &textureFile() const { return textureFile_; }
  void setTextureFile(const QString &filename);

  void setTexture(CQGLTexture *texture) { texture_ = texture; }

  void setNormalTexture(const QString &filename);
  void setNormalTexture(CQGLTexture *texture) { normalTexture_ = texture; }

  void init() override;

  void updateGL();

  bool intersect(const CGLVector3D &p1, const CGLVector3D &p2,
                 CPoint3D &pi1, CPoint3D &pi2) const override;

  CBBox3D calcBBox() override;

  void calcNormals();

  void render() override;

  void addCube(double sx, double sy, double sz);

 protected:
  using Colors = std::vector<CGLColor>;

  static ShaderProgram* s_program;

  CGLColor color_ { 1.0, 1.0, 1.0, 1.0 };

  Shape3DData shapeData_;

  Colors colors_;
  bool   wireframe_ { false };

  QString      textureFile_;
  CQGLTexture *texture_       { nullptr };
  CQGLTexture *normalTexture_ { nullptr };

  bool useDiffuseTexture_ { false };
  bool useNormalTexture_  { false };
  bool useTriangleStrip_  { false };
  bool useTriangleFan_    { false };

  unsigned int pointsBufferId_   { 0 };
  unsigned int normalsBufferId_  { 0 };
  unsigned int colorsBufferId_   { 0 };
  unsigned int texCoordBufferId_ { 0 };
  unsigned int vertexArrayId_    { 0 };
  unsigned int indBufferId_      { 0 };
};

}

#endif
