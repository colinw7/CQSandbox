#include <CQSandboxControl3D.h>
#include <CQSandboxCanvas3D.h>
#include <CGLCamera.h>

#include <CQColorEdit.h>
#include <CQPoint3DEdit.h>
#include <CQRealSpin.h>
#include <CQPropertyViewTree.h>
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

Control3D::
Control3D(CQSandbox::Canvas3D *canvas) :
 QFrame(nullptr), canvas_(canvas)
{
  auto *layout = new QVBoxLayout(this);

  //---

  tab_ = new QTabWidget;

  layout->addWidget(tab_);

  //---

  auto *controlFrame  = new QFrame(this);
  auto *controlLayout = new QGridLayout(controlFrame);

  tab_->addTab(controlFrame, "Control");

  //---

  int controlRow = 0;

  auto addLabelEdit = [&](const QString &label, QWidget *w) {
    controlLayout->addWidget(new QLabel(label), controlRow, 0);
    controlLayout->addWidget(w, controlRow, 1);
    ++controlRow;
  };

  //---

  depthTestCheck_ = new QCheckBox;
  connect(depthTestCheck_, &QCheckBox::stateChanged, this, &Control3D::depthTestSlot);
  addLabelEdit("Depth Test", depthTestCheck_);

  //---

  cullFaceCheck_ = new QCheckBox;
  connect(cullFaceCheck_, &QCheckBox::stateChanged, this, &Control3D::cullFaceSlot);
  addLabelEdit("Cull Face", cullFaceCheck_);

  //---

  frontFaceCheck_ = new QCheckBox;
  connect(frontFaceCheck_, &QCheckBox::stateChanged, this, &Control3D::frontFaceSlot);
  addLabelEdit("Front Face", frontFaceCheck_);

  //---

  bgColorEdit_ = new CQColorEdit;
  connect(bgColorEdit_, &CQColorEdit::colorChanged, this, &Control3D::bgColorSlot);
  addLabelEdit("Bg Color", bgColorEdit_);

  //---

  ambientEdit_ = new CQRealSpin;
  ambientEdit_->setRange(0.0, 1.0);
//connect(ambientEdit_, &CQRealSpin::realValueChanged, this, &Control3D::ambientSlot);
  addLabelEdit("Ambient", ambientEdit_);

  //---

  diffuseEdit_ = new CQRealSpin;
  diffuseEdit_->setRange(0.0, 1.0);
//connect(diffuseEdit_, &CQRealSpin::realValueChanged, this, &Control3D::diffuseSlot);
  addLabelEdit("Diffuse", diffuseEdit_);

  //---

  specularEdit_ = new CQRealSpin;
  specularEdit_->setRange(0.0, 1.0);
//connect(specularEdit_, &CQRealSpin::realValueChanged, this, &Control3D::specularSlot);
  addLabelEdit("Specular", specularEdit_);

  //---

  shininessEdit_ = new CQRealSpin;
  shininessEdit_->setRange(0.0, 100.0);
//connect(shininessEdit_, &CQRealSpin::realValueChanged, this, &Control3D::shininessSlot);
  addLabelEdit("Shininess", shininessEdit_);

  controlLayout->setRowStretch(controlRow, 1);

  //---

  auto *cameraFrame  = new QFrame;
  auto *cameraLayout = new QHBoxLayout(cameraFrame);

  tab_->addTab(cameraFrame, "Camera");

  auto *cameraControlFrame  = new QFrame;
  auto *cameraControlLayout = new QGridLayout(cameraControlFrame);

  cameraLayout->addWidget(cameraControlFrame);

  int cameraRow = 0;

  auto addCameraLabelEdit = [&](const QString &label, QWidget *w) {
    cameraControlLayout->addWidget(new QLabel(label), cameraRow, 0);
    cameraControlLayout->addWidget(w, cameraRow, 1);
    ++cameraRow;
  };

  //---

  cameraRotateCheck_ = new QCheckBox;
  connect(cameraRotateCheck_, &QCheckBox::stateChanged,
          this, &Control3D::cameraRotateSlot);
  addCameraLabelEdit("Rotate", cameraRotateCheck_);

  cameraZoomEdit_ = new CQRealSpin;
  connect(cameraZoomEdit_, &CQRealSpin::realValueChanged, this, &Control3D::cameraZoomSlot);
  addCameraLabelEdit("Zoom", cameraZoomEdit_);

  cameraNearEdit_ = new CQRealSpin;
  connect(cameraNearEdit_, &CQRealSpin::realValueChanged, this, &Control3D::cameraNearSlot);
  addCameraLabelEdit("Near", cameraNearEdit_);

  cameraFarEdit_ = new CQRealSpin;
  connect(cameraFarEdit_, &CQRealSpin::realValueChanged, this, &Control3D::cameraFarSlot);
  addCameraLabelEdit("Far", cameraFarEdit_);

  cameraYawEdit_ = new CQRealSpin;
  connect(cameraYawEdit_, &CQRealSpin::realValueChanged, this, &Control3D::cameraYawSlot);
  addCameraLabelEdit("Yaw", cameraYawEdit_);

  cameraPitchEdit_ = new CQRealSpin;
  connect(cameraPitchEdit_, &CQRealSpin::realValueChanged, this, &Control3D::cameraPitchSlot);
  addCameraLabelEdit("Pitch", cameraPitchEdit_);

  cameraRollEdit_ = new CQRealSpin;
  connect(cameraRollEdit_, &CQRealSpin::realValueChanged, this, &Control3D::cameraRollSlot);
  addCameraLabelEdit("Roll", cameraRollEdit_);

  cameraPosEdit_ = new CQPoint3DEdit;
  connect(cameraPosEdit_, &CQPoint3DEdit::editingFinished, this, &Control3D::cameraPosSlot);
  addCameraLabelEdit("Position", cameraPosEdit_);

  //---

  cameraControlLayout->setRowStretch(cameraRow, 1);

  //---

  auto *lightFrame  = new QFrame;
  auto *lightLayout = new QHBoxLayout(lightFrame);

  tab_->addTab(lightFrame, "Lights");

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
          this, &Control3D::lightSelectedSlot);

  lightLayout->addWidget(lightsList_);

  //--

  lightTypeCombo_ = new QComboBox;

  lightTypeCombo_->addItem("Directional");
  lightTypeCombo_->addItem("Point");
  lightTypeCombo_->addItem("Spot");

  addLightLabelEdit("Type", lightTypeCombo_);

  lightCheck_ = new QCheckBox;

  connect(lightCheck_, &QCheckBox::stateChanged, this, &Control3D::lightCheckSlot);

  addLightLabelEdit("Enabled", lightCheck_);

  lightColorEdit_ = new CQColorEdit;
  connect(lightColorEdit_, &CQColorEdit::colorChanged, this, &Control3D::lightColorSlot);
  addLightLabelEdit("Color", lightColorEdit_);

  //---

  lightPosEdit_ = new CQPoint3DEdit;
  connect(lightPosEdit_, &CQPoint3DEdit::editingFinished, this, &Control3D::lightPosSlot);
  addLightLabelEdit("Position", lightPosEdit_);

  //---

  lightDirEdit_ = new CQPoint3DEdit;
  connect(lightDirEdit_, &CQPoint3DEdit::editingFinished, this, &Control3D::lightDirSlot);
  addLightLabelEdit("Direction", lightDirEdit_);

  //---

  lightCutoffEdit_ = new CQRealSpin;
  connect(lightCutoffEdit_, &CQRealSpin::realValueChanged, this, &Control3D::lightCutoffSlot);
  addLightLabelEdit("Cut Off", lightCutoffEdit_);

  //---

  lightRadiusEdit_ = new CQRealSpin;
  connect(lightRadiusEdit_, &CQRealSpin::realValueChanged, this, &Control3D::lightRadiusSlot);
  addLightLabelEdit("Radius", lightRadiusEdit_);

  //---

  lightControlLayout->setRowStretch(lightRow, 1);

  //---

  auto *objectsFrame  = new QFrame;
  auto *objectsLayout = new QVBoxLayout(objectsFrame);

  tab_->addTab(objectsFrame, "Objects");

  auto *objectsControlFrame  = new QFrame;
  auto *objectsControlLayout = new QVBoxLayout(objectsControlFrame);

  objectsLayout->addWidget(objectsControlFrame);

  objectsList_ = new QListWidget;

  objectsList_->setSelectionMode(QListWidget::SingleSelection);

  connect(objectsList_, &QListWidget::currentItemChanged,
          this, &Control3D::objectSelectedSlot);

  objectsControlLayout->addWidget(objectsList_);

  objectTree_ = new CQPropertyViewTree(this);

  objectsLayout->addWidget(objectTree_);

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

  //---

  update();
}

