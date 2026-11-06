#include <CQSandboxControl3D.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxLight3D.h>
#include <CQSandboxCamera.h>
#include <CQSandboxOrthoCamera.h>
#include <CQSandboxOverview3D.h>
#include <CQSandboxMaterial3D.h>
#include <CQSandboxTexture.h>
#include <CQSandboxUVMap.h>
#include <CQSandboxObjectsList.h>
#include <CQSandboxApp.h>
#include <CQSandboxUI.h>
#include <CQSandboxXml.h>
#include <CQSandboxUtil.h>

#include <CQGLBuffer.h>

#include <CQColorEdit.h>
#include <CQBBox3DEdit.h>
#include <CQPoint3DEdit.h>
#include <CQRealSpin.h>
#include <CQPropertyViewTree.h>
#include <CQImagePreview.h>
#include <CQUtil.h>

#include <QTabWidget>
#include <QGroupBox>
#include <QListWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>

namespace CQSandbox {

class Xml3D : public Xml {
 public:
  Xml3D(Control3D *control) :
   Xml(control->canvas()->tcl()), control_(control) {
  }

 private:
  Control3D* control_ { nullptr };
};

}

//---

namespace CQSandbox {

Control3D::
Control3D(CQSandbox::Canvas3D *canvas) :
 QFrame(nullptr), canvas_(canvas)
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  //---

  auto *layout = new QVBoxLayout(this);

  tab_ = CQUtil::makeWidget<QTabWidget>("tab");

  layout->addWidget(tab_);

  //---

  auto *controlFrame   = addControlFrame();
  auto *cameraFrame    = addCameraFrame();
  auto *lightsFrame    = addLightsFrame();
  auto *materialsFrame = addMaterialsFrame();
  auto *texturesFrame  = addTexturesFrame();
  auto *overviewFrame  = addOverviewFrame();
  auto *objectsFrame   = addObjectsFrame();
  auto *selectionFrame = addSelectionFrame();

  tab_->addTab(controlFrame  , "General");
  tab_->addTab(cameraFrame   , "Camera");
  tab_->addTab(lightsFrame   , "Lights");
  tab_->addTab(materialsFrame, "Materials");
  tab_->addTab(texturesFrame , "Textures");
  tab_->addTab(overviewFrame , "Overview");
  tab_->addTab(objectsFrame  , "Objects");
  tab_->addTab(selectionFrame, "Selection");

  //---

  uiFrame_ = CQUtil::makeWidget<QFrame>("uiFrame");

  tab_->addTab(uiFrame_, "UI");

  //---

  auto *updateButton = new QPushButton("Update");
  auto *closeButton  = new QPushButton("Close");

  auto *buttonFrame  = CQUtil::makeWidget<QFrame>(this, "buttonFrame");
  auto *buttonLayout = new QHBoxLayout(buttonFrame);

  layout->addWidget(buttonFrame);

  buttonLayout->addStretch(1);
  buttonLayout->addWidget(updateButton);
  buttonLayout->addWidget(closeButton);

  connect(canvas_, SIGNAL(objectTransformChanged()), this, SLOT(updateSlot()));

  connect(updateButton, &QPushButton::clicked, this, &Control3D::updateSlot);
  connect(closeButton, &QPushButton::clicked, this, &Control3D::closeSlot);

  connect(closeButton, &QPushButton::clicked, this, &Control3D::closeSlot);

  //---

  updateWidgets();
}

Control3D::
~Control3D()
{
  delete xml_;
}

void
Control3D::
init()
{
  connect(canvas_, SIGNAL(lightChanged()), this, SLOT(updateSlot()));

  connect(canvas_, SIGNAL(objectsChanged()), this, SLOT(objectAddedSlot()));

  connect(canvas_, SIGNAL(lightAdded()), this, SLOT(lightAddedSlot()));

  connect(canvas_, SIGNAL(materialAdded()), this, SLOT(materialAddedSlot()));

  connect(canvas_, SIGNAL(textureAdded()), this, SLOT(textureAddedSlot()));

  connect(canvas_, SIGNAL(selectionChanged()), this, SLOT(selectionChangedSlot()));

  connect(canvas_, SIGNAL(uiUpdateSignal()), this, SLOT(uiSlot()));
}

QFrame *
Control3D::
addControlFrame()
{
  auto *frame  = CQUtil::makeWidget<QFrame>(this, "frame");
  auto *layout = new QVBoxLayout(frame);

  UI ui(this, layout);

  //---

  ui.startGroup("GL");

  controlData_.depthTestCheck = ui.addLabelEdit("Depth Test", new QCheckBox);
  controlData_.cullFaceCheck  = ui.addLabelEdit("Cull Face" , new QCheckBox);
  controlData_.frontFaceCheck = ui.addLabelEdit("Front Face", new QCheckBox);

  controlData_.bgColorEdit = ui.addLabelEdit("Bg Color", new CQColorEdit);

  ui.endGroup();

  //---

  ui.startGroup("Wireframe");

  controlData_.wireframeColor = ui.addLabelEdit("Color", new CQColorEdit);
  controlData_.wireframeAlpha = ui.addLabelEdit("Alpha", new CQRealSpin);

  ui.endGroup();

  //---

  ui.startGroup("Select");

  controlData_.selectColor       = ui.addLabelEdit("Color", new CQColorEdit);
  controlData_.selectAlpha       = ui.addLabelEdit("Alpha", new CQRealSpin);
  controlData_.selectPointSize   = ui.addLabelEdit("Point Size", new CQRealSpin);
  controlData_.selectLineWidth   = ui.addLabelEdit("Line Width", new CQRealSpin);
  controlData_.selectCenterCheck = ui.addLabelEdit("Face Center", new QCheckBox);

  ui.endGroup();

  //---

  ui.startGroup("Edit");

  controlData_.editColor       = ui.addLabelEdit("Color", new CQColorEdit);
  controlData_.editFaceAlpha   = ui.addLabelEdit("Face Alpha", new CQRealSpin);
  controlData_.editPointSize   = ui.addLabelEdit("Point Size", new CQRealSpin);
  controlData_.editLineWidth   = ui.addLabelEdit("Line Width", new CQRealSpin);

  ui.endGroup();

  //---

  ui.startGroup("Shadow");

  controlData_.showShadowCheck  = ui.addLabelEdit("Enabled", new QCheckBox);
  controlData_.debugShadowCheck = ui.addLabelEdit("Debug"  , new QCheckBox);

  ui.endGroup();

  //---

  ui.startGroup("Skybox");

  controlData_.showSkyboxCheck  = ui.addLabelEdit("Visible", new QCheckBox);
  controlData_.debugSkyboxCheck = ui.addLabelEdit("Debug"  , new QCheckBox);

  ui.endGroup();

  //---

  ui.startGroup("Orient");

  controlData_.showOrientCheck = ui.addLabelEdit("Visible", new QCheckBox);

  ui.endGroup();

  //---

  ui.startGroup("BBox");

  controlData_.bboxEdit = ui.addWidget(new CQBBox3DEdit);

  ui.endGroup();

  //---

  ui.addStretch();

  connectControl(true);

  return frame;
}

QFrame *
Control3D::
addCameraFrame()
{
  auto *frame  = CQUtil::makeWidget<QFrame>(this, "frame");
  auto *layout = new QVBoxLayout(frame);

  auto *controlFrame  = CQUtil::makeWidget<QFrame>("control");
  auto *controlLayout = new QGridLayout(controlFrame);

  layout->addWidget(controlFrame);

  int cameraRow = 0;

  auto addLabelEdit = [&](const QString &label, QWidget *w) {
    controlLayout->addWidget(new QLabel(label), cameraRow, 0);
    controlLayout->addWidget(w, cameraRow, 1);
    ++cameraRow;
  };

  auto addRealEdit = [&](const QString &label) {
    auto *edit = new CQRealSpin;
    addLabelEdit(label, edit);
    return edit;
  };

  auto addPoint3DEdit = [&](const QString &label) {
    auto *edit = new CQPoint3DEdit;
    addLabelEdit(label, edit);
    return edit;
  };

#if 0
  auto addCheck = [&](const QString &label) {
    auto *check = new QCheckBox;
    addLabelEdit(label, check);
    return check;
  };
#endif

  auto addCombo = [&](const QString &label, const QStringList &names) {
    auto *combo = new QComboBox;
    for (const auto &name : names)
      combo->addItem(name);
    addLabelEdit(label, combo);
    return combo;
  };

  //---

  cameraData_.typeCombo = addCombo("Type", QStringList() <<
    "Free" << "First Person" << "Ortho");

  cameraData_.orthoTypeCombo = addCombo("Ortho Type", QStringList() <<
    "Top" << "Bottom" << "Left" << "Right" << "Front" << "Back");

#if 0
  cameraData_.rotateCheck = addCheck("Rotate");

  cameraData_.zoomEdit = addRealEdit("Zoom");
#endif

  cameraData_.pitchEdit = addRealEdit("Pitch");
  cameraData_.yawEdit   = addRealEdit("Yaw"  );
  cameraData_.rollEdit  = addRealEdit("Roll" );

  cameraData_.nearEdit = addRealEdit("Near");
  cameraData_.farEdit  = addRealEdit("Far" );
  cameraData_.fovEdit  = addRealEdit("FOV" );

  cameraData_.originEdit   = addPoint3DEdit("Origin"  );
  cameraData_.posEdit      = addPoint3DEdit("Position");
  cameraData_.distanceEdit = addRealEdit   ("Distance");

  //---

  controlLayout->setRowStretch(cameraRow, 1);

  //---

  auto *resetButton = new QPushButton("Reset");

  auto *buttonFrame  = CQUtil::makeWidget<QFrame>(this, "buttonFrame");
  auto *buttonLayout = new QHBoxLayout(buttonFrame);

  layout->addWidget(buttonFrame);

  buttonLayout->addStretch(1);
  buttonLayout->addWidget(resetButton);

  connect(resetButton, &QPushButton::clicked, this, &Control3D::resetCameraSlot);

  //---

  connectCamera(true);

  return frame;
}

