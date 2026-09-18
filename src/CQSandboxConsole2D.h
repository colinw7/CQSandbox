#ifndef CQSandboxConsole2D_H
#define CQSandboxConsole2D_H

#include <QFrame>

namespace CQSandbox {

class Canvas2D;

class Console2D : public QFrame {
  Q_OBJECT

 public:
  Console2D(Canvas2D *canvas);

  Canvas2D *canvas() const { return canvas_; }

  QSize sizeHint() const override;

 private Q_SLOTS:
  void executeCommand(const QString &line);

 private:
  Canvas2D* canvas_ { nullptr };

  QWidget *area_ { nullptr };
};

//---

}

#endif
