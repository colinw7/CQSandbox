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

class Plane3DObj : public Object3D {
  Q_OBJECT

  Q_PROPERTY(QColor  color   READ color       WRITE setColor      )
  Q_PROPERTY(QString texture READ textureFile WRITE setTextureFile)

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Plane3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "plane"; }

  const QColor &color() const { return color_; }
  void setColor(const QColor &c);

  const QString &textureFile() const { return textureFile_; }
  void setTextureFile(const QString &filename);

  void setTexture(CQGLTexture *texture) { texture_ = texture; }

  void init() override;

  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void initShader();

  void updateGL();

  CBBox3D calcBBox() override;

  void render() override;

 protected:
  using Points    = std::vector<CGLVector3D>;
  using Colors    = std::vector<CGLColor>;
  using TexCoords = std::vector<CGLVector2D>;

  static ShaderProgram* s_program;

  QColor color_ { Qt::white };

  QString      textureFile_;
  CQGLTexture *texture_    { nullptr };
  bool         useTexture_ { false };

  Points    points_;
  Colors    colors_;
  TexCoords texCoords_;
};

}

#endif
