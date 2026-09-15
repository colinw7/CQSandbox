#ifndef CQSandboxText3DObj_H
#define CQSandboxText3DObj_H

#include <CQSandboxObject3D.h>
#include <CQSandboxShaderProgram.h>

#include <CGLVector2D.h>
#include <CGLVector3D.h>
#include <CGLColor.h>

namespace CQSandbox {

class FontData;

class Text3DObj : public Object3D {
  Q_OBJECT

  Q_PROPERTY(QString text READ text WRITE setText)
  Q_PROPERTY(double  size READ size WRITE setSize)

 public:
  struct GlyphInfo {
    CGLVector3D positions[4];
    CGLVector2D uvs[4];
    float       offsetX { 0 };
    float       offsetY { 0 };
  };

  class TextShaderProgram : public ShaderProgram {
   public:
    TextShaderProgram(QObject *parent) :
     ShaderProgram(parent) {
    }

    GLint posAttr        { 0 };
    GLint colAttr        { 0 };
    GLint texPosAttr     { 0 };
    GLint textureUniform { 0 };
  };

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Text3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "text"; }

  const QString &text() const { return text_; }
  void setText(const QString &s);

  const CGLColor &color() const { return color_; }
  void setColor(const CGLColor &c) { color_ = c; }

  double size() const { return size_; }
  void setSize(double r) { size_ = r; }

  const Qt::Alignment &align() const { return align_; }
  void setAlign(const Qt::Alignment &v) { align_ = v; }

  bool isRotated() const { return rotated_; }
  void setRotated(bool b) { rotated_ = b; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  CBBox3D calcBBox() override { return bbox_; }

  void render() override;

 private:
  void initShader();

  bool initFont();

  bool readFile(const char *path, std::vector<uint8_t> &bytes) const;

  void updateTextData();

  void initGLData();

  GlyphInfo makeGlyphInfo(uint32_t character, float offsetX, float offsetY) const;

  bool bindTexture();

  int textureId() const;

 private:
  struct GLData {
    GLuint   vao = 0;
    GLuint   vertexBuffer = 0;
    GLuint   uvBuffer = 0;
    GLuint   colBuffer = 0;
    GLuint   indexBuffer = 0;
    uint16_t indexElementCount = 0;
    float    angle { 0.0f };
  };

  static TextShaderProgram* s_program;
  static FontData*          s_fontData;

  QString       text_;
  CGLColor      color_   { 1.0, 1.0, 1.0 };
  double        size_    { 0.05 };
  Qt::Alignment align_   { Qt::AlignCenter };
  bool          rotated_ { false };

  bool textDataValid_ { false };

  std::vector<CGLVector3D> vertices_;
  std::vector<CGLVector2D> uvs_;
  std::vector<CGLColor>    colors_;
  std::vector<uint16_t>    indexes_;

  GLData glData_;
};

}

#endif
