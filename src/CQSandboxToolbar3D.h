#ifndef CQSandboxToolbar3D_H
#define CQSandboxToolbar3D_H

#include <QFrame>

class QToolButton;
class QLabel;

namespace CQSandbox {

class Canvas3D;
class Overview3D;

class CanvasToolbar3D : public QFrame {
  Q_OBJECT

 public:
  CanvasToolbar3D(Canvas3D *canvas);

  Canvas3D *canvas() const { return canvas_; }

  QToolButton *addDebugButton();

  void setInfo(const QString &label);

  void setPos(const QString &label);

 public Q_SLOTS:
  void updateInfo();

  void cameraSlot();
  void modelSlot();
  void lightSlot();
  void gameSlot();

  void wireSlot();
  void solidSlot();
  void texturedSlot();

  void bboxSlot();

  void settingsSlot();

  void metaEditSlot();
  void performanceSlot();
  void optionsSlot();

 private:
  Canvas3D* canvas_ { nullptr };

  QToolButton* cameraButton_ { nullptr };
  QToolButton* modelButton_  { nullptr };
  QToolButton* lightButton_  { nullptr };
  QToolButton* gameButton_   { nullptr };

  QToolButton* wireButton_     { nullptr };
  QToolButton* solidButton_    { nullptr };
  QToolButton* texturedButton_ { nullptr };

  QToolButton* bboxButton_ { nullptr };

  QToolButton* settingsButton_ { nullptr };

  QLabel* infoLabel_ { nullptr };
  QLabel* posLabel_  { nullptr };
};

//---

class OverviewToolbar3D : public QFrame {
  Q_OBJECT

 public:
  OverviewToolbar3D(Overview3D *overview);

  Overview3D *overview() const { return overview_; }

 public Q_SLOTS:
  void updateInfo();

  void cameraSlot();
  void modelSlot();
  void lightSlot();

  void objectSelectSlot();
  void faceSelectSlot();
  void edgeSelectSlot();
  void pointSelectSlot();

  void settingsSlot();

 private:
  Overview3D* overview_ { nullptr };

  QToolButton* cameraButton_ { nullptr };
  QToolButton* modelButton_  { nullptr };
  QToolButton* lightButton_  { nullptr };

  QToolButton* objectSelectButton_ { nullptr };
  QToolButton* faceSelectButton_   { nullptr };
  QToolButton* edgeSelectButton_   { nullptr };
  QToolButton* pointSelectButton_  { nullptr };

  QToolButton* settingsButton_ { nullptr };

  QLabel* infoLabel_ { nullptr };
  QLabel* posLabel_  { nullptr };
};

}

#endif
