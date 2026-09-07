#ifndef CQSandboxLineList3DObj_H
#define CQSandboxLineList3DObj_H

#include <CQSandboxObject3D.h>

#include <CGLColor.h>
#include <CLine3D.h>

namespace CQSandbox {

class ShaderProgram;

class LineList3DObj : public Object3D {
  Q_OBJECT

  Q_PROPERTY(double lineWidth READ lineWidth WRITE setLineWidth)

 public:
  using Lines  = std::vector<CLine3D>;
  using Colors = std::vector<CGLColor>;

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  LineList3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "line_list"; }

  double lineWidth() const { return lineWidth_; }
  void setLineWidth(double r) { lineWidth_ = r; setNeedsUpdate(); }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  const Lines &points() const { return lines_; }
  void setLines(const Lines &lines);

  const Colors &colors() const { return colors_; }

  void init() override;

  void updateGL();

  void initShader();

  void render() override;

  CBBox3D calcBBox() override;

 protected:
  void setNumLines(int n);

 protected:
  static ShaderProgram* s_program;

  Lines  lines_;
  Colors colors_;

  double lineWidth_ { 0.05 };
};

}

#endif
