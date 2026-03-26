#ifndef CQSandboxOthello3DObj_H
#define CQSandboxOthello3DObj_H

#include <CQSandboxObject3D.h>

#ifdef CQSANDBOX_OTHELLO
class COthelloBoard;
#endif

namespace CQSandbox {

#ifdef CQSANDBOX_OTHELLO
class Othello3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Othello3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Othello"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void render() override;

 private:
  COthelloBoard* board_ { nullptr };
};
#endif

}

#endif