QFrame *
Control3D::
addLightsFrame()
{
  auto *frame  = CQUtil::makeWidget<QFrame>("frame");
  auto *layout = new QVBoxLayout(frame);

  //---

  auto *controlFrame  = new QGroupBox("Global");
  auto *controlLayout = new QGridLayout(controlFrame);

  layout->addWidget(controlFrame);

  int lightRow = 0;

  //---

  auto addLabelEdit = [&](const QString &label, QWidget *w) {
    controlLayout->addWidget(new QLabel(label), lightRow, 0);
    controlLayout->addWidget(w, lightRow, 1);
    ++lightRow;
  };

  auto addRealEdit = [&](const QString &label) {
    auto *edit = new CQRealSpin;
    addLabelEdit(label, edit);
    return edit;
  };

  auto addColorEdit = [&](const QString &label) {
    auto *edit = new CQColorEdit;
    addLabelEdit(label, edit);
    return edit;
  };

  auto addCombo = [&](const QString &label, const QStringList &names) {
    auto *combo = new QComboBox;
    for (const auto &name : names)
      combo->addItem(name);
    addLabelEdit(label, combo);
    return combo;
  };

  auto addCheck = [&](const QString &label) {
    auto *check = new QCheckBox;
    addLabelEdit(label, check);
    return check;
  };

  auto addPoint3DEdit = [&](const QString &label) {
    auto *edit = new CQPoint3DEdit;
    addLabelEdit(label, edit);
    return edit;
  };

  //---

  lightsData_.ambientColorEdit = addColorEdit("Ambient Color");

  lightsData_.ambientStrengthEdit = addRealEdit("Ambient Strength");
  lightsData_.ambientStrengthEdit->setRange(0.0, 1.0);

  //---

  lightsData_.diffuseEdit = addRealEdit("Diffuse Strength");
  lightsData_.diffuseEdit->setRange(0.0, 2.0);

  //---

  lightsData_.specularColorEdit = addColorEdit("Specular Color");

  lightsData_.specularEdit = addRealEdit("Specular Strength");
  lightsData_.specularEdit->setRange(0.0, 1.0);

  //---

  lightsData_.emissiveColorEdit = addColorEdit("Emissive Color");

  lightsData_.emissiveEdit = addRealEdit("Emissive Strength");
  lightsData_.emissiveEdit->setRange(0.0, 1.0);

  //---

  lightsData_.shininessEdit = addRealEdit("Shininess");
  lightsData_.shininessEdit->setRange(0.0, 100.0);

  //---

  controlFrame  = new QGroupBox("Lights");
  controlLayout = new QGridLayout(controlFrame);

  layout->addWidget(controlFrame);

  lightRow = 0;

  //---

  lightsData_.list = new QListWidget;

  lightsData_.list->setSelectionMode(QListWidget::SingleSelection);

  controlLayout->addWidget(lightsData_.list, lightRow, 0, 1, 2);

  ++lightRow;

  //--

  lightsData_.typeCombo = addCombo("Type",
    QStringList() << "Directional" << "Point" << "Spot");

  lightsData_.enabledCheck = addCheck("Enabled");

  lightsData_.colorEdit = addColorEdit("Color"); // diffuse

  //---

  lightsData_.powerEdit = addRealEdit("Power");
  lightsData_.powerEdit->setRange(0.0, 100.0);

  //---

  lightsData_.posEdit = addPoint3DEdit("Position");

  //---

  lightsData_.dirEdit = addPoint3DEdit("Direction");

  //---

  lightsData_.cutoffEdit = addRealEdit("Cut Off Angle");

  //---

  lightsData_.radiusEdit = addRealEdit("Point Radius");

  //---

  //layout->setRowStretch(lightRow, 1);
  layout->addStretch(1);

  //---

  auto *resetButton = new QPushButton("Reset");

  auto *buttonFrame  = CQUtil::makeWidget<QFrame>(this, "buttonFrame");
  auto *buttonLayout = new QHBoxLayout(buttonFrame);

  layout->addWidget(buttonFrame);

  buttonLayout->addStretch(1);
  buttonLayout->addWidget(resetButton);

  connect(resetButton, &QPushButton::clicked, this, &Control3D::resetLightSlot);

  //---

  connectLights(true);

  return frame;
}

QFrame *
Control3D::
addMaterialsFrame()
{
  auto *frame  = CQUtil::makeWidget<QFrame>("frame");
  auto *layout = new QVBoxLayout(frame);

  //---

  auto *listFrame  = new QGroupBox("Materials");
  auto *listLayout = new QVBoxLayout(listFrame);

  materialsData_.list = new QListWidget;

  materialsData_.list->setSelectionMode(QListWidget::SingleSelection);

  listLayout->addWidget(materialsData_.list);

  layout->addWidget(listFrame);

  //---

  auto *controlFrame  = new QGroupBox("Data");
  auto *controlLayout = new QGridLayout(controlFrame);

  layout->addWidget(controlFrame);

  //---

  int materialsRow = 0;

  //---

  auto addLabelEdit = [&](const QString &label, QWidget *w) {
    controlLayout->addWidget(new QLabel(label), materialsRow, 0);
    controlLayout->addWidget(w, materialsRow, 1);
    ++materialsRow;
  };

  auto addRealEdit = [&](const QString &label) {
    auto *edit = new CQRealSpin;
    addLabelEdit(label, edit);
    return edit;
  };

  auto addColorEdit = [&](const QString &label) {
    auto *edit = new CQColorEdit;
    addLabelEdit(label, edit);
    return edit;
  };

  //---

  materialsData_.diffuseColorEdit  = addColorEdit("Diffuse");
  materialsData_.specularColorEdit = addColorEdit("Specular");
  materialsData_.emissionColorEdit = addColorEdit("Emission");

  materialsData_.emissionEdit = addRealEdit("Emission Strength");
  materialsData_.emissionEdit->setRange(0.0, 1.0);

  materialsData_.specularEdit = addRealEdit("Specular Strength");
  materialsData_.specularEdit->setRange(0.0, 1.0);

  materialsData_.shininessEdit = addRealEdit("Shininess");
  materialsData_.shininessEdit->setRange(0.0, 100.0);

  materialsData_.transparencyEdit = addRealEdit("Transparency");
  materialsData_.transparencyEdit->setRange(0.0, 1.0);

  materialsData_.reflectivityEdit = addRealEdit("Reflectivity");
  materialsData_.reflectivityEdit->setRange(0.0, 1.0);

  materialsData_.refractivityEdit = addRealEdit("Refractivity");
  materialsData_.refractivityEdit->setRange(0.0, 1.0);

  //---

  connectMaterials(true);

  return frame;
}

QFrame *
Control3D::
addTexturesFrame()
{
  auto *frame  = CQUtil::makeWidget<QFrame>("frame");
  auto *layout = new QVBoxLayout(frame);

  auto *controlFrame  = CQUtil::makeWidget<QFrame>("control");
  auto *controlLayout = new QVBoxLayout(controlFrame);

  layout->addWidget(controlFrame);

  //---

  auto *listFrame  = new QGroupBox("Textures");
  auto *listLayout = new QVBoxLayout(listFrame);

  texturesData_.list = new QListWidget;

  texturesData_.list->setSelectionMode(QListWidget::SingleSelection);

  listLayout->addWidget(texturesData_.list);

  controlLayout->addWidget(listFrame);

  listFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  //---

  texturesData_.image = new CQImagePreview;

  controlLayout->addWidget(texturesData_.image);

  //---

  texturesData_.uvMap = new UVMap(canvas_);

  controlLayout->addWidget(texturesData_.uvMap);

  //---

  //controlLayout->addStretch(1);

  //---

  connectTextures(true);

  return frame;
}

