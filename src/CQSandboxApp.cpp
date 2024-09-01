#include <CQSandboxApp.h>
#include <CQSandbox.h>
#include <CQSandboxEditor.h>
#include <CQSandboxCanvas3D.h>

#include <CQIconButton.h>
#include <CQTclUtil.h>

#include <QToolButton>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFile>

#include <svg/play_svg.h>
#include <svg/pause_svg.h>
#include <svg/play_one_svg.h>

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

  if (is3D()) {
    canvas3D_  = new Canvas3D(this);
    control3D_ = new Control3D(canvas3D_);

    canvas3D_->init();

    layout->addWidget(control3D_);
    layout->addWidget(canvas3D_);

    connect(canvas3D_, &Canvas3D::typeChanged, control3D_, &Control3D::updateInfo);
  }
  else {
    canvas_  = new Canvas(this);
    control_ = new Control(canvas_);

    canvas_->init();

    layout->addWidget(control_);
    layout->addWidget(canvas_);
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
  control_->setInfo(label);
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

Control::
Control(Canvas *canvas) :
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

  playButton_  = addToolButton("play" , "PLAY"    , "Play" , SLOT(playSlot()));
  pauseButton_ = addToolButton("pause", "PAUSE"   , "Pause", SLOT(pauseSlot()));
  stepButton_  = addToolButton("step" , "PLAY_ONE", "Step" , SLOT(stepSlot()));

  layout->addWidget(playButton_);
  layout->addWidget(pauseButton_);
  layout->addWidget(stepButton_);

  editButton_ = new QPushButton("Edit");

  connect(editButton_, SIGNAL(clicked()), this, SLOT(editSlot()));

  layout->addWidget(editButton_);

  infoLabel_ = new QLabel(" ");

  layout->addWidget(infoLabel_);

  layout->addStretch(1);
}

void
Control::
setInfo(const QString &label)
{
  infoLabel_->setText(label);
}

void
Control::
playSlot()
{
  canvas()->play();
}

void
Control::
pauseSlot()
{
  canvas()->pause();
}

void
Control::
stepSlot()
{
  canvas()->step();
}

void
Control::
editSlot()
{
  if (! editor_)
    editor_ = new Editor(canvas());

  editor_->show();
}

//---

Control3D::
Control3D(Canvas3D *canvas) :
 QFrame(canvas), canvas_(canvas)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  auto *layout = new QHBoxLayout(this);

  infoLabel_ = new QLabel(" ");

  layout->addWidget(infoLabel_);

  layout->addStretch(1);

  updateInfo();
}

void
Control3D::
setInfo(const QString &label)
{
  infoLabel_->setText(label);
}

void
Control3D::
updateInfo()
{
  auto type = canvas_->type();

  QString text;

  if      (type == OpenGLWindow::Type::CAMERA) {
    text += "Mode: Camera";
  }
  else if (type == OpenGLWindow::Type::LIGHT) {
    text += "Mode: Light";
  }
  else if (type == OpenGLWindow::Type::MODEL) {
    text += "Mode: Model";
  }

  infoLabel_->setText(text);
}

//---

}
