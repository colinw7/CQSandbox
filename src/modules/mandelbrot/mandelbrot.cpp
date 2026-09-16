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
    // std::cerr << "init\n";

    auto *impl       = getInst();
    auto *mandelbrot = impl->mandelbrot();

    impl->initCalc();

    return 1;
  }

  static int getValue(Tcl_Interp *interp, const char *name, int argc,
                      Tcl_Obj **argv, Tcl_Obj **res) {
    // std::cerr << "getValue: " << name << "\n";

    auto *impl       = getInst();
    auto *mandelbrot = impl->mandelbrot();

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
    else if (name1 == "max_iterations") {
      *res = Tcl_NewIntObj(impl->max_iterations_);
    }
    else if (name1 == "user_x") {
      if (argc < 1) {
        std::cerr << "Missing arguments for user_x\n";
        return 0;
      }

      int ix;
      if (Tcl_GetIntFromObj(interp, argv[0], &ix) != TCL_OK) {
        std::cerr << "Invalid arguments for user_x\n";
        return 0;
      }

      auto x = mandelbrot->pixelXToUser(ix);

      *res = Tcl_NewDoubleObj(x);
    }
    else if (name1 == "user_y") {
      if (argc < 1) {
        std::cerr << "Missing arguments for user_y\n";
        return 0;
      }

      int iy;
      if (Tcl_GetIntFromObj(interp, argv[0], &iy) != TCL_OK) {
        std::cerr << "Invalid arguments for user_y\n";
        return 0;
      }

      auto y = mandelbrot->pixelYToUser(iy);

      *res = Tcl_NewDoubleObj(y);
    }
    else
      return 0;

    return 1;
  }

  static int setValue(Tcl_Interp *interp, const char *name, Tcl_Obj *value,
                      int argc, Tcl_Obj **argv) {
    // std::cerr << "setValue: " << name << " " << value << "\n";

    auto *impl       = getInst();
    auto *mandelbrot = impl->mandelbrot();

    auto name1 = std::string(name);

    if      (name1 == "max_iterations") {
      int n;
      if (Tcl_GetIntFromObj(interp, value, &n) != TCL_OK) {
        std::cerr << "Invalid arguments for max_iterations\n";
        return 0;
      }

      impl->max_iterations_ = n;

      impl->initCalc();
    }
    else if (name1 == "pixel_xmin") {
      int n;
      if (Tcl_GetIntFromObj(interp, value, &n) != TCL_OK) {
        std::cerr << "Invalid arguments for pixel_xmin\n";
        return 0;
      }

      impl->pixel_xmin_ = n;

      impl->initCalc();
    }
    else if (name1 == "pixel_ymin") {
      int n;
      if (Tcl_GetIntFromObj(interp, value, &n) != TCL_OK) {
        std::cerr << "Invalid arguments for pixel_ymin\n";
        return 0;
      }

      impl->pixel_ymin_ = n;

      impl->initCalc();
    }
    else if (name1 == "pixel_xmax") {
      int n;
      if (Tcl_GetIntFromObj(interp, value, &n) != TCL_OK) {
        std::cerr << "Invalid arguments for pixel_xmax\n";
        return 0;
      }

      impl->pixel_xmax_ = n;

      impl->initCalc();
    }
    else if (name1 == "pixel_ymax") {
      int n;
      if (Tcl_GetIntFromObj(interp, value, &n) != TCL_OK) {
        std::cerr << "Invalid arguments for pixel_ymax\n";
        return 0;
      }

      impl->pixel_ymax_ = n;

      impl->initCalc();
    }
    else if (name1 == "xmin") {
      double r;
      if (Tcl_GetDoubleFromObj(interp, value, &r) != TCL_OK) {
        std::cerr << "Invalid arguments for xmin\n";
        return 0;
      }

      impl->xmin_ = r;

      impl->initCalc();
    }
    else if (name1 == "ymin") {
      double r;
      if (Tcl_GetDoubleFromObj(interp, value, &r) != TCL_OK) {
        std::cerr << "Invalid arguments for ymin\n";
        return 0;
      }

      impl->ymin_ = r;

      impl->initCalc();
    }
    else if (name1 == "xmax") {
      double r;
      if (Tcl_GetDoubleFromObj(interp, value, &r) != TCL_OK) {
        std::cerr << "Invalid arguments for xmax\n";
        return 0;
      }

      impl->xmax_ = r;

      impl->initCalc();
    }
    else if (name1 == "ymax") {
      double r;
      if (Tcl_GetDoubleFromObj(interp, value, &r) != TCL_OK) {
        std::cerr << "Invalid arguments for ymax\n";
        return 0;
      }

      impl->ymax_ = r;

      impl->initCalc();
    }
    else
      return 0;

    return 1;
  }

  static int exec(Tcl_Interp *interp, const char *op, int argc, Tcl_Obj **argv, Tcl_Obj **res) {
    // std::cerr << "exec: " << op << " " << argc << "\n";

    auto op1 = std::string(op);

    if      (op1 == "calc") {
      if (argc < 2) {
        std::cerr << "Missing arguments for calc\n";
        return 0;
      }

      double x, y;
      if (Tcl_GetDoubleFromObj(interp, argv[0], &x) != TCL_OK ||
          Tcl_GetDoubleFromObj(interp, argv[1], &y) != TCL_OK) {
        std::cerr << "Invalid arguments for calc\n";
        return 0;
      }

      auto *impl       = getInst();
      auto *mandelbrot = impl->mandelbrot();

      auto n = mandelbrot->calc(x, y, impl->max_iterations_);

      *res = Tcl_NewIntObj(n);
    }
    else {
      std::cerr << "Invalid op '" << op << "'\n";
      return 0;
    }

    return 1;
  }

  //---

  MandelbrotImpl() { }

  CQSandboxShLib::InitProc initProc() override { return init; }
  CQSandboxShLib::GetProc  getProc () override { return getValue; }
  CQSandboxShLib::SetProc  setProc () override { return setValue; }
  CQSandboxShLib::ExecProc execProc() override { return exec; }

  CMandelbrot *mandelbrot() { return &mandelbrot_; }

  void initCalc() {
    mandelbrot_.initCalc(pixel_xmin_, pixel_ymin_, pixel_xmax_, pixel_ymax_,
                         xmin_, ymin_, xmax_, ymax_, max_iterations_);
  }

 private:
  CMandelbrot mandelbrot_;

  CColorRange colorRange_;

  uint iterations_     { 256 };
  uint max_iterations_ { 256 };

  int pixel_xmin_ { 0 };
  int pixel_ymin_ { 0 };
  int pixel_xmax_ { 100 };
  int pixel_ymax_ { 100 };;

  double xmin_ { -2.0 };
  double ymin_ { -1.2 };
  double xmax_ {  1.2 };
  double ymax_ {  1.2 };

  std::vector<int> colors_;
};

//---

extern "C" {

int mandelbrot_init() {
  return (MandelbrotImpl::getInst()->initProc())();
}

int mandelbrot_get_value(Tcl_Interp *interp, const char *name,
                         int argc, Tcl_Obj **argv, Tcl_Obj **res) {
  return (MandelbrotImpl::getInst()->getProc())(interp, name, argc, argv, res);
}

int mandelbrot_set_value(Tcl_Interp *interp, const char *name, Tcl_Obj *value,
                         int argc, Tcl_Obj **argv) {
  return (MandelbrotImpl::getInst()->setProc())(interp, name, value, argc, argv);
}

int mandelbrot_exec(Tcl_Interp *interp, const char *op,
                    int argc, Tcl_Obj **argv, Tcl_Obj **res) {
  return (MandelbrotImpl::getInst()->execProc())(interp, op, argc, argv, res);
}

}
