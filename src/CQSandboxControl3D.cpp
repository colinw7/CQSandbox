#include <CQSandboxControl3D.h>
#include <CQSandboxCanvas3D.h>
#include <CGLCamera.h>

#include <CQColorEdit.h>
#include <CQPoint3DEdit.h>
#include <CQRealSpin.h>

#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>

namespace CQSandbox {

CPoint3D vectorToPoint(const CGLVector3D &v) {
  return CPoint3D(v.getX(), v.getY(), v.getZ());
}

QColor vectorToColor(const CGLVector3D &v) {
  return QColor(v.getX(), v.getY(), v.getZ());
}

CGLVector3D colorToVector(const QColor &c) {
  return CGLVector3D(c.redF(), c.greenF(), c.blueF());
}

CanvasControl3D::
CanvasControl3D(CQSandbox::Canvas3D *canvas) :
 QFrame(nullptr), canvas_(canvas)
{
  auto *layout = new QGridLayout(this);

  int r = 0;

  auto addLabelEdit = [&](const QString &label, QWidget *w) {
    layout->addWidget(new QLabel(label), r, 0);
    layout->addWidget(w, r, 1);
    ++r;
  };

  //---

  auto *bgColorEdit = new CQColorEdit;

  bgColorEdit->setColor(canvas_->bgColor());

  connect(bgColorEdit, &CQColorEdit::colorChanged, this, &CanvasControl3D::bgColorSlot);

  addLabelEdit("Bg Color", bgColorEdit);

  //---

  auto *nearEdit = new CQRealSpin;

  nearEdit->setValue(canvas_->camera()->near());

  connect(nearEdit, &CQRealSpin::realValueChanged, this, &CanvasControl3D::nearSlot);

  addLabelEdit("Near", nearEdit);

  auto *farEdit = new CQRealSpin;

  farEdit->setValue(canvas_->camera()->far());

  connect(farEdit, &CQRealSpin::realValueChanged, this, &CanvasControl3D::farSlot);

  addLabelEdit("Far", farEdit);

  //---

  auto *lightColorEdit = new CQColorEdit;

  lightColorEdit->setColor(vectorToColor(canvas_->light()->color()));

  connect(lightColorEdit, &CQColorEdit::colorChanged, this, &CanvasControl3D::lightColorSlot);

  addLabelEdit("Light Color", lightColorEdit);

  //---

  auto *lightPosEdit = new CQPoint3DEdit;

  auto lightPos = canvas_->light()->pos();

  lightPosEdit->setValue(vectorToPoint(lightPos));

//connect(lightPosEdit, &CQColorEdit::editingFinished, this, &CanvasControl3D::lightPosSlot);

  addLabelEdit("Light Pos", lightPosEdit);

  //---

  auto *ambientEdit = new CQRealSpin;

  ambientEdit->setRange(0.0, 1.0);
  ambientEdit->setValue(canvas_->ambient());

//connect(ambientEdit, &CQRealSpin::realValueChanged, this, &CanvasControl3D::ambientSlot);

  addLabelEdit("Ambient", ambientEdit);

  //---

  auto *diffuseEdit = new CQRealSpin;

  diffuseEdit->setRange(0.0, 1.0);
  diffuseEdit->setValue(canvas_->diffuse());

//connect(diffuseEdit, &CQRealSpin::realValueChanged, this, &CanvasControl3D::diffuseSlot);

  addLabelEdit("Diffuse", diffuseEdit);

  //---

  auto *specularEdit = new CQRealSpin;

  specularEdit->setRange(0.0, 1.0);
  specularEdit->setValue(canvas_->specular());

//connect(specularEdit, &CQRealSpin::realValueChanged, this, &CanvasControl3D::specularSlot);

  addLabelEdit("Specular", specularEdit);

  //---

  auto *shininessEdit = new CQRealSpin;

  shininessEdit->setRange(0.0, 100.0);
  shininessEdit->setValue(canvas_->shininess());

//connect(shininessEdit, &CQRealSpin::realValueChanged, this, &CanvasControl3D::shininessSlot);

  addLabelEdit("Shininess", shininessEdit);

  //---

  layout->setRowStretch(r, 1);
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
lightColorSlot(const QColor &c)
{
  canvas_->light()->setColor(colorToVector(c));
  canvas_->update();
}

}
