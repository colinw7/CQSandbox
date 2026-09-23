#ifndef CQTclCheckBox_H
#define CQTclCheckBox_H

#include <QCheckBox>

struct Tcl_Interp;

/*!
 * \brief Extension of QDoubleSpinBox to allow spin box to increment the digit at the
 * current cursor position. Updates to/from specied tcl variable
 */
class CQTclCheckBox : public QCheckBox {
  Q_OBJECT

  Q_PROPERTY(QString varName READ varName WRITE setVarName)

 public:
  CQTclCheckBox(QWidget *parent, Tcl_Interp *interp=nullptr, const QString &varName="");
  CQTclCheckBox(Tcl_Interp *interp=nullptr, const QString &varName="");

  virtual ~CQTclCheckBox() { }

  const Tcl_Interp *interp() const { return interp_; }
  void setInterp(Tcl_Interp *p);

  const QString &varName() const { return varName_; }
  void setVarName(const QString &s);

  void handleTrace(int /*flags*/);

 Q_SIGNALS:
  void valueChanged(int);

 private Q_SLOTS:
  void valueChangedSlot(int);

 private:
  //! init widget with current value
  void init();

  void initVar();

 private:
  Tcl_Interp* interp_ { nullptr };
  QString     varName_;

  bool tclConnected_ { false };
};

#endif
