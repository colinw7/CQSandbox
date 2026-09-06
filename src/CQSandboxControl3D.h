#ifndef CQSandboxControl3D_H
#define CQSandboxControl3D_H

#include <QFrame>

class CQPoint3DEdit;
class CQColorEdit;
class CQRealSpin;
class CQPropertyViewTree;
class CQXml;

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
 ~Control3D() override;

  Canvas3D *canvas() const { return canvas_; }

  QFrame *uiFrame() const { return uiFrame_; }

  void init();

  bool createUi(const QString &ui);
  bool getUiValue(const QString &name, QVariant &value) const;
  bool setUiValue(const QString &name, const QVariant &value);
  bool getUiWidgetValue(const QString &widget, const QString &name, QVariant &value) const;
  bool setUiWidgetValue(const QString &widget, const QString &name, const QVariant &value);

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

  // camera
  void cameraTypeSlot(int i);
  void cameraOrthoTypeSlot(int i);

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
  void cameraDistanceSlot(double r);

  void resetCameraSlot();

  // light
  void ambientColorSlot(const QColor &c);
  void ambientStrengthSlot();
  void diffuseSlot();
  void specularColorSlot(const QColor &c);
  void specularSlot();
  void emissiveColorSlot(const QColor &c);
  void emissiveSlot();
  void shininessSlot();

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
  void overviewZClipSlot(int);
  void overviewShowCameraSlot(int);
  void overviewShowLightSlot(int);
  void overviewShowBasisSlot(int);

  void overviewBgColorSlot(const QColor &c);
  void overviewStrokeColorSlot(const QColor &c);
  void overviewStrokeAlphaSlot(double a);
  void overviewFillColorSlot(const QColor &c);
  void overviewFillAlphaSlot(double a);
  void overviewSelectedColorSlot(const QColor &c);
  void overviewPointSizeSlot(double a);

 private Q_SLOTS:
  void updateSlot();

  void objectAddedSlot();
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
  };

  ControlData controlData_;

  struct CameraData {
    QComboBox*     typeCombo      { nullptr };
    QComboBox*     orthoTypeCombo { nullptr };
//  QCheckBox*     rotateCheck    { nullptr };
//  CQRealSpin*    zoomEdit       { nullptr };
    CQRealSpin*    pitchEdit      { nullptr };
    CQRealSpin*    yawEdit        { nullptr };
    CQRealSpin*    rollEdit       { nullptr };
    CQRealSpin*    nearEdit       { nullptr };
    CQRealSpin*    farEdit        { nullptr };
    CQRealSpin*    fovEdit        { nullptr };
    CQPoint3DEdit* originEdit     { nullptr };
    CQPoint3DEdit* posEdit        { nullptr };
    CQRealSpin*    distanceEdit   { nullptr };
  };

  CameraData cameraData_;

  struct LightData {
    CQColorEdit*   ambientColorEdit    { nullptr };
    CQRealSpin*    ambientStrengthEdit { nullptr };
    CQRealSpin*    diffuseEdit         { nullptr };
    CQColorEdit*   specularColorEdit   { nullptr };
    CQRealSpin*    specularEdit        { nullptr };
    CQColorEdit*   emissiveColorEdit   { nullptr };
    CQRealSpin*    emissiveEdit        { nullptr };
    CQRealSpin*    shininessEdit       { nullptr };
    QListWidget*   list                { nullptr };
    QComboBox*     typeCombo           { nullptr };
    QCheckBox*     enabledCheck        { nullptr };
    CQColorEdit*   colorEdit           { nullptr };
    CQPoint3DEdit* posEdit             { nullptr };
    CQPoint3DEdit* dirEdit             { nullptr };
    CQRealSpin*    cutoffEdit          { nullptr };
    CQRealSpin*    radiusEdit          { nullptr };
  };

  LightData lightData_;

  struct ObjectsData {
    QListWidget*        list { nullptr };
    CQPropertyViewTree* tree { nullptr };
  };

  ObjectsData objectsData_;

  struct OverviewData {
    QCheckBox* wireFrameCheck { nullptr };
    QCheckBox* solidCheck     { nullptr };
    QCheckBox* zclipCheck     { nullptr };
    QCheckBox* cameraCheck    { nullptr };
    QCheckBox* lightCheck     { nullptr };
    QCheckBox* basisCheck     { nullptr };

    CQColorEdit* bgColor       { nullptr };
    CQColorEdit* strokeColor   { nullptr };
    CQRealSpin*  strokeAlpha   { nullptr };
    CQColorEdit* fillColor     { nullptr };
    CQRealSpin*  fillAlpha     { nullptr };
    CQColorEdit* selectedColor { nullptr };
    CQRealSpin*  pointSize     { nullptr };
  };

  OverviewData overviewData_;

  QFrame* uiFrame_ { nullptr };
  CQXml*  xml_     { nullptr };

  bool shown_          { false };
  bool needsUpdate_    { false };
  bool objectsChanged_ { true };
  bool lightsChanged_  { true };
};

}

#endif
