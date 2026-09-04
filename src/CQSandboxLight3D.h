#ifndef CQSandboxLight3D_H
#define CQSandboxLight3D_H

#include <CGeomLight3D.h>

#include <QObject>

class CQGLBuffer;

namespace CQSandbox {

class Canvas3D;
class ShaderProgram;

class Light3D : public QObject, public CGeomLight3D {
  Q_OBJECT

  Q_PROPERTY(bool  enabled         READ getEnabled         WRITE setEnabled)
  Q_PROPERTY(float spotCutOffAngle READ getSpotCutOffAngle WRITE setSpotCutOffAngle)
  Q_PROPERTY(float pointRadius     READ getPointRadius     WRITE setPointRadius)

 public:
  Light3D(Canvas3D *canvas, const Type &type=Type::DIRECTIONAL);

  virtual ~Light3D();

  int id() const { return id_; }
  void setId(int i) { id_ = i; notifyChanged(); }

  void initBuffer();
  void initShader();

  void render();

  CQGLBuffer *getBuffer() const { return buffer_; }

 Q_SIGNALS:
  void changedSignal();

 private:
  void notifyChanged() override;

 private:
  static ShaderProgram* s_program;

  Canvas3D*   canvas_ { nullptr };
  int         id_     { 0 };
  CQGLBuffer* buffer_ { nullptr };
};

}

#endif
