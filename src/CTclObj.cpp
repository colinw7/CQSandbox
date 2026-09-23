#include <CTclObj.h>

#include <cstring>

namespace CTclVector3D {

struct Data {
  double x { 0.0 };
  double y { 0.0 };
  double z { 0.0 };
};

// Forward declarations of our Type methods
static void FreeIntRepProc  (Tcl_Obj *objPtr);
static void DupPointRepProc (Tcl_Obj *objPtr, Tcl_Obj *copyPtr);
static void UpdateStringProc(Tcl_Obj *objPtr);
static int  SetFromAnyProc  (Tcl_Interp *interp, Tcl_Obj *objPtr);

// 2. Define the Tcl_ObjType structure
static const Tcl_ObjType s_objType = {
  "Vector3D",       // name
  FreeIntRepProc,   // freeIntRepProc
  DupPointRepProc,  // dupIntRepProc
  UpdateStringProc, // updateStringProc
  SetFromAnyProc    // setFromAnyProc (can be nullptr if not needed)
};

// Free memory when the Tcl_Obj refCount drops to 0
static void FreeIntRepProc(Tcl_Obj *objPtr) {
  auto *data = static_cast<Data *>(objPtr->internalRep.otherValuePtr);
  if (data) {
    ckfree(data);
  }
  objPtr->internalRep.otherValuePtr = nullptr;
  objPtr->typePtr                   = nullptr;
}

// Duplicate the object when it is copied (e.g., set to a new variable)
static void DupPointRepProc(Tcl_Obj *objPtr, Tcl_Obj *copyPtr) {
  auto *srcData  = static_cast<Data *>(objPtr->internalRep.otherValuePtr);
  auto *copyData = static_cast<Data *>(ckalloc(sizeof(Data)));

  copyData->x = srcData->x;
  copyData->y = srcData->y;
  copyData->z = srcData->z;

  copyPtr->internalRep.otherValuePtr = copyData;
  copyPtr->typePtr                   = &s_objType;
}

// Generate the string representation (e.g., when you use $point in a script)
static void UpdateStringProc(Tcl_Obj *objPtr) {
  auto *data = static_cast<Data *>(objPtr->internalRep.otherValuePtr);
  char buffer[128];
  sprintf(buffer, "[%lf %lf %lf]", data->x, data->y, data->z);

  objPtr->length = strlen(buffer);
  objPtr->bytes = (char *) ckalloc(objPtr->length + 1);
  strcpy(objPtr->bytes, buffer);
}

#if 1
static int SetFromAnyProc(Tcl_Interp *interp, Tcl_Obj *objPtr) {
  int length;
  char *str = Tcl_GetStringFromObj(objPtr, &length);

  CVector3D v;
  if (! fromString(str, v)) {
    if (interp)
      Tcl_SetObjResult(interp, Tcl_NewStringObj("invalid vector format, expected 'x y z'", -1));
    return TCL_ERROR;
  }

  // 4. Free any old internal representation
  if (objPtr->typePtr && objPtr->typePtr->freeIntRepProc) {
    objPtr->typePtr->freeIntRepProc(objPtr);
  }

  // 5. Allocate and assign the new internal representation
  auto *data = static_cast<Data *>(ckalloc(sizeof(Data)));
  data->x = v.getX();
  data->y = v.getY();
  data->z = v.getZ();

  objPtr->internalRep.otherValuePtr = data;
  objPtr->typePtr                   = &s_objType; // Update type

  return TCL_OK;
}
#endif

//---

bool fromString(const std::string &str, CVector3D &v) {
  auto splitList = [](const std::string &sstr, std::vector<std::string> &strs) {
    int    argc;
    char **argv;

    int rc = Tcl_SplitList(nullptr, sstr.c_str(), &argc, const_cast<const char ***>(&argv));
    if (rc != TCL_OK) return false;

    for (int i = 0; i < argc; ++i)
      strs.push_back(std::string(argv[i]));

    Tcl_Free(reinterpret_cast<char *>(argv));

    return true;
  };

  std::vector<std::string> strs;
  splitList(str, strs);

  auto stringToReal = [](const std::string &rstr, double &r) {
    try {
      r = std::stod(rstr);
      return true;
    }
    catch (...) {
      return false;
    }
  };

  // Parse the string "x y z"
  if (strs.size() != 3)
    return false;

  double x, y, z;
  if (! stringToReal(strs[0], x) || ! stringToReal(strs[1], y) || ! stringToReal(strs[2], z))
    return false;

  v = CVector3D(x, y, z);

  return true;
}

//---

bool isObj(const Tcl_Obj *obj) {
  return (obj->typePtr == &s_objType);
}

//---

Tcl_Obj *newObj(const CVector3D &v) {
  auto *objPtr = Tcl_NewObj(); // Creates untyped object

  objPtr->internalRep.otherValuePtr = nullptr;
  objPtr->typePtr                   = nullptr;

  // Allocate and populate internal representation
  auto *data = static_cast<Data *>(ckalloc(sizeof(Data)));
  data->x = v.getX();
  data->y = v.getY();
  data->z = v.getZ();

  // Free any old internal rep (none here), set the type and pointer
  objPtr->internalRep.otherValuePtr = data;
  objPtr->typePtr                   = &s_objType;

  // Invalidate string so UpdateStringProc gets called when needed
  Tcl_InvalidateStringRep(objPtr);
  return objPtr;
}

int getObj(Tcl_Interp *interp, Tcl_Obj *objPtr, CVector3D &v) {
  // Check if the object is already of our type
  if (! isObj(objPtr)) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("Expected a vector object", -1));
    return TCL_ERROR;
  }

  auto *data = static_cast<Data *>(objPtr->internalRep.otherValuePtr);

  v = CVector3D(data->x, data->y, data->z);

  return TCL_OK;
}

