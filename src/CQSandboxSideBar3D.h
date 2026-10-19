#ifndef CQSandboxSideBar3D_H
#define CQSandboxSideBar3D_H

#include <QFrame>

class CQIconButton;

namespace CQSandbox {

class Canvas3D;
class Overview3D;

class SideBar3D : public QFrame {
  Q_OBJECT

 public:
  SideBar3D(Canvas3D *canvas);

  Canvas3D *canvas() const { return canvas_; }

 private Q_SLOTS:
  void moveSlot  (bool state);
  void rotateSlot(bool state);
  void scaleSlot (bool state);

  void updateButtonState();

 private:
  void connectSlots(bool b);

 private:
  Canvas3D* canvas_ { nullptr };

  CQIconButton* moveButton_   { nullptr };
  CQIconButton* rotateButton_ { nullptr };
  CQIconButton* scaleButton_  { nullptr };
};

}

#endif
