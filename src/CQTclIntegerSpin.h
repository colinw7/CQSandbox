#ifndef CQTclIntegerSpin_H
#define CQTclIntegerSpin_H

#include <QSpinBox>

struct Tcl_Interp;

class CQTclIntegerSpin : public QSpinBox {
  Q_OBJECT

  Q_PROPERTY(QString varName  READ varName    WRITE setVarName)
  Q_PROPERTY(bool    autoStep READ isAutoStep WRITE setAutoStep)
  Q_PROPERTY(int     step     READ step)

 public:
  CQTclIntegerSpin(QWidget *parent, Tcl_Interp *interp=nullptr, const QString &varName="");
  CQTclIntegerSpin(Tcl_Interp *interp=nullptr, const QString &varName="");

  virtual ~CQTclIntegerSpin() { }

  const Tcl_Interp *interp() const { return interp_; }
  void setInterp(Tcl_Interp *p);

  const QString &varName() const { return varName_; }
  void setVarName(const QString &s);

  bool isAutoStep() const { return autoStep_; }
  void setAutoStep(bool b) { autoStep_ = b; }

  int cursorPosition() const;
  void setCursorPosition(int pos);

  int step() const { return step_; }

  void stepBy(int n) override;

  void handleTrace(int /*flags*/);

 Q_SIGNALS:
  void stepChanged(int);

 private Q_SLOTS:
  void valueChangedSlot(int);

  void updateStep();

 private:
  void init();

  void initVar();

  int posToStep(int pos) const;

  bool isNegative() const;

 private:
  Tcl_Interp* interp_ { nullptr };
  QString     varName_;

  bool autoStep_ { true };
  int  step_     { 1 };

  bool tclConnected_ { false };
};

#endif