void
Control3D::
updateSlot()
{
  update();
}

void
Control3D::
closeSlot()
{
  close();
}

void
Control3D::
update()
{
  disconnect(depthTestCheck_, &QCheckBox::stateChanged, this, &Control3D::depthTestSlot);
  disconnect(cullFaceCheck_ , &QCheckBox::stateChanged, this, &Control3D::cullFaceSlot);
  disconnect(frontFaceCheck_, &QCheckBox::stateChanged, this, &Control3D::frontFaceSlot);
  disconnect(bgColorEdit_   , &CQColorEdit::colorChanged, this, &Control3D::bgColorSlot);

  disconnect(cameraRotateCheck_, &QCheckBox::stateChanged,
             this, &Control3D::cameraRotateSlot);
  disconnect(cameraZoomEdit_, &CQRealSpin::realValueChanged,
             this, &Control3D::cameraZoomSlot);
  disconnect(cameraNearEdit_, &CQRealSpin::realValueChanged,
             this, &Control3D::cameraNearSlot);
  disconnect(cameraFarEdit_, &CQRealSpin::realValueChanged,
             this, &Control3D::cameraFarSlot);
  disconnect(cameraYawEdit_, &CQRealSpin::realValueChanged,
             this, &Control3D::cameraYawSlot);
  disconnect(cameraPitchEdit_, &CQRealSpin::realValueChanged,
             this, &Control3D::cameraPitchSlot);
  disconnect(cameraRollEdit_, &CQRealSpin::realValueChanged,
             this, &Control3D::cameraRollSlot);
  disconnect(cameraPosEdit_, &CQPoint3DEdit::editingFinished,
             this, &Control3D::cameraPosSlot);

//disconnect(ambientEdit_  , &CQRealSpin::realValueChanged, this, &Control3D::ambientSlot);
//disconnect(diffuseEdit_  , &CQRealSpin::realValueChanged, this, &Control3D::diffuseSlot);
//disconnect(specularEdit_ , &CQRealSpin::realValueChanged, this, &Control3D::specularSlot);
//disconnect(shininessEdit_, &CQRealSpin::realValueChanged, this, &Control3D::shininessSlot);

  //---

  depthTestCheck_->setChecked(canvas_->isDepthTest());
  cullFaceCheck_ ->setChecked(canvas_->isCullFace());
  frontFaceCheck_->setChecked(canvas_->isFrontFace());
  bgColorEdit_   ->setColor(canvas_->bgColor());

  //---

  auto *camera = canvas_->camera();

  if (camera) {
    cameraRotateCheck_->setChecked(camera->isRotate());
    cameraZoomEdit_   ->setValue  (camera->zoom());
    cameraNearEdit_   ->setValue  (camera->near());
    cameraFarEdit_    ->setValue  (camera->far());
    cameraYawEdit_    ->setValue  (camera->yaw());
    cameraPitchEdit_  ->setValue  (camera->pitch());
    cameraRollEdit_   ->setValue  (camera->roll());
    cameraPosEdit_    ->setValue  (vectorToPoint(camera->position()));
  }

  //---

  ambientEdit_  ->setValue(canvas_->ambient());
  diffuseEdit_  ->setValue(canvas_->diffuse());
  specularEdit_ ->setValue(canvas_->specular());
  shininessEdit_->setValue(canvas_->shininess());

  //---

  connect(depthTestCheck_, &QCheckBox::stateChanged, this, &Control3D::depthTestSlot);
  connect(cullFaceCheck_ , &QCheckBox::stateChanged, this, &Control3D::cullFaceSlot);
  connect(frontFaceCheck_, &QCheckBox::stateChanged, this, &Control3D::frontFaceSlot);
  connect(bgColorEdit_   , &CQColorEdit::colorChanged, this, &Control3D::bgColorSlot);

  connect(cameraRotateCheck_, &QCheckBox::stateChanged,
          this, &Control3D::cameraRotateSlot);
  connect(cameraZoomEdit_, &CQRealSpin::realValueChanged,
          this, &Control3D::cameraZoomSlot);
  connect(cameraNearEdit_, &CQRealSpin::realValueChanged,
          this, &Control3D::cameraNearSlot);
  connect(cameraFarEdit_, &CQRealSpin::realValueChanged,
          this, &Control3D::cameraFarSlot);
  connect(cameraYawEdit_, &CQRealSpin::realValueChanged,
          this, &Control3D::cameraYawSlot);
  connect(cameraPitchEdit_, &CQRealSpin::realValueChanged,
          this, &Control3D::cameraPitchSlot);
  connect(cameraRollEdit_, &CQRealSpin::realValueChanged,
          this, &Control3D::cameraRollSlot);
  connect(cameraPosEdit_, &CQPoint3DEdit::editingFinished,
          this, &Control3D::cameraPosSlot);

//connect(ambientEdit_  , &CQRealSpin::realValueChanged, this, &Control3D::ambientSlot);
//connect(diffuseEdit_  , &CQRealSpin::realValueChanged, this, &Control3D::diffuseSlot);
//connect(specularEdit_ , &CQRealSpin::realValueChanged, this, &Control3D::specularSlot);
//connect(shininessEdit_, &CQRealSpin::realValueChanged, this, &Control3D::shininessSlot);

  //---

  updateLights();
}

