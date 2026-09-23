#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wzero-length-bounds"

#define HAVE_UNISTD_H 1

extern "C" {
#include <tclInt.h>
}

#include <vector>
#include <string>

int
Tcl_InvokeMethod(Tcl_Interp *interp, std::vector<std::string> &argNames,
                 std::vector<Tcl_Obj *> &argValues, const std::string &body,
                 Tcl_Obj **res)
{
  auto *nsPtr = Tcl_GetCurrentNamespace(interp);

  Tcl_CallFrame frame;
  if (Tcl_PushCallFrame(interp, &frame, nsPtr, 0) != TCL_OK)
    return TCL_ERROR;

  for (auto *argValue : argValues)
    Tcl_IncrRefCount(argValue);

  for (uint i = 0; i < argNames.size(); ++i) {
    auto *nameObj = Tcl_NewStringObj(argNames[i].c_str(), argNames[i].size());
    Tcl_IncrRefCount(nameObj);

    Tcl_ObjSetVar2(interp, nameObj, nullptr, argValues[i], 0);

    Tcl_DecrRefCount(nameObj);
  }

  if (Tcl_EvalEx(interp, body.c_str(), body.size(), 0) != TCL_OK)
    return TCL_ERROR;

  for (auto *argValue : argValues)
    Tcl_DecrRefCount(argValue);

  *res = Tcl_GetObjResult(interp);

  Tcl_PopCallFrame(interp);

  return TCL_OK;
}

#pragma GCC diagnostic pop
