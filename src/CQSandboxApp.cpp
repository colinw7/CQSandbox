#include <CQSandboxApp.h>
#include <CQSandboxCanvas.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxControl2D.h>
#include <CQSandboxToolbar2D.h>
#include <CQSandboxControl3D.h>
#include <CQSandboxToolbar3D.h>
#include <CQSandboxStatus.h>
#include <CQSandboxOverview3D.h>

#include <CQTclUtil.h>
#include <CQTabSplit.h>

#include <CQMetaEdit.h>
#include <CQAppOptions.h>

#ifdef CQ_PERF_GRAPH
#include <CQPerfGraph.h>
#endif

#include <QVBoxLayout>
#include <QFile>

#include <svg/play_svg.h>
#include <svg/pause_svg.h>
#include <svg/play_one_svg.h>

#include <svg/camera_svg.h>
#include <svg/model_svg.h>
#include <svg/light_svg.h>
#include <svg/game_svg.h>

#include <svg/edge_select_svg.h>
#include <svg/face_select_svg.h>
#include <svg/object_select_svg.h>
#include <svg/point_select_svg.h>

#include <svg/wireframe_svg.h>
#include <svg/solid_fill_svg.h>
#include <svg/texture_fill_svg.h>

#include <svg/bbox_svg.h>

#include <svg/settings_svg.h>

namespace CQSandbox {

//---

#define Q(x) #x
#define QUOTE(x) Q(x)

QString App::s_buildDir = QUOTE(BUILD_DIR);

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
  layout->setMargin(0); layout->setSpacing(0);

  auto *clayout = new QHBoxLayout;
  clayout->setMargin(0); clayout->setSpacing(0);

  //---

  if (is3D()) {
    layout->addLayout(clayout);

    //---

    canvas3D_        = new Canvas3D(this);
    canvasToolbar3D_ = new CanvasToolbar3D(canvas3D_);

    auto *canvasFrame = new QFrame;

    auto *canvasLayout = new QVBoxLayout(canvasFrame);
    canvasLayout->setMargin(0); canvasLayout->setSpacing(0);

    canvasLayout->addWidget(canvasToolbar3D_);
    canvasLayout->addWidget(canvas3D_);

    if (isOverview()) {
      overview3D_        = new Overview3D(this);
      overviewToolbar3D_ = new OverviewToolbar3D(overview3D_);

      auto *overviewFrame = new QFrame;

      auto *overviewLayout = new QVBoxLayout(overviewFrame);
      overviewLayout->setMargin(0); overviewLayout->setSpacing(0);

      overviewLayout->addWidget(overviewToolbar3D_);
      overviewLayout->addWidget(overview3D_);

      //---

      tab_ = new CQTabSplit;

      tab_->setState(CQTabSplit::State::TAB);

      tab_->addWidget(canvasFrame  , "3D");
      tab_->addWidget(overviewFrame, "2D");

      clayout->addWidget(tab_);
    }
    else {
      clayout->addWidget(canvasFrame);
    }

    //---

    control3D_ = new Control3D(canvas3D_);

    clayout->addWidget(control3D_);

    control3D_->hide();

    //---

    canvas3D_->init();

    if (overview3D_)
      overview3D_->init();

    control3D_->init();

    connect(canvas3D_, &Canvas3D::typeChanged, canvasToolbar3D_, &CanvasToolbar3D::updateInfo);
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

  status_ = new Status(this);

  layout->addWidget(status_);
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

    QTextStream in(&file);
    in.setCodec("UTF-8");

    QString line;

    while (! in.atEnd()) {
      auto line1 = in.readLine();

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

bool
App::
errorMsg(const QString &msg) const
{
  std::cerr << msg.toStdString() << "\n";
  return false;
}

//---

void
App::
showMetaEdit()
{
  static CQMetaEdit *metaEdit;

  if (! metaEdit)
    metaEdit = new CQMetaEdit;

  metaEdit->show();

  metaEdit->raise();
}

void
App::
showPerfDialog()
{
#ifdef CQ_PERF_GRAPH
  auto *dialog = CQPerfDialog::instance();

  dialog->show();
#endif
}

void
App::
showAppOptions()
{
  CQAppOptions::show();
}

}
