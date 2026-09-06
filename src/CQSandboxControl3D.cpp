#include <CQSandboxControl3D.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxLight3D.h>
#include <CQSandboxCamera.h>
#include <CQSandboxOrthoCamera.h>
#include <CQSandboxOverview3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQColorEdit.h>
#include <CQPoint3DEdit.h>
#include <CQRealSpin.h>
#include <CQPropertyViewTree.h>
#include <CQUtil.h>
#include <CQXml.h>

#include <QTabWidget>
#include <QGroupBox>
#include <QListWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>

namespace CQSandbox {

CPoint3D vectorToPoint(const CGLVector3D &v) {
  return CPoint3D(v.getX(), v.getY(), v.getZ());
}

CPoint3D vectorToPoint(const CVector3D &v) {
  return v.point();
}

QColor vectorToColor(const CVector3D &v) {
  QColor c;
  c.setRgbF(v.getX(), v.getY(), v.getZ());
  return c;
}

QColor vectorToColor(const CGLVector3D &v) {
  QColor c;
  c.setRgbF(v.getX(), v.getY(), v.getZ());
  return c;
}

CVector3D colorToVector(const QColor &c) {
  return CVector3D(c.redF(), c.greenF(), c.blueF());
}

}

//---

namespace CQSandbox {

class Xml3D : public CQXml {
 public:
  Xml3D(Control3D *control) :
   CQXml(), control_(control) {
  }

