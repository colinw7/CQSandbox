#ifndef CQSandboxToolbar2D_H
#define CQSandboxToolbar2D_H

#include <QFrame>

class QToolButton;
class QLabel;

namespace CQSandbox {

class Canvas2D;

class Toolbar2D : public QFrame {
  Q_OBJECT

 public:
  Toolbar2D(Canvas2D *canvas);

  Canvas2D *canvas() const { return canvas_; }

  void setInfo(const QString &label);

 private Q_SLOTS:
  void playSlot();
  void pauseSlot();
  void stepSlot();

  void settingsSlot();
  void consoleSlot();

 private:
  Canvas2D* canvas_ { nullptr };

  QToolButton* playButton_  { nullptr };
  QToolButton* pauseButton_ { nullptr };
  QToolButton* stepButton_  { nullptr };

  QToolButton *settingsButton_ { nullptr };
  QToolButton *consoleButton_  { nullptr };

  QLabel* infoLabel_ { nullptr };
};

//---

}

#endif
