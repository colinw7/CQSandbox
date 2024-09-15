#ifndef CQSandboxControl3D_H
#define CQSandboxControl3D_H

#include <QFrame>

namespace CQSandbox {

class Canvas3D;

class CanvasControl3D : public QFrame {
  Q_OBJECT

 public:
  CanvasControl3D(Canvas3D *canvas);

 private Q_SLOTS:
  void bgColorSlot(const QColor &c);
  void nearSlot(double r);
  void farSlot(double r);
  void lightColorSlot(const QColor &c);

 private:
  Canvas3D* canvas_ { nullptr };
};

}

#endif
