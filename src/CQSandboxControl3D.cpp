#include <CQSandboxControl3D.h>
#include <CQSandboxCanvas3D.h>
#include <CGLCamera.h>

#include <CQColorEdit.h>
#include <CQPoint3DEdit.h>
#include <CQRealSpin.h>

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

QColor vectorToColor(const CGLVector3D &v) {
  QColor c;
  c.setRgbF(v.getX(), v.getY(), v.getZ());
  return c;
}

CGLVector3D colorToVector(const QColor &c) {
  return CGLVector3D(c.redF(), c.greenF(), c.blueF());
}

CanvasControl3D::
CanvasControl3D(CQSandbox::Canvas3D *canvas) :
 QFrame(nullptr), canvas_(canvas)
{
  auto *layout = new QVBoxLayout(this);

  //---

  auto *controlFrame  = new QFrame(this);
  auto *controlLayout = new QGridLayout(controlFrame);

  layout->addWidget(controlFrame);

  //---

  int controlRow = 0;

  auto addLabelEdit = [&](const QString &label, QWidget *w) {
    controlLayout->addWidget(new QLabel(label), controlRow, 0);
    controlLayout->addWidget(w, controlRow, 1);
    ++controlRow;
  };

  //---

  depthTestCheck_ = new QCheckBox;

  connect(depthTestCheck_, &QCheckBox::stateChanged, this, &CanvasControl3D::depthTestSlot);

  addLabelEdit("Depth Test", depthTestCheck_);

  //---

  cullFaceCheck_ = new QCheckBox;

  connect(cullFaceCheck_, &QCheckBox::stateChanged, this, &CanvasControl3D::cullFaceSlot);

  addLabelEdit("Cull Face", cullFaceCheck_);

  //---

  frontFaceCheck_ = new QCheckBox;

  connect(frontFaceCheck_, &QCheckBox::stateChanged, this, &CanvasControl3D::frontFaceSlot);

  addLabelEdit("Front Face", frontFaceCheck_);

  //---

  bgColorEdit_ = new CQColorEdit;

  connect(bgColorEdit_, &CQColorEdit::colorChanged, this, &CanvasControl3D::bgColorSlot);

  addLabelEdit("Bg Color", bgColorEdit_);

  //---

  nearEdit_ = new CQRealSpin;

  connect(nearEdit_, &CQRealSpin::realValueChanged, this, &CanvasControl3D::nearSlot);

  addLabelEdit("Near", nearEdit_);

  farEdit_ = new CQRealSpin;

  connect(farEdit_, &CQRealSpin::realValueChanged, this, &CanvasControl3D::farSlot);

  addLabelEdit("Far", farEdit_);

  //---

  ambientEdit_ = new CQRealSpin;

  ambientEdit_->setRange(0.0, 1.0);

//connect(ambientEdit_, &CQRealSpin::realValueChanged, this, &CanvasControl3D::ambientSlot);

  addLabelEdit("Ambient", ambientEdit_);

  //---

  diffuseEdit_ = new CQRealSpin;

  diffuseEdit_->setRange(0.0, 1.0);

//connect(diffuseEdit_, &CQRealSpin::realValueChanged, this, &CanvasControl3D::diffuseSlot);

  addLabelEdit("Diffuse", diffuseEdit_);

  //---

  specularEdit_ = new CQRealSpin;

  specularEdit_->setRange(0.0, 1.0);

//connect(specularEdit_, &CQRealSpin::realValueChanged, this, &CanvasControl3D::specularSlot);

  addLabelEdit("Specular", specularEdit_);

  //---

  shininessEdit_ = new CQRealSpin;

  shininessEdit_->setRange(0.0, 100.0);

//connect(shininessEdit_, &CQRealSpin::realValueChanged, this, &CanvasControl3D::shininessSlot);

  addLabelEdit("Shininess", shininessEdit_);

  //---

  controlLayout->setRowStretch(controlRow, 1);

  //---

  auto *lightGroup  = new QGroupBox("Lights");
  auto *lightLayout = new QHBoxLayout(lightGroup);

  layout->addWidget(lightGroup);

  auto *lightControlFrame  = new QFrame;
  auto *lightControlLayout = new QGridLayout(lightControlFrame);

  lightLayout->addWidget(lightControlFrame);

  int lightRow = 0;

  auto addLightLabelEdit = [&](const QString &label, QWidget *w) {
    lightControlLayout->addWidget(new QLabel(label), lightRow, 0);
    lightControlLayout->addWidget(w, lightRow, 1);
    ++lightRow;
  };

  lightsList_ = new QListWidget;

  lightsList_->setSelectionMode(QListWidget::SingleSelection);

  connect(lightsList_, &QListWidget::currentItemChanged,
          this, &CanvasControl3D::lightSelectedSlot);

  lightLayout->addWidget(lightsList_);

  //--

  lightTypeCombo_ = new QComboBox;

  lightTypeCombo_->addItem("Directional");
  lightTypeCombo_->addItem("Point");
  lightTypeCombo_->addItem("Spot");

  addLightLabelEdit("Type", lightTypeCombo_);

  lightCheck_ = new QCheckBox;

  connect(lightCheck_, &QCheckBox::stateChanged, this, &CanvasControl3D::lightCheckSlot);

  addLightLabelEdit("Enabled", lightCheck_);

  lightColorEdit_ = new CQColorEdit;

  connect(lightColorEdit_, &CQColorEdit::colorChanged, this, &CanvasControl3D::lightColorSlot);

  addLightLabelEdit("Color", lightColorEdit_);

  //---

  lightPosEdit_ = new CQPoint3DEdit;

  connect(lightPosEdit_, &CQPoint3DEdit::editingFinished, this, &CanvasControl3D::lightPosSlot);

  addLightLabelEdit("Position", lightPosEdit_);

  //---

  lightDirEdit_ = new CQPoint3DEdit;

  connect(lightDirEdit_, &CQPoint3DEdit::editingFinished, this, &CanvasControl3D::lightDirSlot);

  addLightLabelEdit("Direction", lightDirEdit_);

  //---

  lightCutoffEdit_ = new CQRealSpin;

  connect(lightCutoffEdit_, &CQRealSpin::realValueChanged, this, &CanvasControl3D::lightCutoffSlot);

  addLightLabelEdit("Cut Off", lightCutoffEdit_);

  //---

  lightRadiusEdit_ = new CQRealSpin;

  connect(lightRadiusEdit_, &CQRealSpin::realValueChanged, this, &CanvasControl3D::lightRadiusSlot);

  addLightLabelEdit("Radius", lightRadiusEdit_);

  //---

  auto *updateButton = new QPushButton("Update");
  auto *closeButton  = new QPushButton("Close");

  auto *buttonFrame  = new QFrame(this);
  auto *buttonLayout = new QHBoxLayout(buttonFrame);

  layout->addWidget(buttonFrame);

  buttonLayout->addStretch(1);
  buttonLayout->addWidget(updateButton);
  buttonLayout->addWidget(closeButton);

  connect(updateButton, &QPushButton::clicked, this, &CanvasControl3D::updateSlot);
  connect(closeButton, &QPushButton::clicked, this, &CanvasControl3D::closeSlot);

  //---

  update();
}

