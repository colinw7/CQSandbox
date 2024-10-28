#include <CQSandboxApp.h>
#include <CQSandboxCanvas.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxControl2D.h>
#include <CQSandboxControl3D.h>

#include <CQIconButton.h>
#include <CQTclUtil.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QFile>

#include <svg/play_svg.h>
#include <svg/pause_svg.h>
#include <svg/play_one_svg.h>
#include <svg/camera_svg.h>
#include <svg/model_svg.h>
#include <svg/light_svg.h>
#include <svg/wireframe_svg.h>
#include <svg/bbox_svg.h>
#include <svg/settings_svg.h>

namespace CQSandbox {

//---

App::
App(QWidget *parent) :
 QFrame(parent)
{
  tcl_ = new CQTcl;

  tcl_->init();
}

void
App::
init()
{
  auto *layout = new QVBoxLayout(this);

  auto *clayout = new QHBoxLayout;

  //---

  if (is3D()) {
    canvas3D_  = new Canvas3D(this);
    toolbar3D_ = new Toolbar3D(canvas3D_);

    canvas3D_->init();

    layout->addWidget(toolbar3D_);
    layout->addLayout(clayout);

    clayout->addWidget(canvas3D_);

    connect(canvas3D_, &Canvas3D::typeChanged, toolbar3D_, &Toolbar3D::updateInfo);

    //---

    control3D_ = new Control3D(canvas3D_);

    clayout->addWidget(control3D_);

    control3D_->hide();
  }
  else {
    canvas_    = new Canvas(this);
    toolbar2D_ = new Toolbar2D(canvas_);

    canvas_->init();

    layout->addWidget(toolbar2D_);
    layout->addLayout(clayout);

    clayout->addWidget(canvas_);

    //---

    control2D_ = new Control2D(canvas_);

    clayout->addWidget(control2D_);

    control2D_->hide();
  }
}

void
App::
show()
{
  if (canvas())
    canvas()->init();

  QFrame::show();
}

void
App::
setInfo(const QString &label)
{
  if (toolbar2D_)
    toolbar2D_->setInfo(label);
}

bool
App::
load(const QString &fileName)
{
  auto fileToLines = [&](const QString &fileName, QStringList &lines) {
    QFile file(fileName);

    if (! file.open(QIODevice::ReadOnly | QIODevice::Text))
      return false;

    QString line;

    while (! file.atEnd()) {
      QByteArray bytes = file.readLine();

      QString line1(bytes);

      if (line1.right(1) == '\n')
        line1 = line1.mid(0, line1.length() - 1);

      lines.push_back(line1);
    }

    return true;
  };

  QStringList lines;

  if (! fileToLines(fileName, lines))
    return false;

  QString line;

  for (const auto &line1 : lines) {
    if (line1.trimmed() == "")
      continue;

    if (line.length())
      line += "\n" + line1;
    else
      line = line1;

    if (CTclUtil::isCompleteLine(line.toStdString())) {
      runTclCmd(line);

      line = "";
    }
  }

  if (line != "")
    runTclCmd(line);

  return true;
}

bool
App::
runTclCmd(const QString &cmd)
{
  auto rc = tcl_->eval(cmd, /*showError*/true, /*showResult*/false);

  if (! rc)
    errorMsg(QString("Command '%1' failed").arg(cmd));

  return rc;
}

QStringList
App::
getArgs(int objc, const Tcl_Obj **objv) const
{
  QStringList args;

  for (int i = 1; i < objc; ++i) {
    auto str = CTclUtil::stringFromObj(const_cast<Tcl_Obj *>(objv[size_t(i)]));

    args.push_back(QString::fromStdString(str));
  }

  return args;
}

void
App::
errorMsg(const QString &msg) const
{
  std::cerr << msg.toStdString() << "\n";
}

//---

Toolbar2D::
Toolbar2D(Canvas *canvas) :
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

  playButton_  = addToolButton("play" , "PLAY"    , "Play" , SLOT(playSlot()));
  pauseButton_ = addToolButton("pause", "PAUSE"   , "Pause", SLOT(pauseSlot()));
  stepButton_  = addToolButton("step" , "PLAY_ONE", "Step" , SLOT(stepSlot()));

  layout->addWidget(playButton_);
  layout->addWidget(pauseButton_);
  layout->addWidget(stepButton_);

  //---

  infoLabel_ = new QLabel(" ");

  layout->addWidget(infoLabel_);

  layout->addStretch(1);

  //---

  settingsButton_ = addCheckButton("settings", "SETTINGS" , "Settings", SLOT(settingsSlot()));

  layout->addWidget(settingsButton_);
}

void
Toolbar2D::
setInfo(const QString &label)
{
  infoLabel_->setText(label);
}

void
Toolbar2D::
playSlot()
{
  canvas()->play();
}

void
Toolbar2D::
pauseSlot()
{
  canvas()->pause();
}

void
Toolbar2D::
stepSlot()
{
  canvas()->step();
}

void
Toolbar2D::
settingsSlot()
{
  auto *button = qobject_cast<CQIconButton *>(sender());

  auto *app = canvas()->app();

  app->control2D()->setVisible(button->isChecked());
}

//---

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

//---

}
