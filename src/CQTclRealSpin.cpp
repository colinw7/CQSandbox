#include <CQTclRealSpin.h>
#include <CQTclUtil.h>

#include <QLineEdit>
#include <QKeyEvent>
#include <cmath>

namespace {

char *traceProc(ClientData data, Tcl_Interp *, const char *, const char *, int flags) {
  auto *th = static_cast<CQTclRealSpin *>(data);
  assert(th);

  th->handleTrace(flags);

  return nullptr;
}

}

//---

CQTclRealSpin::
CQTclRealSpin(QWidget *parent, Tcl_Interp *interp, const QString &varName) :
 QDoubleSpinBox(parent), interp_(interp), varName_(varName)
{
  init();
}

CQTclRealSpin::
CQTclRealSpin(Tcl_Interp *interp, const QString &varName) :
 QDoubleSpinBox(nullptr), interp_(interp), varName_(varName)
{
  init();
}

void
CQTclRealSpin::
init()
{
  setObjectName("realSpin");

  setRange(-1E6, 1E6);

  setDecimals(4);

  initVar();

  connect(this, SIGNAL(valueChanged(double)), this, SLOT(valueChangedSlot(double)));

  connect(lineEdit(), SIGNAL(cursorPositionChanged(int, int)), this, SLOT(updateStep()));

  updateStep();

  connect(this, SIGNAL(valueChanged(double)), this, SIGNAL(realValueChanged(double)));
}

void
CQTclRealSpin::
setInterp(Tcl_Interp *interp)
{
  assert(! interp_);

  interp_ = interp;

  initVar();
}

void
CQTclRealSpin::
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
CQTclRealSpin::
initVar()
{
  if (! interp_ || varName_ == "")
    return;

  double value;
  if (! CQTclUtil::getRealVar(interp_, varName_, value))
    value = 0.0;

  if (value != this->value())
    setValue(value);

  if (! tclConnected_) {
    int flags = TCL_TRACE_READS | TCL_TRACE_WRITES | TCL_TRACE_UNSETS | TCL_GLOBAL_ONLY;

    Tcl_TraceVar(interp_, varName_.toLatin1().constData(), flags,
                 &traceProc, static_cast<ClientData>(this));

    tclConnected_ = true;
  }
}

void
CQTclRealSpin::
valueChangedSlot(double r)
{
  if (interp_ && varName_ != "")
    CQTclUtil::setVar(interp_, varName_, r);

  updateStep();
}

void
CQTclRealSpin::
updateStep()
{
  int pos = cursorPosition();

  double s = posToStep(pos);

  if (std::abs(step() - s) >= 1E-6) {
    step_ = s;

    Q_EMIT stepChanged(step());
  }
}

double
CQTclRealSpin::
posToStep(int pos) const
{
  bool negative = isNegative();

  int dotPos = this->dotPos();

  //---

  // if no dot then power is length - pos
  if (dotPos < 0) {
    if (! negative) {
      if (pos < 1)
        pos = 1;
    }
    else {
      if (pos < 2)
        pos = 2;
    }

    int d = text().length() - pos;

    return std::pow(10, d);
  }

  //---

  // dot on right (1)
  if (pos == dotPos)
    return 1;

  //---

  // dot on left (0.1)
  if (pos == dotPos + 1)
    return 0.1;

  if (! negative) {
    if (pos < 1)
      pos = 1;
  }
  else {
    if (pos < 2)
      pos = 2;
  }

  if (pos > dotPos)
    --pos;

  return std::pow(10, dotPos - pos);
}

void
CQTclRealSpin::
stepBy(int n)
{
  lineEdit()->deselect();

  //---

  double v = value();
  double s = step();

  int pos    = cursorPosition();
  int dotPos = this->dotPos();

  if (dotPos < 0)
    dotPos = lineEdit()->text().length();

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

  int dotPos1 = this->dotPos();

  int pos1 = dotPos1 - dotPos + pos;

  if (pos1 != pos)
    setCursorPosition(pos1);

  updateStep();
}

int
CQTclRealSpin::
cursorPosition() const
{
  return lineEdit()->cursorPosition();
}

void
CQTclRealSpin::
setCursorPosition(int pos)
{
  lineEdit()->setCursorPosition(pos);
}

bool
CQTclRealSpin::
isNegative() const
{
  const auto &text = lineEdit()->text();

  return (text.length() && text[0] == '-');
}

int
CQTclRealSpin::
dotPos() const
{
  const QString &text = lineEdit()->text();

  for (int i = 0; i < text.length(); ++i)
    if (text[i] == '.')
      return i;

  return -1;
}

void
CQTclRealSpin::
setPlaceholderText(const QString &str)
{
  lineEdit()->setPlaceholderText(str);
}

bool
CQTclRealSpin::
event(QEvent *event)
{
  switch (event->type()) {
    case QEvent::KeyPress: {
      auto *ke = static_cast<QKeyEvent *>(event);
      Q_EMIT keyPress(ke->key(), ke->modifiers());
      break;
    }
    default:
      break;
  }

  return QDoubleSpinBox::event(event);
}

void
CQTclRealSpin::
handleTrace(int /*flags*/)
{
  initVar();
}
