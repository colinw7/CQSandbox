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
class Control;
class Control3D;
class CanvasControl3D;
class Editor;

class App : public QFrame {
  Q_OBJECT

 public:
  App(QWidget *parent=nullptr);

  CQTcl *tcl() const { return tcl_; }

  Canvas  *canvas () const { return canvas_; }
  Control *control() const { return control_; }

  Canvas3D  *canvas3D () const { return canvas3D_; }
  Control3D *control3D() const { return control3D_; }

  bool is3D() const { return is3D_; }
  void set3D(bool b) { is3D_ = b; }

  bool load(const QString &filename);

  void init();
  void show();

  void setInfo(const QString &label);

  bool runTclCmd(const QString &cmd);

  QStringList getArgs(int objc, const Tcl_Obj **objv) const;

  void errorMsg(const QString &msg) const;

 private:
  CQTcl* tcl_ { nullptr };

  Canvas*    canvas_    { nullptr };
  Control*   control_   { nullptr };
  Canvas3D*  canvas3D_  { nullptr };
  Control3D* control3D_ { nullptr };
  bool       is3D_      { false };
};

//---

class Control : public QFrame {
  Q_OBJECT

 public:
  Control(Canvas *canvas);

  Canvas *canvas() const { return canvas_; }

  void setInfo(const QString &label);

 private Q_SLOTS:
  void playSlot();
  void pauseSlot();
  void stepSlot();
  void editSlot();

 private:
  Canvas* canvas_ { nullptr };

  QToolButton* playButton_  { nullptr };
  QToolButton* pauseButton_ { nullptr };
  QToolButton* stepButton_  { nullptr };

  QPushButton *editButton_ { nullptr };

  QLabel* infoLabel_ { nullptr };

  Editor* editor_ { nullptr };
};

//---

class Control3D : public QFrame {
  Q_OBJECT

 public:
  Control3D(Canvas3D *canvas);

  Canvas3D *canvas() const { return canvas_; }

  void setInfo(const QString &label);

  void setPos(const QString &label);

 public Q_SLOTS:
  void updateInfo();

  void cameraSlot();
  void modelSlot();
  void lightSlot();
  void wireSlot();
  void settingsSlot();

 private:
  Canvas3D* canvas_ { nullptr };

  QToolButton* cameraButton_   { nullptr };
  QToolButton* modelButton_    { nullptr };
  QToolButton* lightButton_    { nullptr };
  QToolButton* wireButton_     { nullptr };
  QToolButton* settingsButton_ { nullptr };
  QLabel*      infoLabel_      { nullptr };
  QLabel*      posLabel_       { nullptr };

  CanvasControl3D *canvasControl_ { nullptr };
};

}

#endif