QFrame *
Control3D::
addObjectsFrame()
{
  auto *frame  = CQUtil::makeWidget<QFrame>("frame");
  auto *layout = new QVBoxLayout(frame);

  auto *controlFrame  = CQUtil::makeWidget<QFrame>("control");
  auto *controlLayout = new QVBoxLayout(controlFrame);

  layout->addWidget(controlFrame);

  //---

  objectsData_.list = new ObjectsList(canvas_);

  controlLayout->addWidget(objectsData_.list);

  //---

  objectsData_.tree = new CQPropertyViewTree(this);

  layout->addWidget(objectsData_.tree);

  //---

  auto *posFrame  = new QGroupBox("Translate");
  auto *posLayout = new QVBoxLayout(posFrame);

  layout->addWidget(posFrame);

  objectsData_.posEdit = new CQPoint3DEdit;

  posLayout->addWidget(objectsData_.posEdit);

  //---

  auto *scaleFrame  = new QGroupBox("Scale");
  auto *scaleLayout = new QVBoxLayout(scaleFrame);

  layout->addWidget(scaleFrame);

  objectsData_.scaleEdit = new CQPoint3DEdit;

  scaleLayout->addWidget(objectsData_.scaleEdit);

  //---

  auto *rotateFrame  = new QGroupBox("Rotate");
  auto *rotateLayout = new QVBoxLayout(rotateFrame);

  layout->addWidget(rotateFrame);

  objectsData_.rotateEdit = new CQPoint3DEdit;

  rotateLayout->addWidget(objectsData_.rotateEdit);

  //---

  auto *bboxFrame  = new QGroupBox("BBox");
  auto *bboxLayout = new QVBoxLayout(bboxFrame);

  layout->addWidget(bboxFrame);

  objectsData_.bboxEdit = new CQBBox3DEdit;

  bboxLayout->addWidget(objectsData_.bboxEdit);

  //---

  auto *buttonFrame  = CQUtil::makeWidget<QFrame>(this, "buttonFrame");
  auto *buttonLayout = new QHBoxLayout(buttonFrame);

  layout->addWidget(buttonFrame);

  auto *applyButton = new QPushButton("Apply Transform");

  connect(applyButton, SIGNAL(clicked()), this, SLOT(objectApplySlot()));

  buttonLayout->addWidget(applyButton);
  buttonLayout->addStretch(1);

  //---

  connectObjects(true);

  return frame;
}

QFrame *
Control3D::
addOverviewFrame()
{
  auto *frame  = CQUtil::makeWidget<QFrame>("frame");
  auto *layout = new QGridLayout(frame);

  //---

  int row = 0;

  auto addLabelEdit = [&](const QString &label, QWidget *w) {
    layout->addWidget(new QLabel(label), row, 0);
    layout->addWidget(w, row, 1);
    ++row;
  };

  auto addCheck = [&](const QString &label) {
    auto *checkBox = new QCheckBox;
    addLabelEdit(label, checkBox);
    return checkBox;
  };

  auto addRealEdit = [&](const QString &label) {
    auto *edit = new CQRealSpin;
    addLabelEdit(label, edit);
    return edit;
  };

  auto addColorEdit = [&](const QString &label) {
    auto *edit = new CQColorEdit;
    addLabelEdit(label, edit);
    return edit;
  };

  //---

  overviewData_.wireFrameCheck = addCheck("Wireframe"  );
  overviewData_.solidCheck     = addCheck("Solid"      );
  overviewData_.zclipCheck     = addCheck("Z Clip"     );
  overviewData_.cameraCheck    = addCheck("Show Camera");
  overviewData_.lightCheck     = addCheck("Show Light" );
  overviewData_.basisCheck     = addCheck("Show Basis" );

  overviewData_.bgColor       = addColorEdit("Background"    );
  overviewData_.strokeColor   = addColorEdit("Stroke Color"  );
  overviewData_.strokeAlpha   = addRealEdit ("Stroke Alpha"  );
  overviewData_.fillColor     = addColorEdit("Fill Color"    );
  overviewData_.fillAlpha     = addRealEdit ("Fill Alpha"    );
  overviewData_.selectedColor = addColorEdit("Selected Color");
  overviewData_.pointSize     = addRealEdit ("Point Size"    );

  //---

  layout->setRowStretch(row, 1);

  //---

  connectOverview(true);

  return frame;
}

QFrame *
Control3D::
addSelectionFrame()
{
  auto *frame  = CQUtil::makeWidget<QFrame>("frame");
  auto *layout = new QGridLayout(frame);

  //---

  auto *listFrame  = new QGroupBox("Selection");
  auto *listLayout = new QVBoxLayout(listFrame);

  selectionData_.list = new QListWidget;

  selectionData_.list->setSelectionMode(QListWidget::SingleSelection);

  listLayout->addWidget(selectionData_.list);

  layout->addWidget(listFrame);

  //---

  connectSelection(true);

  return frame;
}

void
Control3D::
toggleShown()
{
  setShown(! isShown());
}

void
Control3D::
setShown(bool shown)
{
  if (shown == shown_)
    return;

  shown_ = ! shown_;

  //---

  auto *app = canvas_->app();

  auto geom = app->geometry();

  int w = this->sizeHint().width();

  QRect geom1;

  if (shown_) {
    geom1 = QRect(geom.x(), geom.y(), geom.width() + w + 6, geom.height());

    this->updateWidgets();
    this->show();
  }
  else {
    geom1 = QRect(geom.x(), geom.y(), geom.width() - w - 6, geom.height());

    this->hide();
  }

  app->setGeometry(geom1);

  if (shown_)
    this->setFixedWidth(w);
  else {
    this->setMinimumWidth(0);
    this->setMaximumWidth(QWIDGETSIZE_MAX);
  }

  Q_EMIT shownStateChanged();
}

void
Control3D::
updateSlot()
{
  needsUpdate_ = true;
}

void
Control3D::
objectAddedSlot()
{
  needsUpdate_ = true;

  objectsData_.changed = true;

  if (canvas_->isInited())
    uiSlot();
}

void
Control3D::
lightAddedSlot()
{
  needsUpdate_ = true;

  lightsData_.changed = true;

  if (canvas_->isInited())
    uiSlot();
}

void
Control3D::
materialAddedSlot()
{
  needsUpdate_ = true;

  materialsData_.changed = true;

  if (canvas_->isInited())
    uiSlot();
}

void
Control3D::
textureAddedSlot()
{
  needsUpdate_ = true;

  texturesData_.changed = true;

  if (canvas_->isInited())
    uiSlot();
}

void
Control3D::
selectionChangedSlot()
{
  needsUpdate_ = true;

  objectsData_.changed   = true;
  selectionData_.changed = true;

  if (canvas_->isInited())
    uiSlot();
}

void
Control3D::
uiSlot()
{
  if (needsUpdate_) {
    needsUpdate_ = false;

    updateWidgets();
  }
}

void
Control3D::
closeSlot()
{
  close();
}

void
Control3D::
updateWidgets()
{
  updateControl();
  updateCamera();
  updateLights();
  updateMaterials();
  updateTextures();
  updateObjects();
  updateOverview();
  updateSelection();
}

void
Control3D::
updateControl()
{
  connectControl(false);

  controlData_.depthTestCheck->setChecked(canvas_->isDepthTest());
  controlData_.cullFaceCheck ->setChecked(canvas_->isCullFace());
  controlData_.frontFaceCheck->setChecked(canvas_->isFrontFace());

  controlData_.bgColorEdit->setColor(canvas_->bgColor());

  controlData_.wireframeColor->setColor(Util::RGBAToQColor(canvas_->wireframeColor()));
  controlData_.wireframeAlpha->setValue(canvas_->wireframeAlpha());

  controlData_.selectColor      ->setColor  (Util::RGBAToQColor(canvas_->selectColor()));
  controlData_.selectAlpha      ->setValue  (canvas_->selectAlpha());
  controlData_.selectPointSize  ->setValue  (canvas_->selectPointSize());
  controlData_.selectLineWidth  ->setValue  (canvas_->selectLineWidth());
  controlData_.selectCenterCheck->setChecked(canvas_->isShowSelectedFaceCenter());

  controlData_.editColor    ->setColor  (Util::RGBAToQColor(canvas_->editColor()));
  controlData_.editFaceAlpha->setValue  (canvas_->editFaceAlpha());
  controlData_.editPointSize->setValue  (canvas_->editPointSize());
  controlData_.editLineWidth->setValue  (canvas_->editLineWidth());

  controlData_.showShadowCheck ->setChecked(canvas_->isShadowed());
  controlData_.debugShadowCheck->setChecked(canvas_->isShadowDebug());

  controlData_.showSkyboxCheck ->setChecked(canvas_->isShowSkybox());
  controlData_.debugSkyboxCheck->setChecked(canvas_->isDebugSkybox());

  controlData_.showOrientCheck->setChecked(canvas_->isShowOrient());

  controlData_.bboxEdit->setValue(canvas_->bbox());

  connectControl(true);
}

