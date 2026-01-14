#ifndef CQSandboxApp_H
#define CQSandboxApp_H

#include <CTclUtil.h>

#include <QFrame>

class CQTcl;

class QToolButton;
class QPushButton;
class QLabel;

namespace CQSandbox {

class Canvas;
class Canvas3D;
class Toolbar2D;
class Toolbar3D;
class Control2D;
class Control3D;

class App : public QFrame {
  Q_OBJECT

 public:
  App(QWidget *parent=nullptr);

  CQTcl *tcl() const { return tcl_; }

  Canvas    *canvas   () const { return canvas_; }
  Toolbar2D *toolbar2D() const { return toolbar2D_; }

  Canvas3D  *canvas3D () const { return canvas3D_; }
  Toolbar3D *toolbar3D() const { return toolbar3D_; }

  bool is3D() const { return is3D_; }
  void set3D(bool b) { is3D_ = b; }

  Control2D *control2D() { return control2D_; }
  Control3D *control3D() { return control3D_; }

  bool load(const QString &filename);

  void init();
  void show();

  void setInfo(const QString &label);

  bool runTclCmd(const QString &cmd);

  QStringList getArgs(int objc, const Tcl_Obj **objv) const;

  void errorMsg(const QString &msg) const;

 private:
  CQTcl* tcl_ { nullptr };

  bool initialized_ { false };

  Canvas*    canvas_    { nullptr };
  Toolbar2D* toolbar2D_ { nullptr };
  Canvas3D*  canvas3D_  { nullptr };
  Toolbar3D* toolbar3D_ { nullptr };
  bool       is3D_      { false };
  Control2D* control2D_ { nullptr };
  Control3D* control3D_ { nullptr };
};

}

#endif
