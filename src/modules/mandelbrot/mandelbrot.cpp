#include <CMandelbrot.h>
#include <CDisplayRange2D.h>
#include <CColorRange.h>

#include <iostream>

#include <tcl/tcl.h>

class MandelbrotImpl {
 public:
  MandelbrotImpl() { }

  int init() {
    std::cerr << "init\n";
    return 1;
  }

  int getValue(Tcl_Interp *tcl, const char *name, Tcl_Obj **res) {
    std::cerr << "getValue: " << name << "\n";

    auto name1 = std::string(name);
    if      (name1 == "xmin") {
      *res = Tcl_NewDoubleObj(mandelbrot_.getXMin());
    }
    else if (name1 == "ymin") {
      *res = Tcl_NewDoubleObj(mandelbrot_.getYMin());
    }
    else if (name1 == "xmax") {
      *res = Tcl_NewDoubleObj(mandelbrot_.getXMax());
    }
    else if (name1 == "ymax") {
      *res = Tcl_NewDoubleObj(mandelbrot_.getYMax());
    }
    else
      return 0;

    return 1;
  }

  int setValue(Tcl_Interp *tcl, const char *name, const char *value) {
    std::cerr << "setValue: " << name << " " << value << "\n";
    return 1;
  }

  int exec(Tcl_Interp *tcl, const char *op) {
    std::cerr << "exec: " << op << "\n";
    return 1;
  }

 private:
  CMandelbrot      mandelbrot_;
  CColorRange      colorRange_;
  int              xsize_      { 0 };
  int              ysize_      { 0 };
  uint             iterations_ { 256 };
  std::vector<int> colors_;
};

static MandelbrotImpl *s_impl;

//---

extern "C" {

int mandelbrot_init() {
  s_impl = new MandelbrotImpl;

  return s_impl->init();
}

int mandelbrot_get_value(Tcl_Interp *tcl, const char *name, Tcl_Obj **res) {
  return s_impl->getValue(tcl, name, res);
}

int mandelbrot_set_value(Tcl_Interp *tcl, const char *name, const char *value) {
  return s_impl->setValue(tcl, name, value);
}

int mandelbrot_exec(Tcl_Interp *tcl, const char *op) {
  return s_impl->exec(tcl, op);
}

}
