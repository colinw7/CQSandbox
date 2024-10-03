#ifndef CQSandboxControl3D_H
#define CQSandboxControl3D_H

#include <QFrame>

class CQPoint3DEdit;
class CQColorEdit;
class CQRealSpin;

class QListWidget;
class QListWidgetItem;
class QCheckBox;
class QComboBox;

namespace CQSandbox {

class Canvas3D;

class CanvasControl3D : public QFrame {
  Q_OBJECT

 public:
  CanvasControl3D(Canvas3D *canvas);

  void update();
  void updateLights();

 private Q_SLOTS:
  void depthTestSlot(int b);
  void cullFaceSlot(int b);
  void frontFaceSlot(int b);

  void bgColorSlot(const QColor &c);

  void nearSlot(double r);
  void farSlot(double r);

  void lightSelectedSlot(QListWidgetItem *, QListWidgetItem *);

  void lightCheckSlot(int b);
  void lightColorSlot(const QColor &c);
  void lightPosSlot();
  void lightDirSlot();
  void lightCutoffSlot(double);
  void lightRadiusSlot(double);

 private Q_SLOTS:
  void updateSlot();
  void closeSlot();

 private:
  Canvas3D* canvas_ { nullptr };

  QCheckBox*   depthTestCheck_ { nullptr };
  QCheckBox*   cullFaceCheck_  { nullptr };
  QCheckBox*   frontFaceCheck_ { nullptr };
  CQColorEdit* bgColorEdit_    { nullptr };
  CQRealSpin*  nearEdit_       { nullptr };
  CQRealSpin*  farEdit_        { nullptr };

  QListWidget*   lightsList_      { nullptr };
  QComboBox*     lightTypeCombo_  { nullptr };
  QCheckBox*     lightCheck_      { nullptr };
  CQColorEdit*   lightColorEdit_  { nullptr };
  CQPoint3DEdit* lightPosEdit_    { nullptr };
  CQPoint3DEdit* lightDirEdit_    { nullptr };
  CQRealSpin*    lightCutoffEdit_ { nullptr };
  CQRealSpin*    lightRadiusEdit_ { nullptr };

  CQRealSpin* ambientEdit_    { nullptr };
  CQRealSpin* diffuseEdit_    { nullptr };
  CQRealSpin* specularEdit_   { nullptr };
  CQRealSpin* shininessEdit_  { nullptr };
};

}

#endif