int setObj(Tcl_Interp *interp, Tcl_Obj *objPtr, const CVector3D &v) {
  // Check if the object is already of our type
  if (! isObj(objPtr)) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("Expected a vector object", -1));
    return TCL_ERROR;
  }

  auto *data = static_cast<Data *>(objPtr->internalRep.otherValuePtr);

  data->x = v.getX();
  data->y = v.getY();
  data->z = v.getZ();

  return TCL_OK;
}

//---

int init(Tcl_Interp *) {
  Tcl_RegisterObjType(&s_objType);

  // Tcl_CreateObjCommand(interp, "mycommand", ...);

  return TCL_OK;
}

}

//------

namespace CTclMatrix3D {

struct Data {
  // row major
  double m[16];
};

// Forward declarations of our Type methods
static void FreeIntRepProc  (Tcl_Obj *objPtr);
static void DupPointRepProc (Tcl_Obj *objPtr, Tcl_Obj *copyPtr);
static void UpdateStringProc(Tcl_Obj *objPtr);
static int  SetFromAnyProc  (Tcl_Interp *interp, Tcl_Obj *objPtr);

// 2. Define the Tcl_ObjType structure
static const Tcl_ObjType s_objType = {
  "Matrix3D",       // name
  FreeIntRepProc,   // freeIntRepProc
  DupPointRepProc,  // dupIntRepProc
  UpdateStringProc, // updateStringProc
  SetFromAnyProc    // setFromAnyProc (can be nullptr if not needed)
};

// Free memory when the Tcl_Obj refCount drops to 0
static void FreeIntRepProc(Tcl_Obj *objPtr) {
  auto *data = static_cast<Data *>(objPtr->internalRep.otherValuePtr);
  if (data) {
    ckfree(data);
  }
  objPtr->internalRep.otherValuePtr = nullptr;
  objPtr->typePtr                   = nullptr;
}

// Duplicate the object when it is copied (e.g., set to a new variable)
static void DupPointRepProc(Tcl_Obj *objPtr, Tcl_Obj *copyPtr) {
  auto *srcData  = static_cast<Data *>(objPtr->internalRep.otherValuePtr);
  auto *copyData = static_cast<Data *>(ckalloc(sizeof(Data)));

  for (uint i = 0; i < 16; ++i)
    copyData->m[i] = srcData->m[i];

  copyPtr->internalRep.otherValuePtr = copyData;
  copyPtr->typePtr                   = &s_objType;
}

// Generate the string representation (e.g., when you use $point in a script)
static void UpdateStringProc(Tcl_Obj *objPtr) {
  auto *data = static_cast<Data *>(objPtr->internalRep.otherValuePtr);
  char buffer[1024];
  sprintf(buffer, "[%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf]",
          data->m[ 0], data->m[ 1], data->m[ 2], data->m[ 3],
          data->m[ 4], data->m[ 5], data->m[ 6], data->m[ 7],
          data->m[ 8], data->m[ 9], data->m[10], data->m[11],
          data->m[12], data->m[13], data->m[14], data->m[15]);

  objPtr->length = strlen(buffer);
  objPtr->bytes = (char *) ckalloc(objPtr->length + 1);
  strcpy(objPtr->bytes, buffer);
}

#if 1
static int SetFromAnyProc(Tcl_Interp *interp, Tcl_Obj *objPtr) {
  int length;
  char *str = Tcl_GetStringFromObj(objPtr, &length);

  CMatrix3D m;
  if (! fromString(str, m)) {
    if (interp)
      Tcl_SetObjResult(interp, Tcl_NewStringObj("invalid matrix format, expected 'x y z'", -1));
    return TCL_ERROR;
  }

  // 4. Free any old internal representation
  if (objPtr->typePtr && objPtr->typePtr->freeIntRepProc) {
    objPtr->typePtr->freeIntRepProc(objPtr);
  }

  // 5. Allocate and assign the new internal representation
  auto *data = static_cast<Data *>(ckalloc(sizeof(Data)));

  m.getValues(data->m, 16);

  objPtr->internalRep.otherValuePtr = data;
  objPtr->typePtr                   = &s_objType; // Update type

  return TCL_OK;
}
#endif

//---

bool fromString(const std::string &str, CMatrix3D &m) {
  auto splitList = [](const std::string &sstr, std::vector<std::string> &strs) {
    int    argc;
    char **argv;

    int rc = Tcl_SplitList(nullptr, sstr.c_str(), &argc, const_cast<const char ***>(&argv));
    if (rc != TCL_OK) return false;

    for (int i = 0; i < argc; ++i)
      strs.push_back(std::string(argv[i]));

    Tcl_Free(reinterpret_cast<char *>(argv));

    return true;
  };

  std::vector<std::string> strs;
  splitList(str, strs);

  auto stringToReal = [](const std::string &rstr, double &r) {
    try {
      r = std::stod(rstr);
      return true;
    }
    catch (...) {
      return false;
    }
  };

  // Parse the string "m00 m01 m02 m03 m10 m11 m12 m13 m20 m21 m22 m23 m30 m31 m32 m33"

  if (strs.size() != 16)
    return false;

  double mm[16];

  for (uint i = 0; i < 16; ++i) {
    if (! stringToReal(strs[i], mm[i]))
      return false;
  }

  m = CMatrix3D(mm, 16);

  return true;
}

//---

bool isObj(const Tcl_Obj *obj) {
  return (obj->typePtr == &s_objType);
}

//---

Tcl_Obj *newObj(const CMatrix3D &m) {
  auto *objPtr = Tcl_NewObj(); // Creates untyped object

  objPtr->internalRep.otherValuePtr = nullptr;
  objPtr->typePtr                   = nullptr;

  // Allocate and populate internal representation
  auto *data = static_cast<Data *>(ckalloc(sizeof(Data)));

  m.getValues(data->m, 16);

  // Free any old internal rep (none here), set the type and pointer
  objPtr->internalRep.otherValuePtr = data;
  objPtr->typePtr                   = &s_objType;

  // Invalidate string so UpdateStringProc gets called when needed
  Tcl_InvalidateStringRep(objPtr);
  return objPtr;
}

int getObj(Tcl_Interp *interp, Tcl_Obj *objPtr, CMatrix3D &m) {
  // Check if the object is already of our type
  if (! isObj(objPtr)) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("Expected a matrix object", -1));
    return TCL_ERROR;
  }

  auto *data = static_cast<Data *>(objPtr->internalRep.otherValuePtr);

  m = CMatrix3D(data->m, 16);

  return TCL_OK;
}

