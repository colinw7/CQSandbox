#include <CQSandboxApp.h>
#include <CQSandboxCanvas.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxControl2D.h>
#include <CQSandboxToolbar2D.h>
#include <CQSandboxControl3D.h>
#include <CQSandboxToolbar3D.h>

#include <CQTclUtil.h>

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
  if (initialized_)
    return;

  initialized_ = true;

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

}
