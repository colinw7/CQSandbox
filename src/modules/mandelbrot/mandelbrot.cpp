#include <CQSandboxShlib.h>

#include <CMandelbrot.h>
#include <CDisplayRange2D.h>
#include <CColorRange.h>

#include <iostream>

#include <tcl/tcl.h>

#define MandelbrotImplInst MandelbrotImpl::getInst()

class MandelbrotImpl : public CQSandboxShLib::Impl {
 public:
  static MandelbrotImpl *getInst() {
    static MandelbrotImpl *s_inst;

    if (! s_inst)
      s_inst = new MandelbrotImpl;

    return s_inst;
  }

  static int init() {
    std::cerr << "init\n";
    return 1;
  }

  static int getValue(Tcl_Interp *tcl, const char *name, int argc,
                      Tcl_Obj **argv, Tcl_Obj **res) {
    auto *mandelbrot = getInst()->mandelbrot();

    std::cerr << "getValue: " << name << "\n";

    auto name1 = std::string(name);
    if      (name1 == "xmin") {
      *res = Tcl_NewDoubleObj(mandelbrot->getXMin());
    }
    else if (name1 == "ymin") {
      *res = Tcl_NewDoubleObj(mandelbrot->getYMin());
    }
    else if (name1 == "xmax") {
      *res = Tcl_NewDoubleObj(mandelbrot->getXMax());
    }
    else if (name1 == "ymax") {
      *res = Tcl_NewDoubleObj(mandelbrot->getYMax());
    }
    else
      return 0;

    return 1;
  }

  static int setValue(Tcl_Interp *tcl, const char *name, const char *value,
                      int argc, Tcl_Obj **argv) {
    std::cerr << "setValue: " << name << " " << value << "\n";
    return 1;
  }

  static int exec(Tcl_Interp *tcl, const char *op, int argc, Tcl_Obj **argv, Tcl_Obj **res) {
    std::cerr << "exec: " << op << "\n";
    return 1;
  }

  //---

  MandelbrotImpl() { }

  CQSandboxShLib::InitProc initProc() override { return init; }
  CQSandboxShLib::GetProc  getProc () override { return getValue; }
  CQSandboxShLib::SetProc  setProc () override { return setValue; }
  CQSandboxShLib::ExecProc execProc() override { return exec; }

  CMandelbrot *mandelbrot() { return &mandelbrot_; }

 private:
  CMandelbrot      mandelbrot_;
  CColorRange      colorRange_;
  int              xsize_      { 0 };
  int              ysize_      { 0 };
  uint             iterations_ { 256 };
  std::vector<int> colors_;
};

//---

extern "C" {

int mandelbrot_init() {
  return (MandelbrotImpl::getInst()->initProc())();
}

int mandelbrot_get_value(Tcl_Interp *tcl, const char *name,
                         int argc, Tcl_Obj **argv, Tcl_Obj **res) {
  return (MandelbrotImpl::getInst()->getProc())(tcl, name, argc, argv, res);
}

int mandelbrot_set_value(Tcl_Interp *tcl, const char *name, const char *value,
                         int argc, Tcl_Obj **argv) {
  return (MandelbrotImpl::getInst()->setProc())(tcl, name, value, argc, argv);
}

int mandelbrot_exec(Tcl_Interp *tcl, const char *op,
                    int argc, Tcl_Obj **argv, Tcl_Obj **res) {
  return (MandelbrotImpl::getInst()->execProc())(tcl, op, argc, argv, res);
}

}