void
CanvasControl3D::
updateSlot()
{
  update();
}

void
CanvasControl3D::
closeSlot()
{
  close();
}

void
CanvasControl3D::
update()
{
  disconnect(depthTestCheck_, &QCheckBox::stateChanged, this, &CanvasControl3D::depthTestSlot);
  disconnect(cullFaceCheck_ , &QCheckBox::stateChanged, this, &CanvasControl3D::cullFaceSlot);
  disconnect(frontFaceCheck_, &QCheckBox::stateChanged, this, &CanvasControl3D::frontFaceSlot);
  disconnect(bgColorEdit_   , &CQColorEdit::colorChanged, this, &CanvasControl3D::bgColorSlot);

  disconnect(nearEdit_, &CQRealSpin::realValueChanged, this, &CanvasControl3D::nearSlot);
  disconnect(farEdit_ , &CQRealSpin::realValueChanged, this, &CanvasControl3D::farSlot);

//disconnect(ambientEdit_  , &CQRealSpin::realValueChanged, this, &CanvasControl3D::ambientSlot);
//disconnect(diffuseEdit_  , &CQRealSpin::realValueChanged, this, &CanvasControl3D::diffuseSlot);
//disconnect(specularEdit_ , &CQRealSpin::realValueChanged, this, &CanvasControl3D::specularSlot);
//disconnect(shininessEdit_, &CQRealSpin::realValueChanged, this, &CanvasControl3D::shininessSlot);

  //---

  depthTestCheck_->setChecked(canvas_->isDepthTest());
  cullFaceCheck_ ->setChecked(canvas_->isCullFace());
  frontFaceCheck_->setChecked(canvas_->isFrontFace());
  bgColorEdit_   ->setColor(canvas_->bgColor());

  //---

  nearEdit_->setValue(canvas_->camera()->near());
  farEdit_ ->setValue(canvas_->camera()->far());

  ambientEdit_  ->setValue(canvas_->ambient());
  diffuseEdit_  ->setValue(canvas_->diffuse());
  specularEdit_ ->setValue(canvas_->specular());
  shininessEdit_->setValue(canvas_->shininess());

  //---

  connect(depthTestCheck_, &QCheckBox::stateChanged, this, &CanvasControl3D::depthTestSlot);
  connect(cullFaceCheck_ , &QCheckBox::stateChanged, this, &CanvasControl3D::cullFaceSlot);
  connect(frontFaceCheck_, &QCheckBox::stateChanged, this, &CanvasControl3D::frontFaceSlot);
  connect(bgColorEdit_   , &CQColorEdit::colorChanged, this, &CanvasControl3D::bgColorSlot);

  connect(nearEdit_, &CQRealSpin::realValueChanged, this, &CanvasControl3D::nearSlot);
  connect(farEdit_ , &CQRealSpin::realValueChanged, this, &CanvasControl3D::farSlot);

//connect(ambientEdit_  , &CQRealSpin::realValueChanged, this, &CanvasControl3D::ambientSlot);
//connect(diffuseEdit_  , &CQRealSpin::realValueChanged, this, &CanvasControl3D::diffuseSlot);
//connect(specularEdit_ , &CQRealSpin::realValueChanged, this, &CanvasControl3D::specularSlot);
//connect(shininessEdit_, &CQRealSpin::realValueChanged, this, &CanvasControl3D::shininessSlot);

  //---

  updateLights();
}

