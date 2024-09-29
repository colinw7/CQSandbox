#include <CQSandboxControl3D.h>
#include <CQSandboxCanvas3D.h>
#include <CGLCamera.h>

#include <CQColorEdit.h>
#include <CQPoint3DEdit.h>
#include <CQRealSpin.h>

#include <QGroupBox>
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

  int r = 0;

  auto addLabelEdit = [&](const QString &label, QWidget *w) {
    controlLayout->addWidget(new QLabel(label), r, 0);
    controlLayout->addWidget(w, r, 1);
    ++r;
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

  controlLayout->setRowStretch(r, 1);

  //---

  auto *lightGroup  = new QGroupBox("Light");
  auto *lightLayout = new QGridLayout(lightGroup);

  layout->addWidget(lightGroup);

  int lightRow = 0;

  auto addLightLabelEdit = [&](const QString &label, QWidget *w) {
    lightLayout->addWidget(new QLabel(label), lightRow, 0);
    lightLayout->addWidget(w, lightRow, 1);
    ++lightRow;
  };

  //--

  auto *light = canvas_->currentLight();

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

  auto lightPos = light->position();

//connect(lightPosEdit_, &CQColorEdit::editingFinished, this, &CanvasControl3D::lightPosSlot);

  addLightLabelEdit("Position", lightPosEdit_);

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

  disconnect(lightColorEdit_, &CQColorEdit::colorChanged, this, &CanvasControl3D::lightColorSlot);
  disconnect(lightCheck_    , &QCheckBox::stateChanged, this, &CanvasControl3D::lightCheckSlot);
//disconnect(lightPosEdit_  , &CQColorEdit::editingFinished, this, &CanvasControl3D::lightPosSlot);
//disconnect(ambientEdit_   , &CQRealSpin::realValueChanged, this, &CanvasControl3D::ambientSlot);
//disconnect(diffuseEdit_   , &CQRealSpin::realValueChanged, this, &CanvasControl3D::diffuseSlot);
//disconnect(specularEdit_  , &CQRealSpin::realValueChanged, this, &CanvasControl3D::specularSlot);
//disconnect(shininessEdit_ , &CQRealSpin::realValueChanged, this, &CanvasControl3D::shininessSlot);


  //---

  depthTestCheck_->setChecked(canvas_->isDepthTest());
  cullFaceCheck_ ->setChecked(canvas_->isCullFace());
  frontFaceCheck_->setChecked(canvas_->isFrontFace());
  bgColorEdit_   ->setColor(canvas_->bgColor());

  //---

  nearEdit_->setValue(canvas_->camera()->near());
  farEdit_ ->setValue(canvas_->camera()->far());

  //---

  auto *light = canvas_->currentLight();

  auto lightPos = light->position();

  lightTypeCombo_->setCurrentIndex(int(light->type()));
  lightCheck_    ->setChecked(light->isEnabled());
  lightColorEdit_->setColor(vectorToColor(light->color()));
  lightPosEdit_  ->setValue(vectorToPoint(lightPos));
  ambientEdit_   ->setValue(canvas_->ambient());
  diffuseEdit_   ->setValue(canvas_->diffuse());
  specularEdit_  ->setValue(canvas_->specular());
  shininessEdit_ ->setValue(canvas_->shininess());

  //---

  connect(depthTestCheck_, &QCheckBox::stateChanged, this, &CanvasControl3D::depthTestSlot);
  connect(cullFaceCheck_ , &QCheckBox::stateChanged, this, &CanvasControl3D::cullFaceSlot);
  connect(frontFaceCheck_, &QCheckBox::stateChanged, this, &CanvasControl3D::frontFaceSlot);
  connect(bgColorEdit_   , &CQColorEdit::colorChanged, this, &CanvasControl3D::bgColorSlot);

  connect(nearEdit_, &CQRealSpin::realValueChanged, this, &CanvasControl3D::nearSlot);
  connect(farEdit_ , &CQRealSpin::realValueChanged, this, &CanvasControl3D::farSlot);

  connect(lightColorEdit_, &CQColorEdit::colorChanged, this, &CanvasControl3D::lightColorSlot);
  connect(lightCheck_    , &QCheckBox::stateChanged, this, &CanvasControl3D::lightCheckSlot);
//connect(lightPosEdit_  , &CQColorEdit::editingFinished, this, &CanvasControl3D::lightPosSlot);
//connect(ambientEdit_   , &CQRealSpin::realValueChanged, this, &CanvasControl3D::ambientSlot);
//connect(diffuseEdit_   , &CQRealSpin::realValueChanged, this, &CanvasControl3D::diffuseSlot);
//connect(specularEdit_  , &CQRealSpin::realValueChanged, this, &CanvasControl3D::specularSlot);
//connect(shininessEdit_ , &CQRealSpin::realValueChanged, this, &CanvasControl3D::shininessSlot);
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

}