int setObj(Tcl_Interp *interp, Tcl_Obj *objPtr, const CMatrix3D &m) {
  // Check if the object is already of our type
  if (! isObj(objPtr)) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("Expected a matrix object", -1));
    return TCL_ERROR;
  }

  auto *data = static_cast<Data *>(objPtr->internalRep.otherValuePtr);

  m.getValues(data->m, 16);

  return TCL_OK;
}

//---

int init(Tcl_Interp *) {
  Tcl_RegisterObjType(&s_objType);

  // Tcl_CreateObjCommand(interp, "mycommand", ...);

  return TCL_OK;
}

}

//------

namespace CTclArray2D {

struct Data {
  std::vector<double>* data { nullptr };
  size_t               d1   { 0 };
  size_t               d2   { 0 };
};

// Forward declarations of our Type methods
static void FreeIntRepProc  (Tcl_Obj *objPtr);
static void DupPointRepProc (Tcl_Obj *objPtr, Tcl_Obj *copyPtr);
static void UpdateStringProc(Tcl_Obj *objPtr);
static int  SetFromAnyProc  (Tcl_Interp *interp, Tcl_Obj *objPtr);

// 2. Define the Tcl_ObjType structure
static const Tcl_ObjType s_objType = {
  "Array2D",        // name
  FreeIntRepProc,   // freeIntRepProc
  DupPointRepProc,  // dupIntRepProc
  UpdateStringProc, // updateStringProc
  SetFromAnyProc    // setFromAnyProc (can be nullptr if not needed)
};

// Free memory when the Tcl_Obj refCount drops to 0
static void FreeIntRepProc(Tcl_Obj *objPtr) {
  auto *data = static_cast<Data *>(objPtr->internalRep.otherValuePtr);
  if (data) {
    ckfree(data);
  }
  objPtr->internalRep.otherValuePtr = nullptr;
  objPtr->typePtr                   = nullptr;
}

// Duplicate the object when it is copied (e.g., set to a new variable)
static void DupPointRepProc(Tcl_Obj *objPtr, Tcl_Obj *copyPtr) {
  auto *srcData  = static_cast<Data *>(objPtr->internalRep.otherValuePtr);
  auto *copyData = static_cast<Data *>(ckalloc(sizeof(Data)));

  size_t d1 = srcData->d1*srcData->d2;
  size_t d2 = copyData->d1*copyData->d2;

  size_t i = 0;

  for ( ; i < std::min(d1, d2); ++i)
    (*copyData->data)[i] = (*srcData->data)[i];

  for ( ; i < d2; ++i)
    (*copyData->data)[i] = 0;

  copyPtr->internalRep.otherValuePtr = copyData;
  copyPtr->typePtr                   = &s_objType;
}

// Generate the string representation (e.g., when you use $point in a script)
static void UpdateStringProc(Tcl_Obj *objPtr) {
  auto *data = static_cast<Data *>(objPtr->internalRep.otherValuePtr);
  std::string buffer;
  buffer += "{";
  size_t k = 0;
  for (size_t i = 0; i < data->d1; ++i) {
    if (i > 0) buffer += " ";
    buffer += "{";
    for (size_t j = 0; j < data->d2; ++j, ++k) {
      if (j > 0) buffer += " ";
      buffer += std::to_string((*data->data)[k]);
    }
    buffer += "}";
  }
  buffer += "}";

  objPtr->length = buffer.size();
  objPtr->bytes = (char *) ckalloc(objPtr->length + 1);
  strcpy(objPtr->bytes, &buffer[0]);
}

#if 1
static int SetFromAnyProc(Tcl_Interp *interp, Tcl_Obj *objPtr) {
  int length;
  char *str = Tcl_GetStringFromObj(objPtr, &length);

  CArray2D<double> d;
  if (! fromString(str, d)) {
    if (interp)
      Tcl_SetObjResult(interp, Tcl_NewStringObj("invalid array format, expected 'x y z'", -1));
    return TCL_ERROR;
  }

  // 4. Free any old internal representation
  if (objPtr->typePtr && objPtr->typePtr->freeIntRepProc) {
    objPtr->typePtr->freeIntRepProc(objPtr);
  }

  // 5. Allocate and assign the new internal representation
  auto *data = static_cast<Data *>(ckalloc(sizeof(Data)));

  data->d1 = d.dim(0);
  data->d2 = d.dim(1);

  data->data->resize(data->d1*data->d2);

  objPtr->internalRep.otherValuePtr = data;
  objPtr->typePtr                   = &s_objType; // Update type

  return TCL_OK;
}
#endif

//---

bool fromString(const std::string &str, CArray2D<double> &d) {
  auto splitList = [](const std::string &sstr, std::vector<std::vector<std::string>> &strs) {
    int    argc;
    char **argv;

    int rc = Tcl_SplitList(nullptr, sstr.c_str(), &argc, const_cast<const char ***>(&argv));
    if (rc != TCL_OK) return false;

    for (int i = 0; i < argc; ++i) {
      std::vector<std::string> strs1;

      int    argc1;
      char **argv1;

      int rc1 = Tcl_SplitList(nullptr, argv[i], &argc1, const_cast<const char ***>(&argv1));
      if (rc1 != TCL_OK) return false;

      for (int j = 0; j < argc1; ++j)
        strs1.push_back(std::string(argv1[j]));

      Tcl_Free(reinterpret_cast<char *>(argv1));

      strs.push_back(strs1);
    }

    Tcl_Free(reinterpret_cast<char *>(argv));

    return true;
  };

  std::vector<std::vector<std::string>> strs;
  splitList(str, strs);

  auto stringToReal = [](const std::string &rstr, double &r) {
    try {
      r = std::stod(rstr);
      return true;
    }
    catch (...) {
      return false;
    }
  };

  Data d1;

  d1.d1 = strs.size();

  uint k = 0;

  for (uint i = 0; i < d1.d1; ++i) {
    if (i == 0)
      d1.d2 = strs[i].size();
    else {
      if (strs[i].size() != d1.d2)
        return false;
    }

    if (! d1.data) {
      d1.data = new std::vector<double>;

      d1.data->resize(d1.d1*d1.d2);
    }

    for (uint j = 0; j < d1.d2; ++j, ++k) {
      if (! stringToReal(strs[i][j], (*d1.data)[k]))
        return false;
    }
  }

  d = CArray2D<double>(&(*d1.data)[0], d1.d1, d1.d2);

  return true;
}

//---

bool isObj(const Tcl_Obj *obj) {
  return (obj->typePtr == &s_objType);
}

//---

Tcl_Obj *newObj(const CArray2D<double> &d) {
  auto *objPtr = Tcl_NewObj(); // Creates untyped object

  objPtr->internalRep.otherValuePtr = nullptr;
  objPtr->typePtr                   = nullptr;

  // Allocate and populate internal representation
  auto *data = static_cast<Data *>(ckalloc(sizeof(Data)));

  data->d1   = d.dim(0);
  data->d2   = d.dim(1);
  data->data = new std::vector<double>;

  data->data->resize(data->d1*data->d2);

  memcpy(&(*data->data)[0], d.data(), data->d1*data->d2*sizeof(double));

  // Free any old internal rep (none here), set the type and pointer
  objPtr->internalRep.otherValuePtr = data;
  objPtr->typePtr                   = &s_objType;

  // Invalidate string so UpdateStringProc gets called when needed
  Tcl_InvalidateStringRep(objPtr);
  return objPtr;
}

int getObj(Tcl_Interp *interp, Tcl_Obj *objPtr, CArray2D<double> &d) {
  // Check if the object is already of our type
  if (! isObj(objPtr)) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("Expected an array object", -1));
    return TCL_ERROR;
  }

  auto *data = static_cast<Data *>(objPtr->internalRep.otherValuePtr);

  d = CArray2D<double>(&(*data->data)[0], data->d1, data->d2);

  return TCL_OK;
}

int setObj(Tcl_Interp *interp, Tcl_Obj *objPtr, const CArray2D<double> &d) {
  // Check if the object is already of our type
  if (! isObj(objPtr)) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("Expected an array object", -1));
    return TCL_ERROR;
  }

  auto *data = static_cast<Data *>(objPtr->internalRep.otherValuePtr);

  data->d1 = d.dim(0);
  data->d2 = d.dim(1);

  data->data->resize(data->d1*data->d2);

  memcpy(&(*data->data)[0], &d.data()[0], data->d1*data->d2*sizeof(double));

  return TCL_OK;
}

//---

int init(Tcl_Interp *) {
  Tcl_RegisterObjType(&s_objType);

  // Tcl_CreateObjCommand(interp, "mycommand", ...);

  return TCL_OK;
}

}
