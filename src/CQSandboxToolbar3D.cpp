#include <CQSandboxToolbar3D.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxControl3D.h>
#include <CQSandboxApp.h>

#include <CQIconButton.h>

#include <QLabel>
#include <QHBoxLayout>

namespace CQSandbox {

Toolbar3D::
Toolbar3D(Canvas3D *canvas) :
 QFrame(canvas), canvas_(canvas)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  auto *layout = new QHBoxLayout(this);

  auto addToolButton = [&](const QString &name, const QString &iconName,
                           const QString &tip, const char *slotName) {
    auto *button = new CQIconButton;

    button->setObjectName(name);
    button->setIcon(iconName);
    button->setIconSize(QSize(32, 32));
    button->setAutoRaise(true);
    button->setToolTip(tip);

    connect(button, SIGNAL(clicked()), this, slotName);

    return button;
  };

  auto addCheckButton = [&](const QString &name, const QString &iconName,
                            const QString &tip, const char *slotName) {
    auto *button = new CQIconButton;

    button->setObjectName(name);
    button->setCheckable(true);
    button->setIcon(iconName);
    button->setIconSize(QSize(32, 32));
    button->setAutoRaise(true);
    button->setToolTip(tip);

    connect(button, SIGNAL(clicked()), this, slotName);

    return button;
  };

  cameraButton_ = addToolButton("camera", "CAMERA", "Camera", SLOT(cameraSlot()));
  modelButton_  = addToolButton("model" , "MODEL" , "Model" , SLOT(modelSlot()));
  lightButton_  = addToolButton("light" , "LIGHT" , "Light" , SLOT(lightSlot()));

  layout->addWidget(cameraButton_);
  layout->addWidget(modelButton_);
  layout->addWidget(lightButton_);

  wireButton_ = addCheckButton("wire", "WIREFRAME", "Wireframe", SLOT(wireSlot()));
  bboxButton_ = addCheckButton("bbox", "BBOX"     , "Model"    , SLOT(bboxSlot()));

  layout->addWidget(wireButton_);
  layout->addWidget(bboxButton_);

  //---

  infoLabel_ = new QLabel(" ");

  layout->addWidget(infoLabel_);

  posLabel_ = new QLabel(" ");

  layout->addWidget(posLabel_);

  layout->addStretch(1);

  //---

  settingsButton_ = addCheckButton("settings", "SETTINGS" , "Settings", SLOT(settingsSlot()));

  layout->addWidget(settingsButton_);

  //---

  updateInfo();
}

void
Toolbar3D::
setInfo(const QString &label)
{
  infoLabel_->setText(label);
}

void
Toolbar3D::
setPos(const QString &label)
{
  posLabel_->setText(label);
}

void
Toolbar3D::
updateInfo()
{
  auto type = canvas_->type();

  QString text;

  if      (type == Canvas3D::Type::CAMERA) {
    text += "Mode: Camera";
  }
  else if (type == Canvas3D::Type::LIGHT) {
    text += "Mode: Light";
    text += " #" + QString::number(canvas_->lightNum());
  }
  else if (type == Canvas3D::Type::MODEL) {
    text += "Mode: Model";
  }
  else if (type == Canvas3D::Type::GAME) {
    text += "Mode: Game";
  }

  infoLabel_->setText(text);
}

void
Toolbar3D::
cameraSlot()
{
  canvas_->setType(Canvas3D::Type::CAMERA);
}

void
Toolbar3D::
modelSlot()
{
  canvas_->setType(Canvas3D::Type::MODEL);
}

void
Toolbar3D::
lightSlot()
{
  canvas_->setType(Canvas3D::Type::LIGHT);
}

void
Toolbar3D::
wireSlot()
{
  auto *button = qobject_cast<CQIconButton *>(sender());

  canvas_->setWireframe(button->isChecked());

  canvas_->update();
}

void
Toolbar3D::
bboxSlot()
{
  auto *button = qobject_cast<CQIconButton *>(sender());

  canvas_->setBBox(button->isChecked());

  canvas_->update();
}

void
Toolbar3D::
settingsSlot()
{
  auto *button = qobject_cast<CQIconButton *>(sender());

  auto *app     = canvas()->app();
  auto *control = app->control3D();

  auto geom = app->geometry();

  int w = control->sizeHint().width();

  QRect geom1;

  if (button->isChecked()) {
    geom1 = QRect(geom.x(), geom.y(), geom.width() + w + 6, geom.height());

    control->update();
    control->show();
  }
  else {
    geom1 = QRect(geom.x(), geom.y(), geom.width() - w - 6, geom.height());

    control->hide();
  }

  app->setGeometry(geom1);

  if (button->isChecked())
    control->setFixedWidth(w);
  else {
    control->setMinimumWidth(0);
    control->setMaximumWidth(QWIDGETSIZE_MAX);
  }
}

}