void
CanvasControl3D::
updateLights()
{
  disconnect(lightCheck_     , &QCheckBox::stateChanged, this, &CanvasControl3D::lightCheckSlot);
  disconnect(lightColorEdit_ , &CQColorEdit::colorChanged, this, &CanvasControl3D::lightColorSlot);
  disconnect(lightPosEdit_   , &CQPoint3DEdit::editingFinished,
             this, &CanvasControl3D::lightPosSlot);
  disconnect(lightDirEdit_   , &CQPoint3DEdit::editingFinished,
             this, &CanvasControl3D::lightDirSlot);
  disconnect(lightCutoffEdit_, &CQRealSpin::realValueChanged,
             this, &CanvasControl3D::lightCutoffSlot);
  disconnect(lightRadiusEdit_, &CQRealSpin::realValueChanged,
             this, &CanvasControl3D::lightRadiusSlot);

  disconnect(lightsList_, &QListWidget::currentItemChanged,
             this, &CanvasControl3D::lightSelectedSlot);

  //---

  auto *currentLight = canvas_->currentLight();

  lightTypeCombo_->setCurrentIndex(int(currentLight->type()));
  lightCheck_    ->setChecked(currentLight->isEnabled());
  lightColorEdit_->setColor(vectorToColor(currentLight->color()));
  lightPosEdit_  ->setValue(vectorToPoint(currentLight->position()));
  lightDirEdit_  ->setValue(vectorToPoint(currentLight->direction()));

  lightCutoffEdit_->setEnabled(currentLight->type() == Light3D::Type::SPOT);
  lightCutoffEdit_->setValue(currentLight->cutoff());

  lightRadiusEdit_->setEnabled(currentLight->type() == Light3D::Type::POINT);
  lightRadiusEdit_->setValue(currentLight->radius());

  lightsList_->clear();

  for (auto *light : canvas_->lights()) {
    auto lightName = QString("light%1").arg(light->id());

    auto *item = new QListWidgetItem(lightName);

    lightsList_->addItem(item);

    item->setData(Qt::UserRole, light->id());

    if (light == currentLight)
      item->setSelected(true);
  }

  //---

  connect(lightCheck_     , &QCheckBox::stateChanged, this, &CanvasControl3D::lightCheckSlot);
  connect(lightColorEdit_ , &CQColorEdit::colorChanged, this, &CanvasControl3D::lightColorSlot);
  connect(lightPosEdit_   , &CQPoint3DEdit::editingFinished,
          this, &CanvasControl3D::lightPosSlot);
  connect(lightDirEdit_   , &CQPoint3DEdit::editingFinished,
          this, &CanvasControl3D::lightDirSlot);
  connect(lightCutoffEdit_, &CQRealSpin::realValueChanged,
          this, &CanvasControl3D::lightCutoffSlot);
  connect(lightRadiusEdit_, &CQRealSpin::realValueChanged,
          this, &CanvasControl3D::lightRadiusSlot);

  connect(lightsList_, &QListWidget::currentItemChanged,
          this, &CanvasControl3D::lightSelectedSlot);
}