void
Control3D::
connectControl(bool b)
{
  if (b) {
    connect(controlData_.depthTestCheck, &QCheckBox::stateChanged,
            this, &Control3D::depthTestSlot);
    connect(controlData_.cullFaceCheck , &QCheckBox::stateChanged,
            this, &Control3D::cullFaceSlot);
    connect(controlData_.frontFaceCheck, &QCheckBox::stateChanged,
            this, &Control3D::frontFaceSlot);

    connect(controlData_.bgColorEdit, &CQColorEdit::colorChanged,
            this, &Control3D::bgColorSlot);

    connect(controlData_.wireframeColor, &CQColorEdit::colorChanged,
            this, &Control3D::wireframeColorSlot);
    connect(controlData_.wireframeAlpha, &CQRealSpin::realValueChanged,
            this, &Control3D::wireframeAlphaSlot);

    connect(controlData_.selectColor, &CQColorEdit::colorChanged,
            this, &Control3D::selectColorSlot);
    connect(controlData_.selectAlpha, &CQRealSpin::realValueChanged,
            this, &Control3D::selectAlphaSlot);
    connect(controlData_.selectPointSize, &CQRealSpin::realValueChanged,
            this, &Control3D::selectPointSizeSlot);
    connect(controlData_.selectLineWidth, &CQRealSpin::realValueChanged,
            this, &Control3D::selectLineWidthSlot);
    connect(controlData_.selectCenterCheck, &QCheckBox::stateChanged,
            this, &Control3D::selectFaceCenterSlot);

    connect(controlData_.editColor, &CQColorEdit::colorChanged,
            this, &Control3D::editColorSlot);
    connect(controlData_.editFaceAlpha, &CQRealSpin::realValueChanged,
            this, &Control3D::editFaceAlphaSlot);
    connect(controlData_.editPointSize, &CQRealSpin::realValueChanged,
            this, &Control3D::editPointSizeSlot);
    connect(controlData_.editLineWidth, &CQRealSpin::realValueChanged,
            this, &Control3D::editLineWidthSlot);

    connect(controlData_.showShadowCheck, &QCheckBox::stateChanged,
            this, &Control3D::enableShadowSlot);
    connect(controlData_.debugShadowCheck, &QCheckBox::stateChanged,
            this, &Control3D::debugShadowSlot);

    connect(controlData_.showSkyboxCheck, &QCheckBox::stateChanged,
            this, &Control3D::showSkyboxSlot);
    connect(controlData_.debugSkyboxCheck, &QCheckBox::stateChanged,
            this, &Control3D::debugSkyboxSlot);

    connect(controlData_.showOrientCheck, &QCheckBox::stateChanged,
            this, &Control3D::showOrientSlot);
  }
  else {
    disconnect(controlData_.depthTestCheck, &QCheckBox::stateChanged,
               this, &Control3D::depthTestSlot);
    disconnect(controlData_.cullFaceCheck , &QCheckBox::stateChanged,
               this, &Control3D::cullFaceSlot);
    disconnect(controlData_.frontFaceCheck, &QCheckBox::stateChanged,
               this, &Control3D::frontFaceSlot);

    disconnect(controlData_.bgColorEdit, &CQColorEdit::colorChanged,
               this, &Control3D::bgColorSlot);

    disconnect(controlData_.wireframeColor, &CQColorEdit::colorChanged,
               this, &Control3D::wireframeColorSlot);
    disconnect(controlData_.wireframeAlpha, &CQRealSpin::realValueChanged,
               this, &Control3D::wireframeAlphaSlot);

    disconnect(controlData_.selectColor, &CQColorEdit::colorChanged,
               this, &Control3D::selectColorSlot);
    disconnect(controlData_.selectAlpha, &CQRealSpin::realValueChanged,
               this, &Control3D::selectAlphaSlot);
    disconnect(controlData_.selectPointSize, &CQRealSpin::realValueChanged,
               this, &Control3D::selectPointSizeSlot);
    disconnect(controlData_.selectLineWidth, &CQRealSpin::realValueChanged,
               this, &Control3D::selectLineWidthSlot);
    disconnect(controlData_.selectCenterCheck, &QCheckBox::stateChanged,
               this, &Control3D::selectFaceCenterSlot);

    disconnect(controlData_.editColor, &CQColorEdit::colorChanged,
               this, &Control3D::editColorSlot);
    disconnect(controlData_.editFaceAlpha, &CQRealSpin::realValueChanged,
               this, &Control3D::editFaceAlphaSlot);
    disconnect(controlData_.editPointSize, &CQRealSpin::realValueChanged,
               this, &Control3D::editPointSizeSlot);
    disconnect(controlData_.editLineWidth, &CQRealSpin::realValueChanged,
               this, &Control3D::editLineWidthSlot);

    disconnect(controlData_.showShadowCheck, &QCheckBox::stateChanged,
               this, &Control3D::enableShadowSlot);
    disconnect(controlData_.debugShadowCheck, &QCheckBox::stateChanged,
               this, &Control3D::debugShadowSlot);

    disconnect(controlData_.showSkyboxCheck, &QCheckBox::stateChanged,
               this, &Control3D::showSkyboxSlot);
    disconnect(controlData_.debugSkyboxCheck, &QCheckBox::stateChanged,
               this, &Control3D::debugSkyboxSlot);

    disconnect(controlData_.showOrientCheck, &QCheckBox::stateChanged,
              this, &Control3D::showOrientSlot);
  }
}

void
Control3D::
updateCamera()
{
  connectCamera(false);

  //---

  auto cameraType = canvas_->cameraType();

  if      (cameraType == Canvas3D::CameraType::MODEL)
    cameraData_.typeCombo->setCurrentIndex(0);
  else if (cameraType == Canvas3D::CameraType::FIRST_PERSON)
    cameraData_.typeCombo->setCurrentIndex(1);
  else if (cameraType == Canvas3D::CameraType::ORTHO)
    cameraData_.typeCombo->setCurrentIndex(2);

  //---

  auto *camera = canvas_->currentCamera();

  if (camera) {
#if 0
    cameraData_.rotateCheck->setChecked(camera->isRotate());
    cameraData_.zoomEdit   ->setValue  (camera->zoom());
#endif

    cameraData_.pitchEdit->setValue(CMathGen::RadToDeg(camera->pitch()));
    cameraData_.yawEdit  ->setValue(CMathGen::RadToDeg(camera->yaw()));
    cameraData_.rollEdit ->setValue(CMathGen::RadToDeg(camera->roll()));

    cameraData_.nearEdit->setValue(camera->near());
    cameraData_.farEdit ->setValue(camera->far());
    cameraData_.fovEdit ->setValue(camera->fov());

    cameraData_.originEdit  ->setValue(camera->origin().point());
    cameraData_.posEdit     ->setValue(camera->position().point());
    cameraData_.distanceEdit->setValue(camera->distance());
  }

  //---

  connectCamera(true);
}

void
Control3D::
connectCamera(bool b)
{
  if (b){
    connect(canvas_, SIGNAL(cameraChangedSignal()),
            this, SLOT(updateSlot()));

    connect(cameraData_.typeCombo,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &Control3D::cameraTypeSlot);
    connect(cameraData_.orthoTypeCombo,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &Control3D::cameraOrthoTypeSlot);

#if 0
    connect(cameraData_.rotateCheck , &QCheckBox::stateChanged,
            this, &Control3D::cameraRotateSlot);
    connect(cameraData_.zoomEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::cameraZoomSlot);
#endif

    connect(cameraData_.pitchEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::cameraPitchSlot);
    connect(cameraData_.yawEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::cameraYawSlot);
    connect(cameraData_.rollEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::cameraRollSlot);

    connect(cameraData_.nearEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::cameraNearSlot);
    connect(cameraData_.farEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::cameraFarSlot);
    connect(cameraData_.fovEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::cameraFovSlot);

    connect(cameraData_.originEdit, &CQPoint3DEdit::editingFinished,
            this, &Control3D::cameraOriginSlot);
    connect(cameraData_.posEdit, &CQPoint3DEdit::editingFinished,
            this, &Control3D::cameraPosSlot);
    connect(cameraData_.distanceEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::cameraDistanceSlot);
  }
  else {
    disconnect(canvas_, SIGNAL(cameraChangedSignal()),
               this, SLOT(updateSlot()));

    disconnect(cameraData_.typeCombo,
               static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
               this, &Control3D::cameraTypeSlot);
    disconnect(cameraData_.orthoTypeCombo,
               static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
               this, &Control3D::cameraOrthoTypeSlot);
#if 0
    disconnect(cameraData_.rotateCheck , &QCheckBox::stateChanged,
               this, &Control3D::cameraRotateSlot);
    disconnect(cameraData_.zoomEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::cameraZoomSlot);
#endif

    disconnect(cameraData_.pitchEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::cameraPitchSlot);
    disconnect(cameraData_.yawEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::cameraYawSlot);
    disconnect(cameraData_.rollEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::cameraRollSlot);

    disconnect(cameraData_.nearEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::cameraNearSlot);
    disconnect(cameraData_.farEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::cameraFarSlot);
    disconnect(cameraData_.fovEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::cameraFovSlot);

    disconnect(cameraData_.originEdit, &CQPoint3DEdit::editingFinished,
               this, &Control3D::cameraOriginSlot);
    disconnect(cameraData_.posEdit, &CQPoint3DEdit::editingFinished,
               this, &Control3D::cameraPosSlot);
    disconnect(cameraData_.distanceEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::cameraDistanceSlot);
  }
}

