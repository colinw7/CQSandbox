#ifndef CQSandboxGraph3DObj_H
#define CQSandboxGraph3DObj_H

#include <CQSandboxObject3D.h>

#include <CGLColor.h>

class CForceDirected3D;

namespace CDotParse {
class Parse;
}

namespace CQSandbox {

class Text3DObj;
class ShaderProgram;

class Graph3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Graph3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Graph"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void tick() override;

  void render() override;

 private:
  void addDemoNodes();

  void initSteps();

  bool loadDotFile(const QString &name);

  void updatePoints();

  void updateTextObjs();

 private:
  using Points   = std::vector<CGLVector3D>;
  using TextObjs = std::vector<Text3DObj *>;

  static ShaderProgram* s_program1;
  static ShaderProgram* s_program2;

  CForceDirected3D *forceDirected_ { nullptr };

  CDotParse::Parse *parse_ { nullptr };

  double stepSize_ { 0.01 };

  CGLColor lineColor_ { 1.0, 0.0, 0.0 };

  Points points_;
  Points linePoints_;

  TextObjs textObjs_;

  unsigned int pointsBufferId_ { 0 };
  unsigned int linesBufferId_  { 0 };
  unsigned int pointsArrayId_  { 0 };
  unsigned int linesArrayId_   { 0 };
};

}

#endif