void
CanvasControl3D::
depthTestSlot(int b)
{
  canvas_->setDepthTest(b);
  canvas_->update();
}

void
CanvasControl3D::
cullFaceSlot(int b)
{
  canvas_->setCullFace(b);
  canvas_->update();
}

void
CanvasControl3D::
frontFaceSlot(int b)
{
  canvas_->setFrontFace(b);
  canvas_->update();
}

void
CanvasControl3D::
bgColorSlot(const QColor &c)
{
  canvas_->setBgColor(c);
  canvas_->update();
}

void
CanvasControl3D::
nearSlot(double r)
{
  canvas_->camera()->setNear(r);
  canvas_->update();
}

void
CanvasControl3D::
farSlot(double r)
{
  canvas_->camera()->setFar(r);
  canvas_->update();
}

void
CanvasControl3D::
lightSelectedSlot(QListWidgetItem *item, QListWidgetItem *)
{
  int id = item->data(Qt::UserRole).toInt();

  canvas_->setLightNum(id);

  updateLights();
}

void
CanvasControl3D::
lightCheckSlot(int b)
{
  auto *light = canvas_->currentLight();

  light->setEnabled(b);
  canvas_->update();
}

void
CanvasControl3D::
lightColorSlot(const QColor &c)
{
  auto *light = canvas_->currentLight();

  light->setColor(colorToVector(c));
  canvas_->update();
}

void
CanvasControl3D::
lightPosSlot()
{
  auto *light = canvas_->currentLight();

  auto p = lightPosEdit_->getValue();
  light->setPosition(CGLVector3D(p.x, p.y, p.z));
  canvas_->update();
}

void
CanvasControl3D::
lightDirSlot()
{
  auto *light = canvas_->currentLight();

  auto p = lightDirEdit_->getValue();
  light->setDirection(CGLVector3D(p.x, p.y, p.z));
  canvas_->update();
}

void
CanvasControl3D::
lightCutoffSlot(double r)
{
  auto *light = canvas_->currentLight();

  light->setCutoff(r);
  canvas_->update();
}

void
CanvasControl3D::
lightRadiusSlot(double r)
{
  auto *light = canvas_->currentLight();

  light->setRadius(r);
  canvas_->update();
}

}