void
Control3D::
updateLights()
{
  connectLights(false);

  //---

  lightsData_.ambientColorEdit   ->setColor(Util::RGBAToQColor(canvas_->ambientColor()));
  lightsData_.ambientStrengthEdit->setValue(canvas_->ambientStrength());
  lightsData_.diffuseEdit        ->setValue(canvas_->diffuseStrength());
  lightsData_.specularColorEdit  ->setColor(Util::RGBAToQColor(canvas_->specularColor()));
  lightsData_.specularEdit       ->setValue(canvas_->specularStrength());
  lightsData_.emissiveColorEdit  ->setColor(Util::RGBAToQColor(canvas_->emissiveColor()));
  lightsData_.emissiveEdit       ->setValue(canvas_->emissiveStrength());
  lightsData_.shininessEdit      ->setValue(canvas_->shininess());

  //---

  auto *currentLight = canvas_->currentLight();

  lightsData_.typeCombo->setCurrentIndex(int(currentLight->getType()));

  lightsData_.enabledCheck->setChecked(currentLight->getEnabled());
  lightsData_.colorEdit   ->setColor(Util::RGBAToQColor(currentLight->getDiffuse()));
  lightsData_.powerEdit   ->setValue(currentLight->getPower());
  lightsData_.posEdit     ->setValue(currentLight->getPosition());

  if (currentLight->getType() == Light3D::Type::SPOT)
    lightsData_.dirEdit->setValue(currentLight->getSpotDirection().point());
  else
    lightsData_.dirEdit->setValue(currentLight->getDirection().point());

  lightsData_.cutoffEdit->setEnabled(currentLight->getType() == Light3D::Type::SPOT);
  lightsData_.cutoffEdit->setValue(currentLight->getSpotCutOffAngle());

  lightsData_.radiusEdit->setEnabled(currentLight->getType() == Light3D::Type::POINT);
  lightsData_.radiusEdit->setValue(currentLight->getPointRadius());

  if (lightsData_.changed) {
    lightsData_.changed = false;

    lightsData_.list->clear();

    QListWidgetItem *currentItem = nullptr;

    for (auto *light : canvas_->lights()) {
      auto lightName = QString("light.%1").arg(light->id());

      auto *item = new QListWidgetItem(lightName);

      lightsData_.list->addItem(item);

      item->setData(Qt::UserRole, light->id());

      if (light == currentLight)
        currentItem = item;
    }

    if (currentItem)
      lightsData_.list->setCurrentItem(currentItem, QItemSelectionModel::Select);
  }

  //---

  connectLights(true);
}

void
Control3D::
connectLights(bool b)
{
  if (b) {
    connect(lightsData_.ambientColorEdit, &CQColorEdit::colorChanged,
            this, &Control3D::ambientColorSlot);
    connect(lightsData_.ambientStrengthEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::ambientStrengthSlot);
    connect(lightsData_.diffuseEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::diffuseSlot);
    connect(lightsData_.specularColorEdit, &CQColorEdit::colorChanged,
            this, &Control3D::specularColorSlot);
    connect(lightsData_.specularEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::specularSlot);
    connect(lightsData_.emissiveColorEdit, &CQColorEdit::colorChanged,
            this, &Control3D::emissiveColorSlot);
    connect(lightsData_.emissiveEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::emissiveSlot);
    connect(lightsData_.shininessEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::shininessSlot);

    connect(lightsData_.enabledCheck , &QCheckBox::stateChanged,
            this, &Control3D::lightCheckSlot);
    connect(lightsData_.colorEdit , &CQColorEdit::colorChanged,
            this, &Control3D::lightColorSlot);
    connect(lightsData_.powerEdit , &CQRealSpin::realValueChanged,
            this, &Control3D::lightPowerSlot);
    connect(lightsData_.posEdit   , &CQPoint3DEdit::editingFinished,
            this, &Control3D::lightPosSlot);
    connect(lightsData_.dirEdit   , &CQPoint3DEdit::editingFinished,
            this, &Control3D::lightDirSlot);
    connect(lightsData_.cutoffEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::lightCutoffSlot);
    connect(lightsData_.radiusEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::lightRadiusSlot);
    connect(lightsData_.list, &QListWidget::currentItemChanged,
            this, &Control3D::lightSelectedSlot);
  }
  else {
    disconnect(lightsData_.ambientColorEdit, &CQColorEdit::colorChanged,
               this, &Control3D::ambientColorSlot);
    disconnect(lightsData_.ambientStrengthEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::ambientStrengthSlot);
    disconnect(lightsData_.diffuseEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::diffuseSlot);
    disconnect(lightsData_.specularColorEdit, &CQColorEdit::colorChanged,
               this, &Control3D::specularColorSlot);
    disconnect(lightsData_.specularEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::specularSlot);
    disconnect(lightsData_.emissiveColorEdit, &CQColorEdit::colorChanged,
               this, &Control3D::emissiveColorSlot);
    disconnect(lightsData_.emissiveEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::emissiveSlot);
    disconnect(lightsData_.shininessEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::shininessSlot);

    disconnect(lightsData_.enabledCheck , &QCheckBox::stateChanged,
               this, &Control3D::lightCheckSlot);
    disconnect(lightsData_.colorEdit , &CQColorEdit::colorChanged,
               this, &Control3D::lightColorSlot);
    disconnect(lightsData_.powerEdit , &CQRealSpin::realValueChanged,
               this, &Control3D::lightPowerSlot);
    disconnect(lightsData_.posEdit   , &CQPoint3DEdit::editingFinished,
               this, &Control3D::lightPosSlot);
    disconnect(lightsData_.dirEdit   , &CQPoint3DEdit::editingFinished,
               this, &Control3D::lightDirSlot);
    disconnect(lightsData_.cutoffEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::lightCutoffSlot);
    disconnect(lightsData_.radiusEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::lightRadiusSlot);
    disconnect(lightsData_.list, &QListWidget::currentItemChanged,
               this, &Control3D::lightSelectedSlot);
  }
}

void
Control3D::
updateMaterials()
{
  connectMaterials(false);

  //---

  auto *currentMaterial = canvas_->currentMaterial();

  //---

  if (currentMaterial) {
    materialsData_.diffuseColorEdit->setColor(
      Util::RGBAToQColor(currentMaterial->getDiffuse()));
    materialsData_.specularColorEdit->setColor(
      Util::RGBAToQColor(currentMaterial->getSpecular()));
    materialsData_.emissionColorEdit->setColor(
      Util::RGBAToQColor(currentMaterial->getEmission()));

    materialsData_.emissionEdit    ->setValue(currentMaterial->emissiveStrength());
    materialsData_.specularEdit    ->setValue(currentMaterial->specularStrength());
    materialsData_.shininessEdit   ->setValue(currentMaterial->shininess().value_or(0.0));
    materialsData_.transparencyEdit->setValue(currentMaterial->transparency());
    materialsData_.reflectivityEdit->setValue(currentMaterial->reflectivity());
    materialsData_.refractivityEdit->setValue(currentMaterial->refractivity());
  }

  //---

  if (materialsData_.changed) {
    materialsData_.changed = false;

    materialsData_.list->clear();

    QListWidgetItem *currentItem = nullptr;

    for (auto *material : canvas_->materials()) {
      auto name = QString::fromStdString(material->name());

      auto *item = new QListWidgetItem(name);

      materialsData_.list->addItem(item);

      item->setData(Qt::UserRole, material->id());

      if (material == currentMaterial)
        currentItem = item;
    }

    if (currentItem)
      materialsData_.list->setCurrentItem(currentItem, QItemSelectionModel::Select);
  }

  //---

  connectMaterials(true);
}

void
Control3D::
connectMaterials(bool b)
{
  if (b) {
    connect(materialsData_.diffuseColorEdit, &CQColorEdit::colorChanged,
            this, &Control3D::materialDiffuseColorSlot);
    connect(materialsData_.specularColorEdit, &CQColorEdit::colorChanged,
            this, &Control3D::materialSpecularColorSlot);
    connect(materialsData_.emissionColorEdit, &CQColorEdit::colorChanged,
            this, &Control3D::materialEmissionColorSlot);

    connect(materialsData_.emissionEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::materialEmissionSlot);
    connect(materialsData_.specularEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::materialSpecularSlot);
    connect(materialsData_.shininessEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::materialShininessSlot);

    connect(materialsData_.transparencyEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::transparencySlot);
    connect(materialsData_.reflectivityEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::reflectivitySlot);
    connect(materialsData_.refractivityEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::refractivitySlot);

    connect(materialsData_.list, &QListWidget::currentItemChanged,
            this, &Control3D::materialSelectedSlot);
  }
  else {
    disconnect(materialsData_.diffuseColorEdit, &CQColorEdit::colorChanged,
               this, &Control3D::materialDiffuseColorSlot);
    disconnect(materialsData_.specularColorEdit, &CQColorEdit::colorChanged,
               this, &Control3D::materialSpecularColorSlot);
    disconnect(materialsData_.emissionColorEdit, &CQColorEdit::colorChanged,
               this, &Control3D::materialEmissionColorSlot);

    disconnect(materialsData_.emissionEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::materialEmissionSlot);
    disconnect(materialsData_.specularEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::materialSpecularSlot);
    disconnect(materialsData_.shininessEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::materialShininessSlot);

    disconnect(materialsData_.transparencyEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::transparencySlot);
    disconnect(materialsData_.reflectivityEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::reflectivitySlot);
    disconnect(materialsData_.refractivityEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::refractivitySlot);

    disconnect(materialsData_.list, &QListWidget::currentItemChanged,
               this, &Control3D::materialSelectedSlot);
  }
}

void
Control3D::
updateTextures()
{
  connectTextures(false);

  //---

  if (texturesData_.changed) {
    texturesData_.changed = false;

    texturesData_.list->clear();

    for (auto *texture : TextureMgr::instance()->textures()) {
      auto tname = QString::fromStdString(texture->name());

      auto name = QString("%1.%2").arg(tname).arg(texture->id());

      auto *item = new QListWidgetItem(name);

      texturesData_.list->addItem(item);

      item->setData(Qt::UserRole, int(texture->id()));
    }
  }

  //---

  connectTextures(true);
}

