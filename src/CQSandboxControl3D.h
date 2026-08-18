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

  void init();

  void updateWidgets();

  void toggleShown();

 private:
  void connectLights(bool);

  void updateControl();
  void updateCamera();
  void updateLights();
  void updateObjects();
  void updateOverview();

 private:
  QFrame *addControlFrame();
  QFrame *addCameraFrame();
  QFrame *addLightFrame();
  QFrame *addObjectsFrame();
  QFrame *addOverviewFrame();

 private Q_SLOTS:
  // control
  void depthTestSlot(int b);
  void cullFaceSlot(int b);
  void frontFaceSlot(int b);

  void bgColorSlot(const QColor &c);

  void ambientColorSlot(const QColor &c);
  void ambientStrengthSlot();
  void diffuseSlot();
  void specularColorSlot(const QColor &c);
  void specularSlot();
  void emissiveColorSlot(const QColor &c);
  void emissiveSlot();
  void shininessSlot();

  // camera
//void cameraRotateSlot(int b);
//void cameraZoomSlot(double r);

  void cameraPitchSlot(double r);
  void cameraYawSlot(double r);
  void cameraRollSlot(double r);

  void cameraNearSlot(double r);
  void cameraFarSlot(double r);
  void cameraFovSlot(double r);

  void cameraOriginSlot();
  void cameraPosSlot();

  void resetCameraSlot();

  // light
  void lightSelectedSlot(QListWidgetItem *, QListWidgetItem *);

  void lightCheckSlot(int b);
  void lightColorSlot(const QColor &c);
  void lightPosSlot();
  void lightDirSlot();
  void lightCutoffSlot(double);
  void lightRadiusSlot(double);
  void resetLightSlot();

  // objects
  void objectSelectedSlot(QListWidgetItem *, QListWidgetItem *);

  // overview
  void overviewWireframeSlot(int);
  void overviewSolidSlot(int);
  void overviewShowCameraSlot(int);
  void overviewShowLightSlot(int);
  void overviewShowBasisSlot(int);

 private Q_SLOTS:
  void updateSlot();

  void lightAddedSlot();

  void uiSlot();

  void closeSlot();

 private:
  Canvas3D* canvas_ { nullptr };

  QTabWidget* tab_ { nullptr };

  struct ControlData {
    QCheckBox*   depthTestCheck      { nullptr };
    QCheckBox*   cullFaceCheck       { nullptr };
    QCheckBox*   frontFaceCheck      { nullptr };
    CQColorEdit* bgColorEdit         { nullptr };
    CQColorEdit* ambientColorEdit    { nullptr };
    CQRealSpin*  ambientStrengthEdit { nullptr };
    CQRealSpin*  diffuseEdit         { nullptr };
    CQColorEdit* specularColorEdit   { nullptr };
    CQRealSpin*  specularEdit        { nullptr };
    CQColorEdit* emissiveColorEdit   { nullptr };
    CQRealSpin*  emissiveEdit        { nullptr };
    CQRealSpin*  shininessEdit       { nullptr };
  };

  ControlData controlData_;

  struct CameraData {
//  QCheckBox*     rotateCheck { nullptr };
//  CQRealSpin*    zoomEdit    { nullptr };
    CQRealSpin*    pitchEdit   { nullptr };
    CQRealSpin*    yawEdit     { nullptr };
    CQRealSpin*    rollEdit    { nullptr };
    CQRealSpin*    nearEdit    { nullptr };
    CQRealSpin*    farEdit     { nullptr };
    CQRealSpin*    fovEdit     { nullptr };
    CQPoint3DEdit* originEdit  { nullptr };
    CQPoint3DEdit* posEdit     { nullptr };
  };

  CameraData cameraData_;

  struct LightData {
    QListWidget*   list         { nullptr };
    QComboBox*     typeCombo    { nullptr };
    QCheckBox*     enabledCheck { nullptr };
    CQColorEdit*   colorEdit    { nullptr };
    CQPoint3DEdit* posEdit      { nullptr };
    CQPoint3DEdit* dirEdit      { nullptr };
    CQRealSpin*    cutoffEdit   { nullptr };
    CQRealSpin*    radiusEdit   { nullptr };
  };

  LightData lightData_;

  QListWidget*        objectsList_ { nullptr };
  CQPropertyViewTree* objectTree_  { nullptr };

  struct OverviewData {
    QCheckBox* wireFrameCheck { nullptr };
    QCheckBox* solidCheck     { nullptr };
    QCheckBox* cameraCheck    { nullptr };
    QCheckBox* lightCheck     { nullptr };
    QCheckBox* basisCheck     { nullptr };
  };

  OverviewData overviewData_;

  bool shown_         { false };
  bool needsUpdate_   { false };
  bool lightsChanged_ { true };
};

}

#endif
