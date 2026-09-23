#ifndef CTclObj_H
#define CTclObj_H

#include <tcl/tcl.h>

#include <CVector3D.h>
#include <CMatrix3D.h>
#include <CArray2D.h>

namespace CTclVector3D {

int init(Tcl_Interp *);

bool fromString(const std::string &str, CVector3D &v);

bool isObj(const Tcl_Obj *objPtr);

Tcl_Obj *newObj(const CVector3D &v);
int      getObj(Tcl_Interp *interp, Tcl_Obj *objPtr, CVector3D &v);
int      setObj(Tcl_Interp *interp, Tcl_Obj *objPtr, const CVector3D &v);

}

//---

namespace CTclMatrix3D {

int init(Tcl_Interp *);

bool fromString(const std::string &str, CMatrix3D &m);

bool isObj(const Tcl_Obj *objPtr);

Tcl_Obj *newObj(const CMatrix3D &m);
int      getObj(Tcl_Interp *interp, Tcl_Obj *objPtr, CMatrix3D &m);
int      setObj(Tcl_Interp *interp, Tcl_Obj *objPtr, const CMatrix3D &m);

}

//---

namespace CTclArray2D {

int init(Tcl_Interp *);

bool fromString(const std::string &str, CArray2D<double> &m);

bool isObj(const Tcl_Obj *objPtr);

Tcl_Obj *newObj(const CArray2D<double> &m);
int      getObj(Tcl_Interp *interp, Tcl_Obj *objPtr, CArray2D<double> &m);
int      setObj(Tcl_Interp *interp, Tcl_Obj *objPtr, const CArray2D<double> &m);

}

#endif
