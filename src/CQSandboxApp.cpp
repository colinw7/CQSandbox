#include <CQSandboxApp.h>

#include <CQSandboxCanvas2D.h>
#include <CQSandboxControl2D.h>
#include <CQSandboxToolbar2D.h>
#include <CQSandboxConsole2D.h>

#include <CQSandboxCanvas3D.h>
#include <CQSandboxControl3D.h>
#include <CQSandboxToolbar3D.h>
#include <CQSandboxOverview3D.h>

#include <CQSandboxStatus.h>

#include <CQTclUtil.h>
#include <CQTabSplit.h>

#ifdef CQSANDBOX_META_EDIT
#include <CQMetaEdit.h>
#endif
#include <CQAppOptions.h>
#include <CQUtil.h>

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
#include <svg/overview_svg.h>

#include <svg/settings_svg.h>
#include <svg/console_svg.h>

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
  setObjectName("app");

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

  appFrame_ = new QFrame;

  appFrame_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  auto *layout1 = new QVBoxLayout(appFrame_);
  layout1->setMargin(0); layout1->setSpacing(0);

  layout->addWidget(appFrame_);

  //---

  if      (is2D())
    init2DFrame();
  else if (is3D())
    init3DFrame();

  //---

  status_ = new Status(this);

  layout->addWidget(status_);
}

void
App::
init2DFrame()
{
  auto *frame = add2DFrame(frame2D_);

  appFrame_->layout()->addWidget(frame);
}

void
App::
init3DFrame()
{
  auto *frame = add3DFrame(frame3D_);

  appFrame_->layout()->addWidget(frame);
}

QFrame *
App::
add3DFrame(Frame3D &frame3D)
{
  auto *frame = CQUtil::makeWidget<QFrame>("frame3D");

  auto *layout = new QHBoxLayout(frame);
  layout->setMargin(0); layout->setSpacing(0);

  //---

  // canvas and toolbar
  frame3D.canvasFrame = CQUtil::makeWidget<QFrame>("canvasFrame");

  auto *clayout = new QVBoxLayout(frame3D.canvasFrame);
  clayout->setMargin(0); clayout->setSpacing(0);

  frame3D.canvas  = new Canvas3D(this);
  frame3D.toolbar = new CanvasToolbar3D(frame3D.canvas);

  clayout->addWidget(frame3D.toolbar);
  clayout->addWidget(frame3D.canvas);

  //---

  // view frame
  frame3D.frame = CQUtil::makeWidget<QFrame>("frame");

  auto *flayout = new QHBoxLayout(frame3D.frame);
  flayout->setMargin(0); flayout->setSpacing(0);

  flayout->addWidget(frame3D.canvasFrame);

  layout->addWidget(frame3D.frame);

  showOverview3DI(frame3D, isOverview());

  //---

  // control frame
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

bool
App::
hasOverview3D() const
{
  return frame3D_.overviewFrame;
}

void
App::
showOverview3D(bool show)
{
  showOverview3DI(frame3D_, show);
}

void
App::
showOverview3DI(Frame3D &frame3D, bool show)
{
  auto *layout = qobject_cast<QBoxLayout *>(frame3D.frame->layout());
  layout->setMargin(0); layout->setSpacing(0);

  if (show) {
    if (frame3D.overlayShown)
      return; // already show

    //---

    if (! frame3D.overviewFrame) {
      frame3D.overviewFrame = CQUtil::makeWidget<QFrame>("overviewFrame");

      auto *overviewLayout = new QVBoxLayout(frame3D.overviewFrame);
      overviewLayout->setMargin(0); overviewLayout->setSpacing(0);

      frame3D.overview        = new Overview3D(this);
      frame3D.overviewToolbar = new OverviewToolbar3D(frame3D.overview);

      overviewLayout->addWidget(frame3D.overviewToolbar);
      overviewLayout->addWidget(frame3D.overview);
    }

    //---

    assert(! frame3D.tab);

    frame3D.tab = new CQTabSplit;

    frame3D.tab->setState(CQTabSplit::State::TAB);

    frame3D.tab->addWidget(frame3D.canvasFrame  , "3D");
    frame3D.tab->addWidget(frame3D.overviewFrame, "2D");

    layout->addWidget(frame3D.tab);

    frame3D.canvasFrame  ->show();
    frame3D.overviewFrame->show();
  }
  else {
    if (! frame3D.overlayShown)
      return;

    //---

    assert(frame3D.tab);

    frame3D.tab->removeWidget(frame3D.canvasFrame  , /*delete*/false);
    frame3D.tab->removeWidget(frame3D.overviewFrame, /*delete*/false);

    delete frame3D.tab;

    frame3D.tab = nullptr;

    //---

    layout->addWidget(frame3D.canvasFrame);

    frame3D.canvasFrame  ->show();
    frame3D.overviewFrame->hide();
  }

  frame3D.overlayShown = show;
}

QFrame *
App::
add2DFrame(Frame2D &frame2D)
{
  frame2D.frame = CQUtil::makeWidget<QFrame>("frame2D");

  auto *layout = new QHBoxLayout(frame2D.frame);
  layout->setMargin(0); layout->setSpacing(0);

  //---

  auto *clayout = new QVBoxLayout;
  clayout->setMargin(0); clayout->setSpacing(0);

  layout->addLayout(clayout);

  //---

  frame2D.canvas  = new Canvas2D(this);
  frame2D.toolbar = new CanvasToolbar2D(frame2D.canvas);

  frame2D.canvas->init();

  clayout->addWidget(frame2D.toolbar);
  clayout->addWidget(frame2D.canvas);

  //---

  frame2D.control = new Control2D(frame2D.canvas);

  layout->addWidget(frame2D.control);

  frame2D.control->hide();

  //---

  return frame2D.frame;
}

void
App::
show()
{
  if (canvas2D())
    canvas2D()->init();

  QFrame::show();
}

Console2D *
App::
console2D() const
{
  if (! console2D_) {
    auto *th = const_cast<App *>(this);

    th->console2D_ = new Console2D(canvas2D());
  }

  return console2D_;
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
