#include <CQTclCheckBox.h>
#include <CQTclUtil.h>

namespace {

char *traceProc(ClientData data, Tcl_Interp *, const char *, const char *, int flags) {
  auto *th = static_cast<CQTclCheckBox *>(data);
  assert(th);

  th->handleTrace(flags);

  return nullptr;
}

}

//---

CQTclCheckBox::
CQTclCheckBox(QWidget *parent, Tcl_Interp *interp, const QString &varName) :
 QCheckBox(parent), interp_(interp), varName_(varName)
{
  init();
}

CQTclCheckBox::
CQTclCheckBox(Tcl_Interp *interp, const QString &varName) :
 QCheckBox(nullptr), interp_(interp), varName_(varName)
{
  init();
}

void
CQTclCheckBox::
init()
{
  setObjectName("checkBox");

  initVar();

  connect(this, SIGNAL(stateChanged(int)), this, SLOT(valueChangedSlot(int)));

  connect(this, SIGNAL(stateChanged(int)), this, SIGNAL(valueChanged(int)));
}

void
CQTclCheckBox::
setInterp(Tcl_Interp *interp)
{
  assert(! interp_);

  interp_ = interp;

  initVar();
}

void
CQTclCheckBox::
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
CQTclCheckBox::
initVar()
{
  if (! interp_ || varName_ == "")
    return;

  bool checked;
  if (! CQTclUtil::getBoolVar(interp_, varName_, checked))
    checked = false;

  if (checked != this->isChecked())
    setChecked(checked);

  if (! tclConnected_) {
    int flags = TCL_TRACE_READS | TCL_TRACE_WRITES | TCL_TRACE_UNSETS | TCL_GLOBAL_ONLY;

    Tcl_TraceVar(interp_, varName_.toLatin1().constData(), flags,
                 &traceProc, static_cast<ClientData>(this));

    tclConnected_ = true;
  }
}

void
CQTclCheckBox::
valueChangedSlot(int s)
{
  if (interp_ && varName_ != "")
    CQTclUtil::setVar(interp_, varName_, (s ? 1 : 0));
}

void
CQTclCheckBox::
handleTrace(int /*flags*/)
{
  initVar();
}
