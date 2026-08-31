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

class Canvas;
class Toolbar2D;
class Canvas3D;
class CanvasToolbar3D;
class Control2D;
class Control3D;
class Status;
class Overview3D;
class OverviewToolbar3D;

class App : public QFrame {
  Q_OBJECT

 public:
  static QString buildDir() { return s_buildDir; }

  //---

  App(QWidget *parent=nullptr);

  //---

  bool is3D() const { return is3D_; }
  void set3D(bool b) { is3D_ = b; }

  bool isOverview() const { return isOverview_; }
  void setOverview(bool b) { isOverview_ = b; }

  //---

  Canvas    *canvas   () const { return frame2D_.canvas; }
  Toolbar2D *toolbar2D() const { return frame2D_.toolbar; }
  Control2D *control2D() const { return frame2D_.control; }

  Canvas3D          *canvas3D         () const { return frame3D_.canvas; }
  CanvasToolbar3D   *canvasToolbar3D  () const { return frame3D_.toolbar; }
  Control3D         *control3D        () const { return frame3D_.control; }
  Overview3D        *overview3D       () const { return frame3D_.overview; }
  OverviewToolbar3D *overviewToolbar3D() const { return frame3D_.overviewToolbar; }

  Status *status() const { return status_; }

  //---

  bool load(CQTcl *tcl, const QString &filename);

  void init();
  void show();

  void setInfo(const QString &label);

//bool runTclCmd(const QString &cmd);

  QStringList getArgs(int objc, const Tcl_Obj **objv) const;

  bool errorMsg(const QString &msg) const;

  //---

#ifdef CQSANDBOX_META_EDIT
  void showMetaEdit();
#endif
  void showPerfDialog();
  void showAppOptions();

 private:
  struct Frame3D {
    Canvas3D*          canvas          { nullptr };
    CanvasToolbar3D*   toolbar         { nullptr };
    Control3D*         control         { nullptr };
    Overview3D*        overview        { nullptr };
    OverviewToolbar3D* overviewToolbar { nullptr };
    CQTabSplit*        tab             { nullptr };
  };

  struct Frame2D {
    Canvas*    canvas  { nullptr };
    Toolbar2D* toolbar { nullptr };
    Control2D* control { nullptr };
  };

 private:
  QFrame *add3DFrame(Frame3D &frame3D);
  QFrame *add2DFrame(Frame2D &frame2D);

 private:
  static QString s_buildDir;

  bool initialized_ { false };

  bool is3D_       { false };
  bool isOverview_ { false };

  Frame3D frame3D_;
  Frame2D frame2D_;

  Status* status_ { nullptr };
};

}

#endif
