#ifndef CQSandboxToolbar3D_H
#define CQSandboxToolbar3D_H

#include <QFrame>

class QToolButton;
class QLabel;

namespace CQSandbox {

class Canvas3D;

class Toolbar3D : public QFrame {
  Q_OBJECT

 public:
  Toolbar3D(Canvas3D *canvas);

  Canvas3D *canvas() const { return canvas_; }

  void setInfo(const QString &label);

  void setPos(const QString &label);

 public Q_SLOTS:
  void updateInfo();

  void cameraSlot();
  void modelSlot();
  void lightSlot();
  void wireSlot();
  void bboxSlot();
  void settingsSlot();

 private:
  Canvas3D* canvas_ { nullptr };

  QToolButton* cameraButton_   { nullptr };
  QToolButton* modelButton_    { nullptr };
  QToolButton* lightButton_    { nullptr };
  QToolButton* wireButton_     { nullptr };
  QToolButton* bboxButton_     { nullptr };
  QToolButton* settingsButton_ { nullptr };
  QLabel*      infoLabel_      { nullptr };
  QLabel*      posLabel_       { nullptr };
};

}

#endif
