#ifndef CQSandboxShlib_H
#define CQSandboxShlib_H

#include <tcl/tcl.h>

namespace CQSandboxShLib {

using InitProc = int (*)();

using GetProc = int (*)(Tcl_Interp *interp, const char *name,
                        int argc, Tcl_Obj **argv, Tcl_Obj **res);
using SetProc = int (*)(Tcl_Interp *interp, const char *name, const char *value,
                        int argc, Tcl_Obj **argv);

using ExecProc = int (*)(Tcl_Interp *interp, const char *op,
                         int argc, Tcl_Obj **argv, Tcl_Obj **res);

class Impl {
 public:
  Impl() { }

  virtual ~Impl() { }

  virtual InitProc initProc() = 0;
  virtual GetProc  getProc () = 0;
  virtual SetProc  setProc () = 0;
  virtual ExecProc execProc() = 0;
};

}

#endif