void
Control3D::
connectTextures(bool b)
{
  if (b) {
    connect(texturesData_.list, &QListWidget::currentItemChanged,
            this, &Control3D::textureSelectedSlot);
  }
  else {
    disconnect(texturesData_.list, &QListWidget::currentItemChanged,
               this, &Control3D::textureSelectedSlot);
  }
}

void
Control3D::
updateObjects()
{
  connectObjects(false);

  //---

  if (objectsData_.changed) {
    objectsData_.changed = false;
  }

  //---

  objectsData_.list->updateAll();

  auto *currentObj = getCurrentObject();

  if (currentObj) {
    objectsData_.posEdit   ->setValue(currentObj->position());
    objectsData_.scaleEdit ->setValue(currentObj->scales());
    objectsData_.rotateEdit->setValue(currentObj->anglesDeg());

    objectsData_.bboxEdit->setValue(currentObj->bbox());
  }

  //---

  connectObjects(true);
}

void
Control3D::
connectObjects(bool b)
{
  if (b) {
    connect(objectsData_.list, SIGNAL(currentItemChanged()),
            this, SLOT(objectSelectedSlot()));

    connect(objectsData_.posEdit, &CQPoint3DEdit::editingFinished,
            this, &Control3D::objectPosSlot);
    connect(objectsData_.scaleEdit, &CQPoint3DEdit::editingFinished,
            this, &Control3D::objectScaleSlot);
    connect(objectsData_.rotateEdit, &CQPoint3DEdit::editingFinished,
            this, &Control3D::objectRotateSlot);
  }
  else {
    disconnect(objectsData_.list, SIGNAL(currentItemChanged()),
               this, SLOT(objectSelectedSlot()));

    disconnect(objectsData_.posEdit, &CQPoint3DEdit::editingFinished,
               this, &Control3D::objectPosSlot);
    disconnect(objectsData_.scaleEdit, &CQPoint3DEdit::editingFinished,
               this, &Control3D::objectScaleSlot);
    disconnect(objectsData_.rotateEdit, &CQPoint3DEdit::editingFinished,
               this, &Control3D::objectRotateSlot);
  }
}

void
Control3D::
updateOverview()
{
  auto *overview = canvas_->app()->overview3D();
  if (! overview) return;

  connectOverview(false);

  //---

  overviewData_.wireFrameCheck->setChecked(overview->isWireframe());
  overviewData_.solidCheck    ->setChecked(overview->isSolid());
  overviewData_.zclipCheck    ->setChecked(overview->isZClip());
  overviewData_.cameraCheck   ->setChecked(overview->isCameraVisible());
  overviewData_.lightCheck    ->setChecked(overview->isLightsVisible());
  overviewData_.basisCheck    ->setChecked(overview->isBasisVisible());

  overviewData_.bgColor      ->setColor(overview->bgColor());
  overviewData_.strokeColor  ->setColor(overview->strokeColor());
  overviewData_.strokeAlpha  ->setValue(overview->strokeAlpha());
  overviewData_.fillColor    ->setColor(overview->fillColor());
  overviewData_.fillAlpha    ->setValue(overview->fillAlpha());
  overviewData_.selectedColor->setColor(overview->selectedColor());
  overviewData_.pointSize    ->setValue(overview->pointSize());

  //---

  connectOverview(true);
}

void
Control3D::
connectOverview(bool b)
{
  if (b) {
    connect(overviewData_.wireFrameCheck, &QCheckBox::stateChanged,
            this, &Control3D::overviewWireframeSlot);
    connect(overviewData_.solidCheck, &QCheckBox::stateChanged,
            this, &Control3D::overviewSolidSlot);
    connect(overviewData_.zclipCheck, &QCheckBox::stateChanged,
            this, &Control3D::overviewZClipSlot);
    connect(overviewData_.cameraCheck, &QCheckBox::stateChanged,
            this, &Control3D::overviewShowCameraSlot);
    connect(overviewData_.lightCheck, &QCheckBox::stateChanged,
            this, &Control3D::overviewShowLightSlot);
    connect(overviewData_.basisCheck, &QCheckBox::stateChanged,
            this, &Control3D::overviewShowBasisSlot);

    connect(overviewData_.bgColor, &CQColorEdit::colorChanged,
            this, &Control3D::overviewBgColorSlot);
    connect(overviewData_.strokeColor, &CQColorEdit::colorChanged,
            this, &Control3D::overviewStrokeColorSlot);
    connect(overviewData_.strokeAlpha, &CQRealSpin::realValueChanged,
            this, &Control3D::overviewStrokeAlphaSlot);
    connect(overviewData_.fillColor, &CQColorEdit::colorChanged,
            this, &Control3D::overviewFillColorSlot);
    connect(overviewData_.fillAlpha, &CQRealSpin::realValueChanged,
            this, &Control3D::overviewFillAlphaSlot);
    connect(overviewData_.selectedColor, &CQColorEdit::colorChanged,
            this, &Control3D::overviewSelectedColorSlot);
    connect(overviewData_.pointSize, &CQRealSpin::realValueChanged,
            this, &Control3D::overviewPointSizeSlot);
  }
  else {
    disconnect(overviewData_.wireFrameCheck, &QCheckBox::stateChanged,
               this, &Control3D::overviewWireframeSlot);
    disconnect(overviewData_.solidCheck, &QCheckBox::stateChanged,
               this, &Control3D::overviewSolidSlot);
    disconnect(overviewData_.zclipCheck, &QCheckBox::stateChanged,
               this, &Control3D::overviewZClipSlot);
    disconnect(overviewData_.cameraCheck, &QCheckBox::stateChanged,
               this, &Control3D::overviewShowCameraSlot);
    disconnect(overviewData_.lightCheck, &QCheckBox::stateChanged,
               this, &Control3D::overviewShowLightSlot);
    disconnect(overviewData_.basisCheck, &QCheckBox::stateChanged,
               this, &Control3D::overviewShowBasisSlot);

    disconnect(overviewData_.bgColor, &CQColorEdit::colorChanged,
               this, &Control3D::overviewBgColorSlot);
    disconnect(overviewData_.strokeColor, &CQColorEdit::colorChanged,
               this, &Control3D::overviewStrokeColorSlot);
    disconnect(overviewData_.strokeAlpha, &CQRealSpin::realValueChanged,
               this, &Control3D::overviewStrokeAlphaSlot);
    disconnect(overviewData_.fillColor, &CQColorEdit::colorChanged,
               this, &Control3D::overviewFillColorSlot);
    disconnect(overviewData_.fillAlpha, &CQRealSpin::realValueChanged,
               this, &Control3D::overviewFillAlphaSlot);
    disconnect(overviewData_.selectedColor, &CQColorEdit::colorChanged,
               this, &Control3D::overviewSelectedColorSlot);
    disconnect(overviewData_.pointSize, &CQRealSpin::realValueChanged,
               this, &Control3D::overviewPointSizeSlot);
  }
}

//---

void
Control3D::
connectSelection(bool b)
{
  if (b) {
    connect(selectionData_.list, &QListWidget::currentItemChanged,
            this, &Control3D::selectionSelectedSlot);
  }
  else {
    disconnect(selectionData_.list, &QListWidget::currentItemChanged,
               this, &Control3D::selectionSelectedSlot);
  }
}

void
Control3D::
updateSelection()
{
  connectSelection(false);

  if (selectionData_.changed) {
    auto addItem = [&](const QString &name, int objectInd, int bufferInd, int ind) {
      auto *item = new QListWidgetItem(name);

      selectionData_.list->addItem(item);

      QStringList ids;
      ids << QString::number(objectInd);
      ids << QString::number(bufferInd);
      ids << QString::number(ind);

      item->setData(Qt::UserRole, ids);
    };

    selectionData_.changed = false;

    selectionData_.list->clear();

    for (auto *object : canvas_->objects()) {
      if (! object || ! object->isVisible())
        continue;

      if      (canvas_->editType() == Canvas3D::EditType::POINT) {
        const auto &selectedPoints = object->selectedPoints();

        for (const auto &bufferInds : selectedPoints) {
          for (auto ind : bufferInds.second) {
            addItem(QString("Point: %1").arg(ind), object->ind(), bufferInds.first, ind);
          }
        }
      }
      else if (canvas_->editType() == Canvas3D::EditType::LINE) {
        const auto &selectedLines = object->selectedLines();

        for (const auto &bufferInds : selectedLines) {
          for (auto ind : bufferInds.second) {
            addItem(QString("Line: %1").arg(ind), object->ind(), bufferInds.first, ind);
          }
        }
      }
      else if (canvas_->editType() == Canvas3D::EditType::FACE) {
        const auto &selectedFaces = object->selectedFaces();

        for (const auto &bufferInds : selectedFaces) {
          for (auto ind : bufferInds.second) {
            addItem(QString("Face: %1").arg(ind), object->ind(), bufferInds.first, ind);
          }
        }
      }
      else if (canvas_->editType() == Canvas3D::EditType::OBJECT) {
        if (object->isSelected())
          addItem(QString("Object: %1").arg(object->id()), object->ind(), 0, 0);
      }
      else
        continue;
    }
  }

  connectSelection(true);
}

