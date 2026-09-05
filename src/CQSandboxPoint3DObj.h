#ifndef CQSandboxPoint3DObj_H
#define CQSandboxPoint3DObj_H

#include <CQSandboxObject3D.h>

namespace CQSandbox {

class ShaderProgram;

//---

class Point3DObjMgr : public ObjectMgr3D {
 public:
  Point3DObjMgr() { }

  const char *typeName() const override { return "point"; }

  void initRender(Canvas3D *canvas) override;
  void termRender(Canvas3D *canvas) override;
};

//---

class Point3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  static ShaderProgram* shaderProgram() { return s_program; }

  static void initShader(Canvas3D *canvas);

  static void initDraw(Canvas3D *canvas);
  static void termDraw(Canvas3D *canvas);

  //---

  Point3DObj(Canvas3D *canvas);

  //--

  const char *typeName() const override { return "point"; }

  virtual ObjectMgr3D *mgr() override { return s_objectMgr; }

  //---

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void updateGL();

  CBBox3D calcBBox() override;

  void render() override;

 protected:
  static ShaderProgram* s_program;
  static Point3DObjMgr* s_objectMgr;

  double size_  { 3 };
  QColor color_ { Qt::red };
};

}

#endif
