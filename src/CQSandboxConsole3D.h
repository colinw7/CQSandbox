#ifndef CQSandboxConsole3D_H
#define CQSandboxConsole3D_H

#include <QFrame>

namespace CQSandbox {

class Canvas3D;

class Console3D : public QFrame {
  Q_OBJECT

 public:
  Console3D(Canvas3D *canvas);

  Canvas3D *canvas() const { return canvas_; }

  QSize sizeHint() const override;

 private Q_SLOTS:
  void executeCommand(const QString &line);

 private:
  Canvas3D* canvas_ { nullptr };

  QWidget *area_ { nullptr };
};

//---

}

#endif
