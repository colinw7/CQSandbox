#ifndef CQSandboxApp_H
#define CQSandboxApp_H

#include <CTclUtil.h>

#include <QFrame>

class CQTcl;
class CQTabSplit;

class QToolButton;
class QPushButton;
class QLabel;

namespace CQSandbox {

class Canvas2D;
class CanvasToolbar2D;
class Control2D;
class Console2D;

class Canvas3D;
class CanvasToolbar3D;
class Overview3D;
class OverviewToolbar3D;
class Control3D;

class Status;

class App : public QFrame {
  Q_OBJECT

 public:
  static QString buildDir() { return s_buildDir; }

  //---

  App(QWidget *parent=nullptr);

  //---

  bool is2D() const { return is2D_; }
  void set2D(bool b) { is2D_ = b; }

  bool is3D() const { return is3D_; }
  void set3D(bool b) { is3D_ = b; }

  bool isOverview() const { return isOverview_; }
  void setOverview(bool b) { isOverview_ = b; }

  //---

  // 2D
  Canvas2D        *canvas2D       () const { return frame2D_.canvas; }
  CanvasToolbar2D *canvasToolbar2D() const { return frame2D_.toolbar; }
  Control2D       *control2D      () const { return frame2D_.control; }

  Console2D *console2D() const;

  // 3D
  Canvas3D          *canvas3D         () const { return frame3D_.canvas; }
  CanvasToolbar3D   *canvasToolbar3D  () const { return frame3D_.toolbar; }
  Control3D         *control3D        () const { return frame3D_.control; }
  Overview3D        *overview3D       () const { return frame3D_.overview; }
  OverviewToolbar3D *overviewToolbar3D() const { return frame3D_.overviewToolbar; }

  // Common
  Status *status() const { return status_; }

  //---

  bool load(CQTcl *tcl, const QString &filename);

  void init();
  void show();

//bool runTclCmd(const QString &cmd);

  QStringList getArgs(int objc, const Tcl_Obj **objv) const;

  bool errorMsg(const QString &msg) const;

  //---

  bool hasOverview3D() const;
  void showOverview3D(bool show);

  //---

#ifdef CQSANDBOX_META_EDIT
  void showMetaEdit();
#endif
  void showPerfDialog();
  void showAppOptions();

 private:
  struct Frame3D {
    QFrame*            frame           { nullptr };
    QFrame*            canvasFrame     { nullptr };
    Canvas3D*          canvas          { nullptr };
    CanvasToolbar3D*   toolbar         { nullptr };
    Control3D*         control         { nullptr };
    bool               overlayShown    { false };
    QFrame*            overviewFrame   { nullptr };
    Overview3D*        overview        { nullptr };
    OverviewToolbar3D* overviewToolbar { nullptr };
    CQTabSplit*        tab             { nullptr };
  };

  struct Frame2D {
    QFrame*          frame   { nullptr };
    Canvas2D*        canvas  { nullptr };
    CanvasToolbar2D* toolbar { nullptr };
    Control2D*       control { nullptr };
  };

 private:
  void init2DFrame();
  void init3DFrame();

  QFrame *add3DFrame(Frame3D &frame3D);
  QFrame *add2DFrame(Frame2D &frame2D);

  void showOverview3DI(Frame3D &frame3D, bool show);

 private:
  static QString s_buildDir;

  bool initialized_ { false };

  bool is3D_       { false };
  bool is2D_       { false };
  bool isOverview_ { false };

  QFrame* appFrame_ { nullptr };

  Frame3D frame3D_;
  Frame2D frame2D_;

  Console2D* console2D_ { nullptr };

  Status* status_ { nullptr };
};

}

#endif
