#ifndef CQSandboxControl3D_H
#define CQSandboxControl3D_H

#include <QFrame>

class CQPoint3DEdit;
class CQColorEdit;
class CQRealSpin;
class CQPropertyViewTree;

class QListWidget;
class QListWidgetItem;
class QCheckBox;
class QComboBox;
class QTabWidget;

namespace CQSandbox {

class Canvas3D;

class Control3D : public QFrame {
  Q_OBJECT

 public:
  Control3D(Canvas3D *canvas);

  void update();
  void updateLights();

 private Q_SLOTS:
  void depthTestSlot(int b);
  void cullFaceSlot(int b);
  void frontFaceSlot(int b);

  void bgColorSlot(const QColor &c);

  void cameraRotateSlot(int b);
  void cameraZoomSlot(double r);
  void cameraNearSlot(double r);
  void cameraFarSlot(double r);
  void cameraYawSlot(double r);
  void cameraPitchSlot(double r);
  void cameraRollSlot(double r);
  void cameraPosSlot();

  void lightSelectedSlot(QListWidgetItem *, QListWidgetItem *);

  void lightCheckSlot(int b);
  void lightColorSlot(const QColor &c);
  void lightPosSlot();
  void lightDirSlot();
  void lightCutoffSlot(double);
  void lightRadiusSlot(double);

  void objectSelectedSlot(QListWidgetItem *, QListWidgetItem *);

 private Q_SLOTS:
  void updateSlot();
  void closeSlot();

 private:
  Canvas3D* canvas_ { nullptr };

  QTabWidget* tab_ { nullptr };

  QCheckBox*   depthTestCheck_ { nullptr };
  QCheckBox*   cullFaceCheck_  { nullptr };
  QCheckBox*   frontFaceCheck_ { nullptr };
  CQColorEdit* bgColorEdit_    { nullptr };

  QCheckBox*     cameraRotateCheck_ { nullptr };
  CQRealSpin*    cameraZoomEdit_    { nullptr };
  CQRealSpin*    cameraNearEdit_    { nullptr };
  CQRealSpin*    cameraFarEdit_     { nullptr };
  CQRealSpin*    cameraYawEdit_     { nullptr };
  CQRealSpin*    cameraPitchEdit_   { nullptr };
  CQRealSpin*    cameraRollEdit_    { nullptr };
  CQPoint3DEdit* cameraPosEdit_     { nullptr };

  QListWidget*   lightsList_      { nullptr };
  QComboBox*     lightTypeCombo_  { nullptr };
  QCheckBox*     lightCheck_      { nullptr };
  CQColorEdit*   lightColorEdit_  { nullptr };
  CQPoint3DEdit* lightPosEdit_    { nullptr };
  CQPoint3DEdit* lightDirEdit_    { nullptr };
  CQRealSpin*    lightCutoffEdit_ { nullptr };
  CQRealSpin*    lightRadiusEdit_ { nullptr };

  QListWidget*        objectsList_ { nullptr };
  CQPropertyViewTree* objectTree_  { nullptr };

  CQRealSpin* ambientEdit_    { nullptr };
  CQRealSpin* diffuseEdit_    { nullptr };
  CQRealSpin* specularEdit_   { nullptr };
  CQRealSpin* shininessEdit_  { nullptr };
};

}

#endif
