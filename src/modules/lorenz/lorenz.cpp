#include <CQSandboxShlib.h>

#include <CLorenzCalc.h>

#include <iostream>

#include <tcl/tcl.h>

#define LorenzImplInst LorenzImpl::getInst()

class LorenzImpl : public CQSandboxShLib::Impl {
 public:
  static LorenzImpl *getInst() {
    static LorenzImpl *s_inst;

    if (! s_inst)
      s_inst = new LorenzImpl;

    return s_inst;
  }

  static int init() {
    // std::cerr << "init\n";

    auto *impl       = getInst();
    auto *lorenz = impl->lorenz();

    return 1;
  }

  static int getValue(Tcl_Interp *interp, const char *name, int argc,
                      Tcl_Obj **argv, Tcl_Obj **res) {
    // std::cerr << "getValue: " << name << "\n";

    auto *impl       = getInst();
    auto *lorenz = impl->lorenz();

    auto name1 = std::string(name);

    if      (name1 == "xmin") {
      *res = Tcl_NewDoubleObj(lorenz->getXMin());
    }
    else if (name1 == "ymin") {
      *res = Tcl_NewDoubleObj(lorenz->getYMin());
    }
    else if (name1 == "zmin") {
      *res = Tcl_NewDoubleObj(lorenz->getZMin());
    }
    else if (name1 == "xmax") {
      *res = Tcl_NewDoubleObj(lorenz->getXMax());
    }
    else if (name1 == "ymax") {
      *res = Tcl_NewDoubleObj(lorenz->getYMax());
    }
    else if (name1 == "zmax") {
      *res = Tcl_NewDoubleObj(lorenz->getZMax());
    }
    else if (name1 == "iteration_start") {
      *res = Tcl_NewIntObj(lorenz->getIterationStart());
    }
    else if (name1 == "iteration_end") {
      *res = Tcl_NewIntObj(lorenz->getIterationEnd());
    }
    else
      return 0;

    return 1;
  }

  static int setValue(Tcl_Interp *interp, const char *name, Tcl_Obj *value,
                      int argc, Tcl_Obj **argv) {
    // std::cerr << "setValue: " << name << " " << value << "\n";

    auto *impl       = getInst();
    auto *lorenz = impl->lorenz();

    auto name1 = std::string(name);

    if      (name1 == "iteration_start") {
      int n;
      if (Tcl_GetIntFromObj(interp, value, &n) != TCL_OK) {
        std::cerr << "Invalid arguments for iteration_start\n";
        return 0;
      }

      lorenz->setIterationStart(n);
    }
    else if (name1 == "iteration_end") {
      int n;
      if (Tcl_GetIntFromObj(interp, value, &n) != TCL_OK) {
        std::cerr << "Invalid arguments for iteration_end\n";
        return 0;
      }

      lorenz->setIterationEnd(n);
    }
    else
      return 0;

    return 1;
  }

  static int exec(Tcl_Interp *interp, const char *op, int argc, Tcl_Obj **argv, Tcl_Obj **res) {
    // std::cerr << "exec: " << op << " " << argc << "\n";

    auto op1 = std::string(op);

    if      (op1 == "calc") {
      auto *impl       = getInst();
      auto *lorenz = impl->lorenz();

      double x, y, z;
      lorenz->nextValue(x, y, z);

      auto *xObj = Tcl_NewDoubleObj(x);
      auto *yObj = Tcl_NewDoubleObj(y);
      auto *zObj = Tcl_NewDoubleObj(z);

      *res = Tcl_NewListObj(0, nullptr);

      Tcl_ListObjAppendElement(interp, *res, xObj);
      Tcl_ListObjAppendElement(interp, *res, yObj);
      Tcl_ListObjAppendElement(interp, *res, zObj);
    }
    else {
      std::cerr << "Invalid op '" << op << "'\n";
      return 0;
    }

    return 1;
  }

  //---

  LorenzImpl() { }

  CQSandboxShLib::InitProc initProc() override { return init; }
  CQSandboxShLib::GetProc  getProc () override { return getValue; }
  CQSandboxShLib::SetProc  setProc () override { return setValue; }
  CQSandboxShLib::ExecProc execProc() override { return exec; }

  CLorenzCalc *lorenz() { return &lorenz_; }

 private:
  CLorenzCalc lorenz_;
};

//---

extern "C" {

int lorenz_init() {
  return (LorenzImpl::getInst()->initProc())();
}

int lorenz_get_value(Tcl_Interp *interp, const char *name,
                         int argc, Tcl_Obj **argv, Tcl_Obj **res) {
  return (LorenzImpl::getInst()->getProc())(interp, name, argc, argv, res);
}

int lorenz_set_value(Tcl_Interp *interp, const char *name, Tcl_Obj *value,
                         int argc, Tcl_Obj **argv) {
  return (LorenzImpl::getInst()->setProc())(interp, name, value, argc, argv);
}

int lorenz_exec(Tcl_Interp *interp, const char *op,
                    int argc, Tcl_Obj **argv, Tcl_Obj **res) {
  return (LorenzImpl::getInst()->execProc())(interp, op, argc, argv, res);
}

}
