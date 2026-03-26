#ifndef CQSandboxShaderShape3DObj_H
#define CQSandboxShaderShape3DObj_H

#include <CQSandboxShape3DObj.h>

#include <CGLVector2D.h>
#include <CGLColor.h>

class CQGLTexture;

namespace CQSandbox {

class ShaderProgram;
class ShaderToyProgram;

class ShaderShape3DObj : public Object3D {
  Q_OBJECT

 public:
  struct VertexData {
    CGLVector3D position;
    CGLVector3D normal;
    CGLVector2D texCoord;

    VertexData(const CGLVector3D &position, const CGLVector3D &normal,
               const CGLVector2D &texCoord) :
     position(position), normal(normal), texCoord(texCoord) {
    }
  };

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  ShaderShape3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "ShaderShape"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  const CGLColor &color() const { return color_; }
  void setColor(const CGLColor &c) { color_ = c; }

  void setShaderToyTexture(const QString &filename);

  void init() override;

  void updateGL();

  bool intersect(const CGLVector3D &p1, const CGLVector3D &p2,
                 CPoint3D &pi1, CPoint3D &pi2) const override;

  CBBox3D calcBBox() override;

  void calcNormals();

  void preRender() override;

  void render() override;

 protected:
  using Points    = std::vector<CGLVector3D>;
  using Indices   = std::vector<unsigned int>;
  using Colors    = std::vector<CGLColor>;
  using TexCoords = std::vector<CGLVector2D>;

  struct ShaderToyData {
    CQGLTexture*      texture { nullptr };
    ShaderToyProgram* program { nullptr };
  };

  static ShaderProgram* s_program;

  CGLColor color_ { 1.0, 1.0, 1.0, 1.0 };

  Shape3DData shapeData_;

  Colors colors_;
  bool   wireframe_ { false };

  bool useTriangleStrip_ { false };
  bool useTriangleFan_   { false };

  unsigned int pointsBufferId_   { 0 };
  unsigned int normalsBufferId_  { 0 };
  unsigned int colorsBufferId_   { 0 };
  unsigned int texCoordBufferId_ { 0 };
  unsigned int vertexArrayId_    { 0 };
  unsigned int indBufferId_      { 0 };

  ShaderToyData shaderToyData_;

  int shaderWidth_  { 512 };
  int shaderHeight_ { 512 };
};

}

#endif
