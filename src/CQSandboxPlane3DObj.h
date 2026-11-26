#ifndef CQSandboxPlane3DObj_H
#define CQSandboxPlane3DObj_H

#include <CQSandboxObject3D.h>

#include <CGLColor.h>
#include <CGLVector2D.h>
#include <CGLVector3D.h>

#include <QColor>

class CQGLTexture;

namespace CQSandbox {

class ShaderProgram;
class Texture;

class Plane3DObj : public Object3D {
  Q_OBJECT

  Q_PROPERTY(QColor  diffuseColor   READ diffuseColor       WRITE setDiffuseColor)
  Q_PROPERTY(QString diffuseTexture READ diffuseTextureFile WRITE setDiffuseTextureFile)

 public:
  using Points = std::vector<CGLVector3D>;

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  static ShaderProgram* s_shaderProgram() { return s_program; }

  //---

  Plane3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "plane"; }

  void init() override;

  ShaderProgram *shaderProgram();

  //---

  const Points &points() const { return points_; }

  //---

  QColor diffuseColor() const;
  void setDiffuseColor(const QColor &c);

  //---

  QString diffuseTextureFile() const;
  bool setDiffuseTextureFile(const QString &filename);

  Texture *diffuseTexture() const;
  void setDiffuseTexture(Texture *texture);

  //---

  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  //---

  void initShader();

  CBBox3D calcBBox() override;

  void loadBuffer() override;

  void render() override;

 private:
  void setTexCoords(const CPoint2D &p1, const CPoint2D &p2);

 protected:
  using Colors    = std::vector<CGLColor>;
  using TexCoords = std::vector<CGLVector2D>;

  static ShaderProgram* s_program;

  Points    points_;
  Colors    colors_;
  TexCoords texCoords_;
};

}

#endif