//---

Object3D *
Control3D::
getCurrentObject() const
{
  auto *indObj = objectsData_.list->getObjectListSelected();

  return indObj;
}

//---

void
Control3D::
depthTestSlot(int b)
{
  canvas_->setDepthTest(b);
  canvas_->update();
}

void
Control3D::
cullFaceSlot(int b)
{
  canvas_->setCullFace(b);
  canvas_->update();
}

void
Control3D::
frontFaceSlot(int b)
{
  canvas_->setFrontFace(b);
  canvas_->update();
}

void
Control3D::
bgColorSlot(const QColor &c)
{
  canvas_->setBgColor(c);
  canvas_->update();
}

void
Control3D::
wireframeColorSlot(const QColor &c)
{
  canvas_->setWireframeColor(Util::QColorToRGBA(c));
  canvas_->update();
}

void
Control3D::
wireframeAlphaSlot(double r)
{
  canvas_->setWireframeAlpha(r);
  canvas_->update();
}

void
Control3D::
selectColorSlot(const QColor &c)
{
  canvas_->setSelectColor(Util::QColorToRGBA(c));
  canvas_->update();
}

void
Control3D::
selectAlphaSlot(double r)
{
  canvas_->setSelectAlpha(r);
  canvas_->update();
}

void
Control3D::
selectPointSizeSlot(double r)
{
  canvas_->setSelectPointSize(r);
  canvas_->update();
}


void
Control3D::
selectLineWidthSlot(double r)
{
  canvas_->setSelectLineWidth(r);
  canvas_->update();
}

void
Control3D::
selectFaceCenterSlot(int s)
{
  canvas_->setShowSelectedFaceCenter(s);
  canvas_->update();
}

void
Control3D::
editColorSlot(const QColor &c)
{
  canvas_->setEditColor(Util::QColorToRGBA(c));
  canvas_->update();
}

void
Control3D::
editFaceAlphaSlot(double r)
{
  canvas_->setEditFaceAlpha(r);
  canvas_->update();
}

void
Control3D::
editPointSizeSlot(double r)
{
  canvas_->setEditPointSize(r);
  canvas_->update();
}


void
Control3D::
editLineWidthSlot(double r)
{
  canvas_->setEditLineWidth(r);
  canvas_->update();
}

void
Control3D::
enableShadowSlot(int b)
{
  canvas_->setShadowed(b);
  canvas_->update();
}

void
Control3D::
debugShadowSlot(int b)
{
  canvas_->setShadowDebug(b);
  canvas_->update();
}

void
Control3D::
showSkyboxSlot(int b)
{
  canvas_->setShowSkybox(b);
  canvas_->update();
}

void
Control3D::
debugSkyboxSlot(int b)
{
  canvas_->setDebugSkybox(b);
  canvas_->update();
}

void
Control3D::
showOrientSlot(int b)
{
  canvas_->setShowOrient(b);
  canvas_->update();
}

//---

void
Control3D::
ambientColorSlot(const QColor &c)
{
  canvas_->setAmbientColor(Util::QColorToRGBA(c));
  canvas_->update();
}

void
Control3D::
ambientStrengthSlot()
{
  auto a = lightsData_.ambientStrengthEdit->value();

  canvas_->setAmbientStrength(a);
  canvas_->update();
}

void
Control3D::
diffuseSlot()
{
  auto a = lightsData_.diffuseEdit->value();

  canvas_->setDiffuseStrength(a);
  canvas_->update();
}

void
Control3D::
specularColorSlot(const QColor &c)
{
  canvas_->setSpecularColor(Util::QColorToRGBA(c));
  canvas_->update();
}

void
Control3D::
specularSlot()
{
  auto a = lightsData_.specularEdit->value();

  canvas_->setSpecularStrength(a);
  canvas_->update();
}

void
Control3D::
emissiveColorSlot(const QColor &c)
{
  canvas_->setEmissiveColor(Util::QColorToRGBA(c));
  canvas_->update();
}

void
Control3D::
emissiveSlot()
{
  auto a = lightsData_.emissiveEdit->value();

  canvas_->setEmissiveStrength(a);
  canvas_->update();
}

void
Control3D::
shininessSlot()
{
  auto a = lightsData_.shininessEdit->value();

  canvas_->setShininess(a);
  canvas_->update();
}

void
Control3D::
cameraTypeSlot(int i)
{
  if      (i == 0)
    canvas_->setCameraType(Canvas3D::CameraType::MODEL);
  else if (i == 1)
    canvas_->setCameraType(Canvas3D::CameraType::FIRST_PERSON);
  else if (i == 2)
    canvas_->setCameraType(Canvas3D::CameraType::ORTHO);
}

void
Control3D::
cameraOrthoTypeSlot(int i)
{
  auto *camera = canvas_->orthoCamera();

  if      (i == 0)
    camera->setOrthoType(OrthoCamera::OthroType::TOP);
  else if (i == 1)
    camera->setOrthoType(OrthoCamera::OthroType::BOTTOM);
  else if (i == 2)
    camera->setOrthoType(OrthoCamera::OthroType::LEFT);
  else if (i == 3)
    camera->setOrthoType(OrthoCamera::OthroType::RIGHT);
  else if (i == 4)
    camera->setOrthoType(OrthoCamera::OthroType::FRONT);
  else if (i == 5)
    camera->setOrthoType(OrthoCamera::OthroType::BACK);
}

#if 0
void
Control3D::
cameraRotateSlot(int b)
{
  auto *camera = canvas_->currentCamera();

  disconnect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));

  camera()->setRotate(b);

  connect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));
}
#endif

#if 0
void
Control3D::
cameraZoomSlot(double r)
{
  auto *camera = canvas_->currentCamera();

  disconnect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));

  camera->setZoom(r);

  connect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));
}
#endif

void
Control3D::
cameraPitchSlot(double r)
{
  auto *camera = canvas_->currentCamera();

  disconnect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));

  camera->setPitch(CMathGen::DegToRad(r));

  connect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));
}

void
Control3D::
cameraYawSlot(double r)
{
  auto *camera = canvas_->currentCamera();

  disconnect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));

  camera->setYaw(CMathGen::DegToRad(r));

  connect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));
}

void
Control3D::
cameraRollSlot(double r)
{
  auto *camera = canvas_->currentCamera();

  disconnect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));

  camera->setRoll(CMathGen::DegToRad(r));

  connect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));
}

void
Control3D::
cameraNearSlot(double r)
{
  auto *camera = canvas_->currentCamera();

  disconnect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));

  camera->setNear(r);

  connect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));
}

void
Control3D::
cameraFarSlot(double r)
{
  auto *camera = canvas_->currentCamera();

  disconnect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));

  camera->setFar(r);

  connect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));
}

void
Control3D::
cameraFovSlot(double r)
{
  auto *camera = canvas_->currentCamera();

  disconnect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));

  camera->setFov(r);

  connect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));
}

void
Control3D::
cameraPosSlot()
{
  auto *camera = canvas_->currentCamera();

  disconnect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));

  auto p = cameraData_.posEdit->getValue();

  camera->setPosition(CVector3D(p.x, p.y, p.z));

  connect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));
}

void
Control3D::
cameraOriginSlot()
{
  auto *camera = canvas_->currentCamera();

  disconnect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));

  auto p = cameraData_.originEdit->getValue();

  camera->setOrigin(CVector3D(p.x, p.y, p.z));

  connect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));
}

void
Control3D::
cameraDistanceSlot(double r)
{
  auto *camera = canvas_->currentCamera();

  disconnect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));

  camera->setDistance(r);

  connect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));
}

void
Control3D::
resetCameraSlot()
{
  canvas_->resetCamera();
}

//---

void
Control3D::
lightSelectedSlot(QListWidgetItem *item, QListWidgetItem *)
{
  int id = item->data(Qt::UserRole).toInt();

  canvas_->setLightNum(id);

  updateLights();
}

void
Control3D::
lightCheckSlot(int b)
{
  auto *light = canvas_->currentLight();

  light->setEnabled(b);
  canvas_->update();
}

void
Control3D::
lightColorSlot(const QColor &c)
{
  auto *light = canvas_->currentLight();

  light->setDiffuse(Util::QColorToRGBA(c));
  canvas_->update();
}

void
Control3D::
lightPowerSlot(double r)
{
  auto *light = canvas_->currentLight();

  light->setPower(r);
  canvas_->update();
}

void
Control3D::
lightPosSlot()
{
  auto *light = canvas_->currentLight();

  auto p = lightsData_.posEdit->getValue();
  light->setPosition(CVector3D(p.x, p.y, p.z));
  canvas_->update();
}

void
Control3D::
lightDirSlot()
{
  auto *light = canvas_->currentLight();

  auto p = lightsData_.dirEdit->getValue();
  if (light->getType() == Light3D::Type::SPOT)
    light->setSpotDirection(CVector3D(p.x, p.y, p.z));
  else
    light->setDirection(CVector3D(p.x, p.y, p.z));
  canvas_->update();
}

