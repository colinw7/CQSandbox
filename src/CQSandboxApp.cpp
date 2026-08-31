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

#ifdef CQSANDBOX_META_EDIT
#include <CQMetaEdit.h>
#endif
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
  //tcl_ = new CQTcl;

  //tcl_->init();
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

  //---

  if (is3D()) {
    auto *frame = add3DFrame(frame3D_);

    layout->addWidget(frame);
  }
  else {
    auto *frame = add2DFrame(frame2D_);

    layout->addWidget(frame);
  }

  status_ = new Status(this);

  layout->addWidget(status_);
}

QFrame *
App::
add3DFrame(Frame3D &frame3D)
{
  auto *frame  = new QFrame;
  auto *layout = new QHBoxLayout(frame);

  //---

  auto *canvasFrame = new QFrame;

  auto *clayout = new QVBoxLayout(canvasFrame);
  clayout->setMargin(0); clayout->setSpacing(0);

  //---

  frame3D.canvas  = new Canvas3D(this);
  frame3D.toolbar = new CanvasToolbar3D(frame3D.canvas);

  clayout->addWidget(frame3D.toolbar);
  clayout->addWidget(frame3D.canvas);

  //---

  if (isOverview()) {
    auto *overviewFrame = new QFrame;

    auto *overviewLayout = new QVBoxLayout(overviewFrame);
    overviewLayout->setMargin(0); overviewLayout->setSpacing(0);

    frame3D.overview        = new Overview3D(this);
    frame3D.overviewToolbar = new OverviewToolbar3D(frame3D.overview);

    overviewLayout->addWidget(frame3D.overviewToolbar);
    overviewLayout->addWidget(frame3D.overview);

    //---

    frame3D.tab = new CQTabSplit;

    frame3D.tab->setState(CQTabSplit::State::TAB);

    frame3D.tab->addWidget(canvasFrame  , "3D");
    frame3D.tab->addWidget(overviewFrame, "2D");

    layout->addWidget(frame3D.tab);
  }
  else {
    layout->addWidget(canvasFrame);
  }

  //---

  frame3D.control = new Control3D(frame3D.canvas);

  layout->addWidget(frame3D.control);

  frame3D.control->hide();

  //---

  frame3D.canvas->init();

  if (frame3D.overview)
    frame3D.overview->init();

  frame3D.control->init();

  //---

  return frame;
}

QFrame *
App::
add2DFrame(Frame2D &frame2D)
{
  auto *frame  = new QFrame;
  auto *layout = new QHBoxLayout(frame);

  //---

  auto *clayout = new QVBoxLayout;
  clayout->setMargin(0); clayout->setSpacing(0);

  layout->addLayout(clayout);

  //---

  frame2D.canvas  = new Canvas(this);
  frame2D.toolbar = new Toolbar2D(frame2D.canvas);

  frame2D.canvas->init();

  clayout->addWidget(frame2D.toolbar);
  clayout->addWidget(frame2D.canvas);

  //---

  frame2D.control = new Control2D(frame2D.canvas);

  layout->addWidget(frame2D.control);

  frame2D.control->hide();

  //---

  return frame;
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
  if (toolbar2D())
    toolbar2D()->setInfo(label);
}

bool
App::
load(CQTcl *tcl, const QString &fileName)
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

  auto runTclCmd = [&](const QString &cmd) {
    auto rc = tcl->eval(cmd, /*showError*/true, /*showResult*/false);

    if (! rc)
      (void) errorMsg(QString("Command '%1' failed").arg(cmd));

    return rc;
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

#if 0
bool
App::
runTclCmd(const QString &cmd)
{
  auto rc = tcl_->eval(cmd, /*showError*/true, /*showResult*/false);

  if (! rc)
    errorMsg(QString("Command '%1' failed").arg(cmd));

  return rc;
}
#endif

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

#ifdef CQSANDBOX_META_EDIT
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
#endif

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
