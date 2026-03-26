#ifndef CQSandboxCube3DObj_H
#define CQSandboxCube3DObj_H

#include <CQSandboxShape3DObj.h>

namespace CQSandbox {

class Cube3DObj : public Shape3DObj {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Cube3DObj(Canvas3D *canvas);

  void init() override;
};

}

#endif
