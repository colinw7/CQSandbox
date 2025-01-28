#ifndef CQSandboxToolbar2D_H
#define CQSandboxToolbar2D_H

#include <QFrame>

class QToolButton;
class QLabel;

namespace CQSandbox {

class Canvas;

class Toolbar2D : public QFrame {
  Q_OBJECT

 public:
  Toolbar2D(Canvas *canvas);

  Canvas *canvas() const { return canvas_; }

  void setInfo(const QString &label);

 private Q_SLOTS:
  void playSlot();
  void pauseSlot();
  void stepSlot();

  void settingsSlot();

 private:
  Canvas* canvas_ { nullptr };

  QToolButton* playButton_  { nullptr };
  QToolButton* pauseButton_ { nullptr };
  QToolButton* stepButton_  { nullptr };

  QToolButton *settingsButton_ { nullptr };

  QLabel* infoLabel_ { nullptr };
};

//---

}

#endif