void
Control3D::
updateLights()
{
  disconnect(lightCheck_     , &QCheckBox::stateChanged, this, &Control3D::lightCheckSlot);
  disconnect(lightColorEdit_ , &CQColorEdit::colorChanged, this, &Control3D::lightColorSlot);
  disconnect(lightPosEdit_   , &CQPoint3DEdit::editingFinished,
             this, &Control3D::lightPosSlot);
  disconnect(lightDirEdit_   , &CQPoint3DEdit::editingFinished,
             this, &Control3D::lightDirSlot);
  disconnect(lightCutoffEdit_, &CQRealSpin::realValueChanged,
             this, &Control3D::lightCutoffSlot);
  disconnect(lightRadiusEdit_, &CQRealSpin::realValueChanged,
             this, &Control3D::lightRadiusSlot);

  disconnect(lightsList_, &QListWidget::currentItemChanged,
             this, &Control3D::lightSelectedSlot);
  disconnect(objectsList_, &QListWidget::currentItemChanged,
             this, &Control3D::objectSelectedSlot);

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

  objectsList_->clear();

  for (auto *object : canvas_->objects()) {
    auto objectName = QString("%1.%2").arg(object->typeName()).arg(object->ind());

    auto *item = new QListWidgetItem(objectName);

    objectsList_->addItem(item);

    item->setData(Qt::UserRole, int(object->ind()));
  }

  //---

  connect(lightCheck_     , &QCheckBox::stateChanged, this, &Control3D::lightCheckSlot);
  connect(lightColorEdit_ , &CQColorEdit::colorChanged, this, &Control3D::lightColorSlot);
  connect(lightPosEdit_   , &CQPoint3DEdit::editingFinished,
          this, &Control3D::lightPosSlot);
  connect(lightDirEdit_   , &CQPoint3DEdit::editingFinished,
          this, &Control3D::lightDirSlot);
  connect(lightCutoffEdit_, &CQRealSpin::realValueChanged,
          this, &Control3D::lightCutoffSlot);
  connect(lightRadiusEdit_, &CQRealSpin::realValueChanged,
          this, &Control3D::lightRadiusSlot);

  connect(lightsList_, &QListWidget::currentItemChanged,
          this, &Control3D::lightSelectedSlot);
  connect(objectsList_, &QListWidget::currentItemChanged,
          this, &Control3D::objectSelectedSlot);
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
cameraZoomSlot(double r)
{
  canvas_->camera()->setZoom(r);
  canvas_->update();
}

void
Control3D::
cameraNearSlot(double r)
{
  canvas_->camera()->setNear(r);
  canvas_->update();
}

void
Control3D::
cameraRotateSlot(int b)
{
  canvas_->camera()->setRotate(b);
  canvas_->update();
}

void
Control3D::
cameraFarSlot(double r)
{
  canvas_->camera()->setFar(r);
  canvas_->update();
}

void
Control3D::
cameraYawSlot(double r)
{
  canvas_->camera()->setYaw(r);
  canvas_->update();
}

void
Control3D::
cameraPitchSlot(double r)
{
  canvas_->camera()->setPitch(r);
  canvas_->update();
}

void
Control3D::
cameraRollSlot(double r)
{
  canvas_->camera()->setRoll(r);
  canvas_->update();
}

void
Control3D::
cameraPosSlot()
{
  auto p = cameraPosEdit_->getValue();

  canvas_->camera()->setPosition(CGLVector3D(p.x, p.y, p.z));
  canvas_->update();
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

  light->setColor(colorToVector(c));
  canvas_->update();
}

void
Control3D::
lightPosSlot()
{
  auto *light = canvas_->currentLight();

  auto p = lightPosEdit_->getValue();
  light->setPosition(CGLVector3D(p.x, p.y, p.z));
  canvas_->update();
}

void
Control3D::
lightDirSlot()
{
  auto *light = canvas_->currentLight();

  auto p = lightDirEdit_->getValue();
  light->setDirection(CGLVector3D(p.x, p.y, p.z));
  canvas_->update();
}

void
Control3D::
lightCutoffSlot(double r)
{
  auto *light = canvas_->currentLight();

  light->setCutoff(r);
  canvas_->update();
}

void
Control3D::
lightRadiusSlot(double r)
{
  auto *light = canvas_->currentLight();

  light->setRadius(r);
  canvas_->update();
}

void
Control3D::
objectSelectedSlot(QListWidgetItem *item, QListWidgetItem *)
{
  int ind = item->data(Qt::UserRole).toInt();

  auto *indObj = canvas_->objectFromInd(ind);

  for (auto *obj : canvas_->objects())
    obj->setSelected(obj == indObj);

  objectTree_->clear();

  if (indObj) {
    auto properties = CQUtil::getPropertyList(indObj);

    for (auto &prop : properties) {
      objectTree_->addProperty("", indObj, prop);
    }
  }
}

}