  void execSlot(const QString &value, const QStringList &args) override {
    auto *canvas = control_->canvas();

    auto text = getExecData("text").toString();
    canvas->tcl()->createVar("execText", text);

    canvas->tcl()->createVar("execArgs", args);

    canvas->runTclCmd(value);
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
  auto *layout = new QVBoxLayout(this);

  //---

  tab_ = new QTabWidget;

  layout->addWidget(tab_);

  //---

  auto *controlFrame  = addControlFrame();
  auto *cameraFrame   = addCameraFrame();
  auto *lightFrame    = addLightFrame();
  auto *objectsFrame  = addObjectsFrame();
  auto *overviewFrame = addOverviewFrame();

  uiFrame_ = new QFrame;

  tab_->addTab(controlFrame , "Control");
  tab_->addTab(cameraFrame  , "Camera");
  tab_->addTab(lightFrame   , "Lights");
  tab_->addTab(objectsFrame , "Objects");
  tab_->addTab(overviewFrame, "Overview");
  tab_->addTab(uiFrame_     , "UI");

  //---

  auto *updateButton = new QPushButton("Update");
  auto *closeButton  = new QPushButton("Close");

  auto *buttonFrame  = new QFrame(this);
  auto *buttonLayout = new QHBoxLayout(buttonFrame);

  layout->addWidget(buttonFrame);

  buttonLayout->addStretch(1);
  buttonLayout->addWidget(updateButton);
  buttonLayout->addWidget(closeButton);

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
  for (auto *camera : canvas_->cameras())
    connect(camera, SIGNAL(stateChangedSignal()), this, SLOT(updateSlot()));

  for (auto *light : canvas_->lights())
    connect(light, SIGNAL(changedSignal()), this, SLOT(updateSlot()));

  connect(canvas_, SIGNAL(objectsChanged()), this, SLOT(objectAddedSlot()));

  connect(canvas_, SIGNAL(lightAdded()), this, SLOT(lightAddedSlot()));

  connect(canvas_, SIGNAL(uiUpdateSignal()), this, SLOT(uiSlot()));
}

QFrame *
Control3D::
addControlFrame()
{
  auto *frame  = new QFrame(this);
  auto *layout = new QGridLayout(frame);

  //---

  int row = 0;

  auto addLabelEdit = [&](const QString &label, QWidget *w) {
    layout->addWidget(new QLabel(label), row, 0);
    layout->addWidget(w, row, 1);
    ++row;
  };

  auto addCheck = [&](const QString &label, const char *slotName) {
    auto *checkBox = new QCheckBox;
    connect(checkBox, SIGNAL(stateChanged(int)), this, slotName);
    addLabelEdit(label, checkBox);
    return checkBox;
  };

  auto addColorEdit = [&](const QString &label, const char *slotName) {
    auto *edit = new CQColorEdit;
    connect(edit, SIGNAL(colorChanged(const QColor &)), this, slotName);
    addLabelEdit(label, edit);
    return edit;
  };

  //---

  controlData_.depthTestCheck = addCheck("Depth Test", SLOT(depthTestSlot(int)));
  controlData_.cullFaceCheck  = addCheck("Cull Face" , SLOT(cullFaceSlot(int)));
  controlData_.frontFaceCheck = addCheck("Front Face", SLOT(frontFaceSlot(int)));

  //---

  controlData_.bgColorEdit = addColorEdit("Bg Color", SLOT(bgColorSlot(const QColor &)));

  //---

  layout->setRowStretch(row, 1);

  //---

  return frame;
}

QFrame *
Control3D::
addCameraFrame()
{
  auto *frame  = new QFrame;
  auto *layout = new QVBoxLayout(frame);

  auto *controlFrame  = new QFrame;
  auto *controlLayout = new QGridLayout(controlFrame);

  layout->addWidget(controlFrame);

  int cameraRow = 0;

  auto addLabelEdit = [&](const QString &label, QWidget *w) {
    controlLayout->addWidget(new QLabel(label), cameraRow, 0);
    controlLayout->addWidget(w, cameraRow, 1);
    ++cameraRow;
  };

  auto addRealEdit = [&](const QString &label, const char *slotName) {
    auto *edit = new CQRealSpin;
    connect(edit, SIGNAL(realValueChanged(double)), this, slotName);
    addLabelEdit(label, edit);
    return edit;
  };

  auto addPoint3DEdit = [&](const QString &label, const char *slotName) {
    auto *edit = new CQPoint3DEdit;
    connect(edit, SIGNAL(editingFinished()), this, slotName);
    addLabelEdit(label, edit);
    return edit;
  };

#if 0
  auto addCheck = [&](const QString &label, const char *slotName) {
    auto *check = new QCheckBox;
    connect(check , SIGNAL(stateChanged(int)), this, slotName);
    addLabelEdit(label, check);
    return check;
  };
#endif

  auto addCombo = [&](const QString &label, const QStringList &names, const char *slotName) {
    auto *combo = new QComboBox;
    for (const auto &name : names)
      combo->addItem(name);
    connect(combo , SIGNAL(currentIndexChanged(int)), this, slotName);
    addLabelEdit(label, combo);
    return combo;
  };

  //---

  cameraData_.typeCombo = addCombo("Type", QStringList() <<
    "Free" << "First Person" << "Ortho", SLOT(cameraTypeSlot(int)));

  cameraData_.orthoTypeCombo = addCombo("Ortho Type", QStringList() <<
    "Top" << "Bottom" << "Left" << "Right" << "Front" << "Back",
    SLOT(cameraOrthoTypeSlot(int)));

#if 0
  cameraData_.rotateCheck = addCheck("Rotate", SLOT(cameraRotateSlot(int)));

  cameraData_.zoomEdit = addRealEdit("Zoom", SLOT(cameraZoomSlot(double)));
#endif

  cameraData_.pitchEdit = addRealEdit("Pitch", SLOT(cameraPitchSlot(double)));
  cameraData_.yawEdit   = addRealEdit("Yaw"  , SLOT(cameraYawSlot(double)));
  cameraData_.rollEdit  = addRealEdit("Roll" , SLOT(cameraRollSlot(double)));

  cameraData_.nearEdit = addRealEdit("Near", SLOT(cameraNearSlot(double)));
  cameraData_.farEdit  = addRealEdit("Far" , SLOT(cameraFarSlot(double)));
  cameraData_.fovEdit  = addRealEdit("FOV" , SLOT(cameraFovSlot(double)));

  cameraData_.originEdit   = addPoint3DEdit("Origin"  , SLOT(cameraOriginSlot()));
  cameraData_.posEdit      = addPoint3DEdit("Position", SLOT(cameraPosSlot()));
  cameraData_.distanceEdit = addRealEdit   ("Distance", SLOT(cameraDistanceSlot(double)));

  //---

  controlLayout->setRowStretch(cameraRow, 1);

  //---

  auto *resetButton = new QPushButton("Reset");

  auto *buttonFrame  = new QFrame(this);
  auto *buttonLayout = new QHBoxLayout(buttonFrame);

  layout->addWidget(buttonFrame);

  buttonLayout->addStretch(1);
  buttonLayout->addWidget(resetButton);

  connect(resetButton, &QPushButton::clicked, this, &Control3D::resetCameraSlot);

  //---

  return frame;
}

QFrame *
Control3D::
addLightFrame()
{
  auto *frame  = new QFrame;
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

  lightData_.ambientColorEdit = addColorEdit("Ambient Color");

  lightData_.ambientStrengthEdit = addRealEdit("Ambient Strength");
  lightData_.ambientStrengthEdit->setRange(0.0, 1.0);

  //---

  lightData_.diffuseEdit = addRealEdit("Diffuse Strength");
  lightData_.diffuseEdit->setRange(0.0, 1.0);

  //---

  lightData_.specularColorEdit = addColorEdit("Specular Color");

  lightData_.specularEdit = addRealEdit("Specular Strength");
  lightData_.specularEdit->setRange(0.0, 1.0);

  //---

  lightData_.emissiveColorEdit = addColorEdit("Emissive Color");

  lightData_.emissiveEdit = addRealEdit("Emissive Strength");
  lightData_.emissiveEdit->setRange(0.0, 1.0);

  //---

  lightData_.shininessEdit = addRealEdit("Shininess");
  lightData_.shininessEdit->setRange(0.0, 100.0);

  //---

  controlFrame  = new QGroupBox("Lights");
  controlLayout = new QGridLayout(controlFrame);

  layout->addWidget(controlFrame);

  lightRow = 0;

  //---

  lightData_.list = new QListWidget;

  lightData_.list->setSelectionMode(QListWidget::SingleSelection);

  controlLayout->addWidget(lightData_.list, lightRow, 0, 1, 2);

  ++lightRow;

  //--

  lightData_.typeCombo = addCombo("Type",
    QStringList() << "Directional" << "Point" << "Spot");

  lightData_.enabledCheck = addCheck("Enabled");

  lightData_.colorEdit = addColorEdit("Color"); // diffuse

  //---

  lightData_.posEdit = addPoint3DEdit("Position");

  //---

  lightData_.dirEdit = addPoint3DEdit("Direction");

  //---

  lightData_.cutoffEdit = addRealEdit("Cut Off Angle");

  //---

  lightData_.radiusEdit = addRealEdit("Point Radius");

  //---

  //layout->setRowStretch(lightRow, 1);
  layout->addStretch(1);

  //---

  auto *resetButton = new QPushButton("Reset");

  auto *buttonFrame  = new QFrame(this);
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
addObjectsFrame()
{
  auto *frame  = new QFrame;
  auto *layout = new QVBoxLayout(frame);

  auto *controlFrame  = new QFrame;
  auto *controlLayout = new QVBoxLayout(controlFrame);

  layout->addWidget(controlFrame);

  objectsData_.list = new QListWidget;

  objectsData_.list->setSelectionMode(QListWidget::SingleSelection);

  connect(objectsData_.list, &QListWidget::currentItemChanged,
          this, &Control3D::objectSelectedSlot);

  controlLayout->addWidget(objectsData_.list);

  objectsData_.tree = new CQPropertyViewTree(this);

  layout->addWidget(objectsData_.tree);

  //---

  return frame;
}

QFrame *
Control3D::
addOverviewFrame()
{
  auto *frame  = new QFrame;
  auto *layout = new QGridLayout(frame);

  //---

  int row = 0;

  auto addLabelEdit = [&](const QString &label, QWidget *w) {
    layout->addWidget(new QLabel(label), row, 0);
    layout->addWidget(w, row, 1);
    ++row;
  };

  auto addCheck = [&](const QString &label, const char *slotName) {
    auto *checkBox = new QCheckBox;
    connect(checkBox, SIGNAL(stateChanged(int)), this, slotName);
    addLabelEdit(label, checkBox);
    return checkBox;
  };

  auto addRealEdit = [&](const QString &label, const char *slotName) {
    auto *edit = new CQRealSpin;
    connect(edit, SIGNAL(realValueChanged(double)), this, slotName);
    addLabelEdit(label, edit);
    return edit;
  };

  auto addColorEdit = [&](const QString &label, const char *slotName) {
    auto *edit = new CQColorEdit;
    connect(edit, SIGNAL(colorChanged(const QColor &)), this, slotName);
    addLabelEdit(label, edit);
    return edit;
  };

  //---

  overviewData_.wireFrameCheck = addCheck("Wireframe"  , SLOT(overviewWireframeSlot(int)));
  overviewData_.solidCheck     = addCheck("Solid"      , SLOT(overviewSolidSlot(int)));
  overviewData_.zclipCheck     = addCheck("Z Clip"     , SLOT(overviewZClipSlot(int)));
  overviewData_.cameraCheck    = addCheck("Show Camera", SLOT(overviewShowCameraSlot(int)));
  overviewData_.lightCheck     = addCheck("Show Light" , SLOT(overviewShowLightSlot(int)));
  overviewData_.basisCheck     = addCheck("Show Basis" , SLOT(overviewShowBasisSlot(int)));

  overviewData_.bgColor =
    addColorEdit("Background"    , SLOT(overviewBgColorSlot(const QColor &)));
  overviewData_.strokeColor =
    addColorEdit("Stroke Color"  , SLOT(overviewStrokeColorSlot(const QColor &)));
  overviewData_.strokeAlpha =
    addRealEdit ("Stroke Alpha"  , SLOT(overviewStrokeAlphaSlot(double)));
  overviewData_.fillColor =
    addColorEdit("Fill Color"    , SLOT(overviewFillColorSlot(const QColor &)));
  overviewData_.fillAlpha =
    addRealEdit ("Fill Alpha"    , SLOT(overviewFillAlphaSlot(double)));
  overviewData_.selectedColor =
    addColorEdit("Selected Color", SLOT(overviewSelectedColorSlot(const QColor &)));
  overviewData_.pointSize =
    addRealEdit ("Point Size"    , SLOT(overviewPointSizeSlot(double)));

  //---

  layout->setRowStretch(row, 1);

  //---

  return frame;
}

void
Control3D::
toggleShown()
{
  auto *app = canvas_->app();

  auto geom = app->geometry();

  int w = this->sizeHint().width();

  QRect geom1;

  if (! shown_) {
    geom1 = QRect(geom.x(), geom.y(), geom.width() + w + 6, geom.height());

    this->updateWidgets();
    this->show();
  }
  else {
    geom1 = QRect(geom.x(), geom.y(), geom.width() - w - 6, geom.height());

    this->hide();
  }

  app->setGeometry(geom1);

  shown_ = ! shown_;

  if (shown_)
    this->setFixedWidth(w);
  else {
    this->setMinimumWidth(0);
    this->setMaximumWidth(QWIDGETSIZE_MAX);
  }
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
  needsUpdate_    = true;
  objectsChanged_ = true;

  uiSlot();
}

void
Control3D::
lightAddedSlot()
{
  needsUpdate_   = true;
  lightsChanged_ = true;

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
  updateObjects();
  updateOverview();
}

void
Control3D::
updateControl()
{
  disconnect(controlData_.depthTestCheck, &QCheckBox::stateChanged,
             this, &Control3D::depthTestSlot);
  disconnect(controlData_.cullFaceCheck , &QCheckBox::stateChanged,
             this, &Control3D::cullFaceSlot);
  disconnect(controlData_.frontFaceCheck, &QCheckBox::stateChanged,
             this, &Control3D::frontFaceSlot);
  disconnect(controlData_.bgColorEdit   , &CQColorEdit::colorChanged,
             this, &Control3D::bgColorSlot);

  controlData_.depthTestCheck->setChecked(canvas_->isDepthTest());
  controlData_.cullFaceCheck ->setChecked(canvas_->isCullFace());
  controlData_.frontFaceCheck->setChecked(canvas_->isFrontFace());
  controlData_.bgColorEdit   ->setColor(canvas_->bgColor());

  connect(controlData_.depthTestCheck, &QCheckBox::stateChanged,
          this, &Control3D::depthTestSlot);
  connect(controlData_.cullFaceCheck , &QCheckBox::stateChanged,
          this, &Control3D::cullFaceSlot);
  connect(controlData_.frontFaceCheck, &QCheckBox::stateChanged,
          this, &Control3D::frontFaceSlot);
  connect(controlData_.bgColorEdit   , &CQColorEdit::colorChanged,
          this, &Control3D::bgColorSlot);
}

void
Control3D::
updateCamera()
{
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

    cameraData_.originEdit  ->setValue(vectorToPoint(camera->origin()));
    cameraData_.posEdit     ->setValue(vectorToPoint(camera->position()));
    cameraData_.distanceEdit->setValue(camera->distance());
  }

  //---

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

void
Control3D::
updateLights()
{
  connectLights(false);

  //---

  lightData_.ambientColorEdit   ->setColor(Util::RGBAToQColor(canvas_->ambientColor()));
  lightData_.ambientStrengthEdit->setValue(canvas_->ambientStrength());
  lightData_.diffuseEdit        ->setValue(canvas_->diffuseStrength());
  lightData_.specularColorEdit  ->setColor(Util::RGBAToQColor(canvas_->specularColor()));
  lightData_.specularEdit       ->setValue(canvas_->specularStrength());
  lightData_.emissiveColorEdit  ->setColor(Util::RGBAToQColor(canvas_->emissiveColor()));
  lightData_.emissiveEdit       ->setValue(canvas_->emissiveStrength());
  lightData_.shininessEdit      ->setValue(canvas_->shininess());

  //---

  auto *currentLight = canvas_->currentLight();

  lightData_.typeCombo->setCurrentIndex(int(currentLight->getType()));

  lightData_.enabledCheck->setChecked(currentLight->getEnabled());
  lightData_.colorEdit   ->setColor(Util::RGBAToQColor(currentLight->getDiffuse()));
  lightData_.posEdit     ->setValue(currentLight->getPosition());

  if (currentLight->getType() == Light3D::Type::SPOT)
    lightData_.dirEdit->setValue(vectorToPoint(currentLight->getSpotDirection()));
  else
    lightData_.dirEdit->setValue(vectorToPoint(currentLight->getDirection()));

  lightData_.cutoffEdit->setEnabled(currentLight->getType() == Light3D::Type::SPOT);
  lightData_.cutoffEdit->setValue(currentLight->getSpotCutOffAngle());

  lightData_.radiusEdit->setEnabled(currentLight->getType() == Light3D::Type::POINT);
  lightData_.radiusEdit->setValue(currentLight->getPointRadius());

  if (lightsChanged_) {
    lightsChanged_ = false;

    lightData_.list->clear();

    QListWidgetItem *currentItem = nullptr;

    for (auto *light : canvas_->lights()) {
      auto lightName = QString("light.%1").arg(light->id());

      auto *item = new QListWidgetItem(lightName);

      lightData_.list->addItem(item);

      item->setData(Qt::UserRole, light->id());

      if (light == currentLight)
        currentItem = item;
    }

    if (currentItem)
      lightData_.list->setCurrentItem(currentItem, QItemSelectionModel::Select);
  }

  //---

  connectLights(true);
}

void
Control3D::
connectLights(bool b)
{
  if (b) {
    connect(lightData_.ambientColorEdit, &CQColorEdit::colorChanged,
            this, &Control3D::ambientColorSlot);
    connect(lightData_.ambientStrengthEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::ambientStrengthSlot);
    connect(lightData_.diffuseEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::diffuseSlot);
    connect(lightData_.specularColorEdit, &CQColorEdit::colorChanged,
            this, &Control3D::specularColorSlot);
    connect(lightData_.specularEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::specularSlot);
    connect(lightData_.emissiveColorEdit, &CQColorEdit::colorChanged,
            this, &Control3D::emissiveColorSlot);
    connect(lightData_.emissiveEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::emissiveSlot);
    connect(lightData_.shininessEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::shininessSlot);

    connect(lightData_.enabledCheck , &QCheckBox::stateChanged,
            this, &Control3D::lightCheckSlot);
    connect(lightData_.colorEdit , &CQColorEdit::colorChanged,
            this, &Control3D::lightColorSlot);
    connect(lightData_.posEdit   , &CQPoint3DEdit::editingFinished,
            this, &Control3D::lightPosSlot);
    connect(lightData_.dirEdit   , &CQPoint3DEdit::editingFinished,
            this, &Control3D::lightDirSlot);
    connect(lightData_.cutoffEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::lightCutoffSlot);
    connect(lightData_.radiusEdit, &CQRealSpin::realValueChanged,
            this, &Control3D::lightRadiusSlot);
    connect(lightData_.list, &QListWidget::currentItemChanged,
            this, &Control3D::lightSelectedSlot);
  }
  else {
    disconnect(lightData_.ambientColorEdit, &CQColorEdit::colorChanged,
               this, &Control3D::ambientColorSlot);
    disconnect(lightData_.ambientStrengthEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::ambientStrengthSlot);
    disconnect(lightData_.diffuseEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::diffuseSlot);
    disconnect(lightData_.specularColorEdit, &CQColorEdit::colorChanged,
               this, &Control3D::specularColorSlot);
    disconnect(lightData_.specularEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::specularSlot);
    disconnect(lightData_.emissiveColorEdit, &CQColorEdit::colorChanged,
               this, &Control3D::emissiveColorSlot);
    disconnect(lightData_.emissiveEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::emissiveSlot);
    disconnect(lightData_.shininessEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::shininessSlot);

    disconnect(lightData_.enabledCheck , &QCheckBox::stateChanged,
               this, &Control3D::lightCheckSlot);
    disconnect(lightData_.colorEdit , &CQColorEdit::colorChanged,
               this, &Control3D::lightColorSlot);
    disconnect(lightData_.posEdit   , &CQPoint3DEdit::editingFinished,
               this, &Control3D::lightPosSlot);
    disconnect(lightData_.dirEdit   , &CQPoint3DEdit::editingFinished,
               this, &Control3D::lightDirSlot);
    disconnect(lightData_.cutoffEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::lightCutoffSlot);
    disconnect(lightData_.radiusEdit, &CQRealSpin::realValueChanged,
               this, &Control3D::lightRadiusSlot);
    disconnect(lightData_.list, &QListWidget::currentItemChanged,
               this, &Control3D::lightSelectedSlot);
  }
}

void
Control3D::
updateObjects()
{
  if (objectsChanged_) {
    disconnect(objectsData_.list, &QListWidget::currentItemChanged,
             this, &Control3D::objectSelectedSlot);

    //---

    objectsChanged_ = false;

    QListWidgetItem *currentItem = nullptr;

    objectsData_.list->clear();

    for (auto *object : canvas_->objects()) {
      auto objectName = QString("%1.%2").arg(object->typeName()).arg(object->ind());

      auto *item = new QListWidgetItem(objectName);

      objectsData_.list->addItem(item);

      item->setData(Qt::UserRole, int(object->ind()));

      if (! currentItem)
        currentItem = item;
    }

    if (currentItem)
      objectsData_.list->setCurrentItem(currentItem, QItemSelectionModel::Select);

    //---

    auto items = objectsData_.list->selectedItems();

    if (items.size() > 0)
      currentItem = items[0];

    if (currentItem)
      objectSelectedSlot(currentItem, nullptr);

    //---

    connect(objectsData_.list, &QListWidget::currentItemChanged,
            this, &Control3D::objectSelectedSlot);
  }
}

void
Control3D::
updateOverview()
{
  auto *overview = canvas_->app()->overview3D();
  if (! overview) return;

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

  overviewData_.wireFrameCheck->setChecked(overview->isWireframe());
  overviewData_.solidCheck    ->setChecked(overview->isSolid());
  overviewData_.zclipCheck    ->setChecked(overview->isZClip());
  overviewData_.cameraCheck   ->setChecked(overview->isCameraVisible());
  overviewData_.lightCheck    ->setChecked(overview->isLightsVisible());
  overviewData_.basisCheck    ->setChecked(overview->isBasisVisible());

  overviewData_.bgColor      ->setColor  (overview->bgColor());
  overviewData_.strokeColor  ->setColor  (overview->strokeColor());
  overviewData_.strokeAlpha  ->setValue  (overview->strokeAlpha());
  overviewData_.fillColor    ->setColor  (overview->fillColor());
  overviewData_.fillAlpha    ->setValue  (overview->fillAlpha());
  overviewData_.selectedColor->setColor  (overview->selectedColor());
  overviewData_.pointSize    ->setValue  (overview->pointSize());

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
ambientColorSlot(const QColor &c)
{
  canvas_->setAmbientColor(Util::QColorToRGBA(c));
  canvas_->update();
}

void
Control3D::
ambientStrengthSlot()
{
  auto a = lightData_.ambientStrengthEdit->value();

  canvas_->setAmbientStrength(a);
  canvas_->update();
}

void
Control3D::
diffuseSlot()
{
  auto a = lightData_.diffuseEdit->value();

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
  auto a = lightData_.specularEdit->value();

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
  auto a = lightData_.emissiveEdit->value();

  canvas_->setEmissiveStrength(a);
  canvas_->update();
}

void
Control3D::
shininessSlot()
{
  auto a = lightData_.shininessEdit->value();

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
lightPosSlot()
{
  auto *light = canvas_->currentLight();

  auto p = lightData_.posEdit->getValue();
  light->setPosition(CPoint3D(p.x, p.y, p.z));
  canvas_->update();
}

void
Control3D::
lightDirSlot()
{
  auto *light = canvas_->currentLight();

  auto p = lightData_.dirEdit->getValue();
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
objectSelectedSlot(QListWidgetItem *item, QListWidgetItem *)
{
  int ind = item->data(Qt::UserRole).toInt();

  auto *indObj = canvas_->objectFromInd(ind);

#if 0
  for (auto *obj : canvas_->objects())
    obj->setSelected(obj == indObj);
#endif

  objectsData_.tree->clear();

  if (indObj) {
    auto properties = CQUtil::getPropertyList(indObj);

    for (auto &prop : properties) {
      objectsData_.tree->addProperty("", indObj, prop);
    }
  }
}

void
Control3D::
overviewWireframeSlot(int state)
{
  auto *overview = canvas_->app()->overview3D();

  overview->setWireframe(state);
}

void
Control3D::
overviewSolidSlot(int state)
{
  auto *overview = canvas_->app()->overview3D();

  overview->setSolid(state);
}

void
Control3D::
overviewZClipSlot(int state)
{
  auto *overview = canvas_->app()->overview3D();

  overview->setZClip(state);
}

void
Control3D::
overviewShowCameraSlot(int state)
{
  auto *overview = canvas_->app()->overview3D();

  overview->setCameraVisible(state);
}

void
Control3D::
overviewShowLightSlot(int state)
{
  auto *overview = canvas_->app()->overview3D();

  overview->setLightsVisible(state);
}

void
Control3D::
overviewShowBasisSlot(int state)
{
  auto *overview = canvas_->app()->overview3D();

  overview->setBasisVisible(state);
}

void
Control3D::
overviewBgColorSlot(const QColor &c)
{
  auto *overview = canvas_->app()->overview3D();

  overview->setBgColor(c);
}

void
Control3D::
overviewStrokeColorSlot(const QColor &c)
{
  auto *overview = canvas_->app()->overview3D();

  overview->setStrokeColor(c);
}

void
Control3D::
overviewStrokeAlphaSlot(double a)
{
  auto *overview = canvas_->app()->overview3D();

  overview->setStrokeAlpha(a);
}

void
Control3D::
overviewFillColorSlot(const QColor &c)
{
  auto *overview = canvas_->app()->overview3D();

  overview->setFillColor(c);
}

void
Control3D::
overviewFillAlphaSlot(double a)
{
  auto *overview = canvas_->app()->overview3D();

  overview->setFillAlpha(a);
}

void
Control3D::
overviewSelectedColorSlot(const QColor &c)
{
  auto *overview = canvas_->app()->overview3D();

  overview->setSelectedColor(c);
}

void
Control3D::
overviewPointSizeSlot(double s)
{
  auto *overview = canvas_->app()->overview3D();

  overview->setPointSize(s);
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
