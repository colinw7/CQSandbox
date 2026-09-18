#include <CQTclIntegerSpin.h>
#include <CQTclUtil.h>

#include <QLineEdit>
#include <cmath>

namespace {

char *traceProc(ClientData data, Tcl_Interp *, const char *, const char *, int flags) {
  auto *th = static_cast<CQTclIntegerSpin *>(data);
  assert(th);

  th->handleTrace(flags);

  return nullptr;
}

}

//---

CQTclIntegerSpin::
CQTclIntegerSpin(QWidget *parent, Tcl_Interp *interp, const QString &varName) :
 QSpinBox(parent), interp_(interp), varName_(varName)
{
  init();
}

CQTclIntegerSpin::
CQTclIntegerSpin(Tcl_Interp *interp, const QString &varName) :
 QSpinBox(nullptr), interp_(interp), varName_(varName)
{
  init();
}

void
CQTclIntegerSpin::
init()
{
  setObjectName("integerSpin");

  setRange(-INT_MAX, INT_MAX);

  initVar();

  connect(this, SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlot(int)));

  connect(lineEdit(), SIGNAL(cursorPositionChanged(int, int)), this, SLOT(updateStep()));

  updateStep();
}

void
CQTclIntegerSpin::
setInterp(Tcl_Interp *interp)
{
  assert(! interp_);

  interp_ = interp;

  initVar();
}

void
CQTclIntegerSpin::
setVarName(const QString &s)
{
  if (interp_ && varName_ != "") {
    int flags = TCL_TRACE_READS | TCL_TRACE_WRITES | TCL_TRACE_UNSETS | TCL_GLOBAL_ONLY;

    Tcl_UntraceVar(interp_, varName_.toLatin1().constData(), flags,
                   &traceProc, static_cast<ClientData>(this));

    tclConnected_ = false;
  }

  varName_ = s;

  initVar();
}

void
CQTclIntegerSpin::
initVar()
{
  if (! interp_ || varName_ == "")
    return;

  int value;
  if (! CQTclUtil::getIntVar(interp_, varName_, value))
    value = 0;

  if (value != this->value())
    setValue(value);

  if (! tclConnected_) {
    int flags = TCL_TRACE_READS | TCL_TRACE_WRITES | TCL_TRACE_UNSETS | TCL_GLOBAL_ONLY;

    Tcl_TraceVar(interp_, varName_.toLatin1().constData(), flags,
                 &traceProc, static_cast<ClientData>(this));

    tclConnected_ = true;
  }
}

int
CQTclIntegerSpin::
cursorPosition() const
{
  return lineEdit()->cursorPosition();
}

void
CQTclIntegerSpin::
setCursorPosition(int pos)
{
  lineEdit()->setCursorPosition(pos);
}

void
CQTclIntegerSpin::
valueChangedSlot(int i)
{
  if (interp_ && varName_ != "")
    CQTclUtil::setVar(interp_, varName_, i);

  updateStep();
}

void
CQTclIntegerSpin::
updateStep()
{
  int pos = cursorPosition();

  int s = posToStep(pos);

  if (step() != s) {
    step_ = s;

    Q_EMIT stepChanged(step());
  }
}

int
CQTclIntegerSpin::
posToStep(int pos) const
{
  bool negative = isNegative();

  if (! negative) {
    if (pos < 1)
      pos = 1;
  }
  else {
    if (pos < 2)
      pos = 2;
  }

  int d = text().length() - pos;

  return int(pow(10, d));
}

void
CQTclIntegerSpin::
stepBy(int n)
{
  int v = value();
  int s = step();

  int l   = text().length();
  int pos = cursorPosition();

  bool negative = isNegative();

  if (! negative) {
    if (pos < 1)
      pos = 1;
  }
  else {
    if (pos < 2)
      pos = 2;
  }

  setValue(v + n*s);

  int l1 = text().length();

  int pos1 = l1 - l + pos;

  if (pos1 != pos)
    setCursorPosition(pos1);

  updateStep();
}

bool
CQTclIntegerSpin::
isNegative() const
{
  const QString &text = lineEdit()->text();

  return (text.length() && text[0] == '-');
}

void
CQTclIntegerSpin::
handleTrace(int /*flags*/)
{
  initVar();
}