void
Control3D::
lightCutoffSlot(double r)
{
  auto *light = canvas_->currentLight();

  light->setSpotCutOffAngle(r);
  canvas_->update();
}

void
Control3D::
lightRadiusSlot(double r)
{
  auto *light = canvas_->currentLight();

  light->setPointRadius(r);
  canvas_->update();
}

void
Control3D::
resetLightSlot()
{
  auto *light = canvas_->currentLight();

  canvas_->resetLight(light);
}

void
Control3D::
materialDiffuseColorSlot(const QColor &c)
{
  auto *material = canvas_->currentMaterial();
  if (! material) return;

  material->setDiffuse(Util::QColorToRGBA(c));
  canvas_->update();
}

void
Control3D::
materialSpecularColorSlot(const QColor &c)
{
  auto *material = canvas_->currentMaterial();
  if (! material) return;

  material->setSpecular(Util::QColorToRGBA(c));
  canvas_->update();
}

void
Control3D::
materialEmissionColorSlot(const QColor &c)
{
  auto *material = canvas_->currentMaterial();
  if (! material) return;

  material->setEmission(Util::QColorToRGBA(c));
  canvas_->update();
}

void
Control3D::
materialEmissionSlot(double r)
{
  auto *material = canvas_->currentMaterial();
  if (! material) return;

  material->setEmissiveStrength(r);
  canvas_->update();
}

void
Control3D::
materialSpecularSlot(double r)
{
  auto *material = canvas_->currentMaterial();
  if (! material) return;

  material->setSpecularStrength(r);
  canvas_->update();
}

void
Control3D::
materialShininessSlot(double r)
{
  auto *material = canvas_->currentMaterial();
  if (! material) return;

  material->setShininess(r);
  canvas_->update();
}

void
Control3D::
transparencySlot(double r)
{
  auto *material = canvas_->currentMaterial();
  if (! material) return;

  material->setTransparency(r);
  canvas_->update();
}

void
Control3D::
reflectivitySlot(double r)
{
  auto *material = canvas_->currentMaterial();
  if (! material) return;

  material->setReflectivity(r);
  canvas_->update();
}

void
Control3D::
refractivitySlot(double r)
{
  auto *material = canvas_->currentMaterial();
  if (! material) return;

  material->setRefractivity(r);
  canvas_->update();
}

void
Control3D::
materialSelectedSlot(QListWidgetItem *item, QListWidgetItem *)
{
  int id = item->data(Qt::UserRole).toInt();

  canvas_->setMaterialId(id);

  updateMaterials();
}

//---

void
Control3D::
textureSelectedSlot(QListWidgetItem *item, QListWidgetItem *)
{
  int id = item->data(Qt::UserRole).toInt();

  auto *texture = TextureMgr::instance()->getTexture(id);
  if (! texture) return;

  auto *glTexture = texture->glTexture(canvas_);
  if (! glTexture) return;

  auto image = glTexture->getImage();

  texturesData_.image->clear();

  texturesData_.image->addImage(glTexture->getImage());
}

//---

void
Control3D::
objectSelectedSlot()
{
  auto *object = objectsData_.list->getObjectListSelected();
  if (! object) return;

  canvas_->selectObject(object, /*clear*/true);

  auto skipPropeties = QStringList() <<
    "xangle" << "yangle" << "zangle" <<
    "xpos" << "ypos" << "zpos" <<
    "xscale" << "yscale" << "zscale";

  objectsData_.tree->clear();

  if (object) {
    auto properties = CQUtil::getPropertyList(object);

    for (auto &prop : properties) {
      if (skipPropeties.contains(prop))
        continue;

      objectsData_.tree->addProperty("", object, prop);
    }
  }
}

void
Control3D::
objectPosSlot()
{
  auto p = objectsData_.posEdit->getValue();

  auto *obj = getCurrentObject();

  if (obj)
    obj->setPosition(p);
}

void
Control3D::
objectScaleSlot()
{
  auto p = objectsData_.scaleEdit->getValue();

  auto *obj = getCurrentObject();

  if (obj)
    obj->setScales(p);
}

void
Control3D::
objectRotateSlot()
{
  auto p = objectsData_.rotateEdit->getValue();

  auto *obj = getCurrentObject();

  if (obj)
    obj->setAnglesDeg(p);
}

void
Control3D::
objectApplySlot()
{
  auto *obj = getCurrentObject();

  if (obj)
    obj->applyTransform();
}

void
Control3D::
overviewWireframeSlot(int state)
{
  auto *overview = canvas_->app()->overview3D();

  if (overview)
    overview->setWireframe(state);
}

void
Control3D::
overviewSolidSlot(int state)
{
  auto *overview = canvas_->app()->overview3D();

  if (overview)
    overview->setSolid(state);
}

void
Control3D::
overviewZClipSlot(int state)
{
  auto *overview = canvas_->app()->overview3D();

  if (overview)
    overview->setZClip(state);
}

void
Control3D::
overviewShowCameraSlot(int state)
{
  auto *overview = canvas_->app()->overview3D();

  if (overview)
    overview->setCameraVisible(state);
}

void
Control3D::
overviewShowLightSlot(int state)
{
  auto *overview = canvas_->app()->overview3D();

  if (overview)
    overview->setLightsVisible(state);
}

void
Control3D::
overviewShowBasisSlot(int state)
{
  auto *overview = canvas_->app()->overview3D();

  if (overview)
    overview->setBasisVisible(state);
}

void
Control3D::
overviewBgColorSlot(const QColor &c)
{
  auto *overview = canvas_->app()->overview3D();

  if (overview)
    overview->setBgColor(c);
}

void
Control3D::
overviewStrokeColorSlot(const QColor &c)
{
  auto *overview = canvas_->app()->overview3D();

  if (overview)
    overview->setStrokeColor(c);
}

void
Control3D::
overviewStrokeAlphaSlot(double a)
{
  auto *overview = canvas_->app()->overview3D();

  if (overview)
    overview->setStrokeAlpha(a);
}

void
Control3D::
overviewFillColorSlot(const QColor &c)
{
  auto *overview = canvas_->app()->overview3D();

  if (overview)
    overview->setFillColor(c);
}

void
Control3D::
overviewFillAlphaSlot(double a)
{
  auto *overview = canvas_->app()->overview3D();

  if (overview)
    overview->setFillAlpha(a);
}

void
Control3D::
overviewSelectedColorSlot(const QColor &c)
{
  auto *overview = canvas_->app()->overview3D();

  if (overview)
    overview->setSelectedColor(c);
}

void
Control3D::
overviewPointSizeSlot(double s)
{
  auto *overview = canvas_->app()->overview3D();

  if (overview)
    overview->setPointSize(s);
}

//---

void
Control3D::
selectionSelectedSlot(QListWidgetItem *item, QListWidgetItem *)
{
  auto ids = item->data(Qt::UserRole).toStringList();

  auto objectInd = ids[0].toInt();
  auto bufferInd = ids[1].toInt();
  auto ind       = ids[2].toInt();

  std::cerr << objectInd << " " << bufferInd << " " << ind << "\n";

  auto *object = canvas_->getObjectByInd(objectInd);
  if (! object) return;

  auto *buffer = CQGLBufferMgrInst->getBuffer(bufferInd);
  if (! buffer) return;

  if      (canvas_->editType() == Canvas3D::EditType::POINT) {
    auto *pointData = object->getPointData(ind);
    if (! pointData) return;

    CQGLBuffer::PointData pointData1;
    buffer->getPointData(pointData->ind, pointData1);

    std::cerr << "I: " << pointData->ind << " ";
    pointData1.print(std::cerr);
  }
  else if (canvas_->editType() == Canvas3D::EditType::LINE) {
  }
  else if (canvas_->editType() == Canvas3D::EditType::FACE) {
    auto *faceData = object->getFaceData(ind);
    if (! faceData) return;

    for (int i = 0; i < faceData->len; ++i) {
      CQGLBuffer::PointData pointData;
      buffer->getPointData(faceData->pos + i, pointData);

      std::cerr << "I: " << faceData->pos + i << " ";
      pointData.print(std::cerr);
    }
  }
  else if (canvas_->editType() == Canvas3D::EditType::OBJECT) {
  }
}

//---

bool
Control3D::
createUi(const QString &ui)
{
  if (! xml_)
    xml_ = new Xml3D(this);

  return xml_->createWidgetsFromString(uiFrame_, ui.toStdString());
}

bool
Control3D::
getUiValue(const QString &name, QVariant &value) const
{
  if (! xml_) return false;

  value = xml_->getExecData(name);

  return true;
}

bool
Control3D::
setUiValue(const QString &name, const QVariant &value)
{
  if (! xml_) return false;

  xml_->setExecData(name, value);

  return true;
}

bool
Control3D::
getUiWidgetValue(const QString &widget, const QString &name, QVariant &value) const
{
  if (! xml_) return false;

  auto *w = xml_->getWidget(widget);
  if (! w) return false;

  if (! xml_->getWidgetData(w, name, value))
    return false;

  return true;
}

bool
Control3D::
setUiWidgetValue(const QString &widget, const QString &name, const QVariant &value)
{
  if (! xml_) return false;

  auto *w = xml_->getWidget(widget);
  if (! w) return false;

  if (! xml_->setWidgetData(w, name, value))
    return false;

  return true;
}

}
