#ifndef CQTclRealSpin_H
#define CQTclRealSpin_H

#include <QDoubleSpinBox>

struct Tcl_Interp;

/*!
 * \brief Extension of QDoubleSpinBox to allow spin box to increment the digit at the
 * current cursor position. Updates to/from specied tcl variable
 */
class CQTclRealSpin : public QDoubleSpinBox {
  Q_OBJECT

  Q_PROPERTY(QString varName  READ varName    WRITE setVarName)
  Q_PROPERTY(bool    autoStep READ isAutoStep WRITE setAutoStep)
  Q_PROPERTY(double  step     READ step)

 public:
  CQTclRealSpin(QWidget *parent, Tcl_Interp *interp=nullptr, const QString &varName="");
  CQTclRealSpin(Tcl_Interp *interp=nullptr, const QString &varName="");

  virtual ~CQTclRealSpin() { }

  const Tcl_Interp *interp() const { return interp_; }
  void setInterp(Tcl_Interp *p);

  const QString &varName() const { return varName_; }
  void setVarName(const QString &s);

  //! get/set auto step
  bool isAutoStep() const { return autoStep_; }
  void setAutoStep(bool b) { autoStep_ = b; }

  //! get current step
  double step() const { return step_; }

  //! step by n
  void stepBy(int n) override;

  //! get/set cursor position
  int cursorPosition() const;
  void setCursorPosition(int pos);

  void setPlaceholderText(const QString &str);

  bool event(QEvent *event) override;

  void handleTrace(int /*flags*/);

 Q_SIGNALS:
  void realValueChanged(double);

  //! emitted when step changed
  void stepChanged(double);

  void keyPress(int key, int modifiers);

 private Q_SLOTS:
  void valueChangedSlot(double);

  //! update step from cursor position
  void updateStep();

 private:
  //! init widget with current value
  void init();

  void initVar();

  //! calc step for cursor position
  double posToStep(int pos) const;

  //! check if value is negative
  bool isNegative() const;

  //! find position of decimal point
  int dotPos() const;

 private:
  Tcl_Interp* interp_ { nullptr };
  QString     varName_;

  bool   autoStep_ { true }; //!< support increment using cursor position
  double step_     { 1 };    //!< current step increment

  bool tclConnected_ { false };
};

#endif
