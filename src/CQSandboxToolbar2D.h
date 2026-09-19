#ifndef CQSandboxToolbar2D_H
#define CQSandboxToolbar2D_H

#include <QFrame>

class QToolButton;
class QLabel;

namespace CQSandbox {

class Canvas2D;

class CanvasToolbar2D : public QFrame {
  Q_OBJECT

 public:
  CanvasToolbar2D(Canvas2D *canvas);

  Canvas2D *canvas() const { return canvas_; }

  void setInfo(const QString &label);
  void setPos(const QString &label);

  void connectSlots(bool b);

 private Q_SLOTS:
  void updateInfo();

  void cameraSlot();
  void modelSlot();
  void gameSlot();

  void playSlot();
//void pauseSlot();
  void stepSlot();

  void settingsSlot();
  void consoleSlot();

#ifdef CQSANDBOX_META_EDIT
  void metaEditSlot();
#endif
  void performanceSlot();

 private:
  Canvas2D* canvas_ { nullptr };

  QToolButton* cameraButton_ { nullptr };
  QToolButton* modelButton_  { nullptr };
  QToolButton* gameButton_   { nullptr };

  QToolButton* playButton_  { nullptr };
//QToolButton* pauseButton_ { nullptr };
  QToolButton* stepButton_  { nullptr };

  QToolButton *settingsButton_ { nullptr };
  QToolButton *consoleButton_  { nullptr };

  QLabel* infoLabel_ { nullptr };
  QLabel* posLabel_ { nullptr };
};

//---

}

#endif
