#ifndef CTclUtil_H
#define CTclUtil_H

// TODO: See CTclValue.h

#include <tcl/tcl.h>

#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <cstring>
#include <cassert>

#undef Bool
#undef CursorShape

using Tcl_Size = int;

namespace CTclUtil {

using StringList = std::vector<std::string>;

//---

bool isCompleteLine(const std::string &line);

inline int eval(Tcl_Interp *interp, const char *str) {
  return Tcl_EvalEx(interp, str, -1, 0);
}

//---

inline Tcl_Obj *createStrObj(const std::string &str) {
  return Tcl_NewStringObj(str.c_str(), int(str.size()));
}

inline Tcl_Obj *createIntObj(int i) {
  return Tcl_NewIntObj(i);
}

inline Tcl_Obj *createRealObj(double r) {
  return Tcl_NewDoubleObj(r);
}

inline Tcl_Obj *createIntsObj(Tcl_Interp *interp, const std::vector<int> &ivals) {
  auto *obj = Tcl_NewListObj(0, nullptr);

  for (const auto &i : ivals) {
    auto *iobj = Tcl_NewIntObj(i);

    Tcl_ListObjAppendElement(interp, obj, iobj);
  }

  return obj;
}

inline std::string stringFromObj(const Tcl_Obj *obj) {
  Tcl_Size len = 0;

  char *str = Tcl_GetStringFromObj(const_cast<Tcl_Obj *>(obj), &len);

  return std::string(str, size_t(len));
}

inline void createVar(Tcl_Interp *interp, const std::string &name, const std::string &value) {
  auto *nameObj  = createStrObj(name ); Tcl_IncrRefCount(nameObj);
  auto *valueObj = createStrObj(value);

  Tcl_ObjSetVar2(interp, nameObj, nullptr, valueObj, TCL_GLOBAL_ONLY);

  Tcl_DecrRefCount(nameObj);
}

inline void createVar(Tcl_Interp *interp, const std::string &name, int value) {
  auto *nameObj  = createStrObj(name ); Tcl_IncrRefCount(nameObj);
  auto *valueObj = createIntObj(value);

  Tcl_ObjSetVar2(interp, nameObj, nullptr, valueObj, TCL_GLOBAL_ONLY);

  Tcl_DecrRefCount(nameObj);
}

inline void createVar(Tcl_Interp *interp, const std::string &name, double value) {
  auto *nameObj  = createStrObj(name ); Tcl_IncrRefCount(nameObj);
  auto *valueObj = createRealObj(value);

  Tcl_ObjSetVar2(interp, nameObj, nullptr, valueObj, TCL_GLOBAL_ONLY);

  Tcl_DecrRefCount(nameObj);
}

inline void createVar(Tcl_Interp *interp, const std::string &name, const std::vector<int> &values) {
  auto *nameObj  = createStrObj(name ); Tcl_IncrRefCount(nameObj);
  auto *valueObj = createIntsObj(interp, values);

  Tcl_ObjSetVar2(interp, nameObj, nullptr, valueObj, TCL_GLOBAL_ONLY);

  Tcl_DecrRefCount(nameObj);
}

//---

inline std::string getVar(Tcl_Interp *interp, const std::string &name) {
  auto *nameObj = createStrObj(name); Tcl_IncrRefCount(nameObj);

  auto *obj = Tcl_ObjGetVar2(interp, nameObj, nullptr, TCL_GLOBAL_ONLY);

  Tcl_DecrRefCount(nameObj);

  if (! obj)
    return std::string();

  return stringFromObj(obj);
}

//---

inline StringList getObjArgs(int objc, Tcl_Obj * const *objv) {
  StringList args;

  for (int i = 1; i < objc; ++i)
    args.push_back(stringFromObj(const_cast<Tcl_Obj *>(objv[size_t(i)])));

  return args;
}

//---

inline bool splitList(const std::string &str, StringList &strs) {
  Tcl_Size argc;
  char **  argv;

  int rc = Tcl_SplitList(nullptr, str.c_str(), &argc, const_cast<const char ***>(&argv));
  if (rc != TCL_OK) return false;

  for (int i = 0; i < argc; ++i)
    strs.push_back(std::string(argv[i]));

  Tcl_Free(reinterpret_cast<char *>(argv));

  return true;
}

inline std::string mergeList(const StringList &strs) {
  auto argc = strs.size();

  std::vector<char *> argv;

  argv.resize(argc);

  for (size_t i = 0; i < argc; ++i)
    argv[i] = strdup(strs[i].c_str());

  char *res = Tcl_Merge(int(argc), &argv[0]);

  std::string str(res);

  for (size_t i = 0; i < argc; ++i)
    free(argv[i]);

  Tcl_Free(res);

  return str;
}

//---

inline bool isDomainError(Tcl_Interp *interp, int rc) {
  Tcl_Obj *options = Tcl_GetReturnOptions(interp, rc);

  Tcl_Obj *key = Tcl_NewStringObj("-errorcode", -1);

  Tcl_Obj *errorMsg;
  Tcl_IncrRefCount(key);
  Tcl_DictObjGet(nullptr, options, key, &errorMsg);
  Tcl_DecrRefCount(key);

  auto msg = stringFromObj(errorMsg);

  Tcl_DecrRefCount(options);

  return (msg.size() > 12 && msg.substr(0, 12) == "ARITH DOMAIN");
}

//---

inline std::string errorInfo(Tcl_Interp *interp, int rc) {
  auto *options = Tcl_GetReturnOptions(interp, rc);

  auto *key1 = Tcl_NewStringObj("-errorcode", -1);
  auto *key2 = Tcl_NewStringObj("-errorinfo", -1);

  Tcl_Obj *errorMsg;
  Tcl_IncrRefCount(key1);
  Tcl_DictObjGet(nullptr, options, key1, &errorMsg);
  Tcl_DecrRefCount(key1);

  std::string msg;

  if (errorMsg)
    msg = stringFromObj(errorMsg);

  Tcl_Obj *stackTrace;
  Tcl_IncrRefCount(key2);
  Tcl_DictObjGet(nullptr, options, key2, &stackTrace);
  Tcl_DecrRefCount(key2);

  if (! stackTrace)
    return "";

  auto trace = stringFromObj(stackTrace);

  Tcl_DecrRefCount(options);

  return msg + "\n" + trace;
}

}

//---

class CTcl {
 public:
  using ObjCmdProc = Tcl_ObjCmdProc *;
  using ObjCmdData = ClientData;
  using IntList    = std::vector<int>;
  using RealList   = std::vector<double>;
  using StringList = std::vector<std::string>;
  using Traces     = std::set<std::string>;

  struct EvalData {
    bool        showError  { false };
    bool        showResult { false };
    int         rc { 0 };
    std::string res;
    std::string errMsg;
  };

 public:
  CTcl();

  virtual ~CTcl();

  //---

  Tcl_Interp *interp() const { return interp_; }

  //---

  bool init();

  bool initTk();

#ifdef TIX_SUPPORT
  bool initTix();
#endif

  //---

  // set/get variable
  void createVar(const std::string &name, const std::string &value) {
    CTclUtil::createVar(interp(), name, value);
  }

  void createVar(const std::string &name, int value) {
    CTclUtil::createVar(interp(), name, value);
  }

  void createVar(const std::string &name, double value) {
    CTclUtil::createVar(interp(), name, value);
  }

  void createVar(const std::string &name, const IntList &values) {
    CTclUtil::createVar(interp(), name, values);
  }

  std::string getVar(const std::string &name) const {
    return CTclUtil::getVar(interp(), name);
  }

  //---

  // create command
  Tcl_Command createExprCommand(const std::string &name, ObjCmdProc proc, ObjCmdData data) {
    auto mathName = "tcl::mathfunc::" + name;

    return createObjCommandI(mathName, proc, data);
  }

  Tcl_Command createObjCommand(const std::string &name, ObjCmdProc proc, ObjCmdData data) {
    commandNames_.push_back(name);

    return createObjCommandI(name, proc, data);
  }

  const StringList &commandNames() const { return commandNames_; }

  //---

  // create alias
  int createAlias(const std::string &newName, const std::string &oldName) {
    return Tcl_CreateAlias(interp(), newName.c_str(), interp(), oldName.c_str(), 0, nullptr);
  }

  //---

  // evaluate expression
  bool evalExpr(const std::string &expr, std::string &res, bool showError=false) {
    EvalData evalData;

    evalData.showError = showError;

    auto rc = evalExpr(expr, evalData);

    res = evalData.res;

    return rc;
  }

  bool evalExpr(const std::string &expr, bool showError=false) {
    EvalData evalData;

    evalData.showError = showError;

    return evalExpr(expr, evalData);
  }

  bool evalExpr(const std::string &expr, EvalData &evalData) {
    return eval("expr {" + expr + "}", evalData);
  }

  //---

  bool eval(const std::string &cmd, std::string &res, bool showError=false) {
    EvalData evalData;

    evalData.showError = showError;

    auto rc = eval(cmd, evalData);

    res = evalData.res;

    return rc;
  }

  bool eval(const std::string &cmd, bool showError=false, bool showResult=false) {
    EvalData evalData;

    evalData.showError  = showError;
    evalData.showResult = showResult;

    return eval(cmd, evalData);
  }

  bool eval(const std::string &cmd, EvalData &evalData) {
    bool res = true;

    evalData.rc = CTclUtil::eval(interp(), cmd.c_str());

    lastError_ = evalData.rc;

    if (evalData.rc != TCL_OK) {
      evalData.errMsg = errorInfo(evalData.rc);

      if (evalData.showError)
        outputError(evalData.errMsg);

      res = false;
    }

    evalData.res = getResult();

    if (evalData.showResult) {
      if (evalData.res.size())
        outputResult(evalData.res);
    }

    return res;
  }

  static bool splitList(const std::string &str, StringList &strs) {
    return CTclUtil::splitList(str, strs);
  }

  static std::string mergeList(const StringList &strs) {
    return CTclUtil::mergeList(strs);
  }

  //---

  void traceVar(const std::string &name) {
    int flags = TCL_TRACE_READS | TCL_TRACE_WRITES | TCL_TRACE_UNSETS | TCL_GLOBAL_ONLY;

    ClientData data =
      Tcl_VarTraceInfo(interp(), name.c_str(), flags, &CTcl::traceProc, nullptr);

    if (! data) {
      Tcl_TraceVar(interp(), name.c_str(), flags,
                   &CTcl::traceProc, static_cast<ClientData>(this));

      traces_.insert(name);
    }
  }

  void untraceVar(const std::string &name) {
    int flags = TCL_TRACE_READS | TCL_TRACE_WRITES | TCL_TRACE_UNSETS | TCL_GLOBAL_ONLY;

    Tcl_UntraceVar(interp(), name.c_str(), flags,
                   &CTcl::traceProc, static_cast<ClientData>(this));

    traces_.erase(name);
  }

  void handleTrace(const char *name, int flags) {
    // ignore unset called on trace destruction
    if (flags & TCL_TRACE_UNSETS) return;

    bool handled = false;

    if (flags & TCL_TRACE_READS ) { handleRead (name); handled = true; }
    if (flags & TCL_TRACE_WRITES) { handleWrite(name); handled = true; }
  //if (flags & TCL_TRACE_UNSETS) { handleUnset(name); handled = true; }

    assert(handled);
  }

  //---

  virtual void handleRead(const char *name) {
    std::cerr << "CTcl::handleRead " << name << "\n";
  }

  virtual void handleWrite(const char *name) {
    std::cerr << "CTcl::handleWrite " << name << "\n";
  }

#if 0
  virtual void handleUnset(const char *name) {
    std::cerr << "CTcl::handleUnset " << name << "\n";
  }
#endif

  virtual void outputError(const std::string &msg) {
    std::cerr << msg << "\n";
  }

  virtual void outputResult(const std::string &res) {
    std::cout << res.c_str() << "\n";
  }

  void processEvents() {
    while (Tcl_DoOneEvent(TCL_DONT_WAIT));
  }

  void setResult(Tcl_Obj *obj) {
    Tcl_SetObjResult(interp(), obj);
  }

  void setResult(int rc) {
    Tcl_SetObjResult(interp(), Tcl_NewIntObj(rc));
  }

  void setResult(double rc) {
    Tcl_SetObjResult(interp(), Tcl_NewDoubleObj(rc));
  }

  void setResult(const char *rc) {
    Tcl_SetObjResult(interp(), Tcl_NewStringObj(rc, -1));
  }

  void setResult(const std::string &rc) {
    Tcl_SetObjResult(interp(), Tcl_NewStringObj(rc.c_str(), int(rc.size())));
  }

  void setResult(const IntList &rc) {
    auto *obj = Tcl_NewListObj(0, nullptr);

    for (const auto &i : rc) {
      auto *iobj = Tcl_NewIntObj(i);

      Tcl_ListObjAppendElement(interp(), obj, iobj);
    }

    Tcl_SetObjResult(interp(), obj);
  }

  void setResult(const RealList &rc) {
    auto *obj = Tcl_NewListObj(0, nullptr);

    for (const auto &r : rc) {
      auto *robj = Tcl_NewDoubleObj(r);

      Tcl_ListObjAppendElement(interp(), obj, robj);
    }

    Tcl_SetObjResult(interp(), obj);
  }

  void setResult(const StringList &rc) {
    auto *obj = Tcl_NewListObj(0, nullptr);

    for (const auto &s : rc) {
      auto *sobj = Tcl_NewStringObj(s.c_str(), int(s.size()));

      Tcl_ListObjAppendElement(interp(), obj, sobj);
    }

    Tcl_SetObjResult(interp(), obj);
  }

  void setResult(const std::vector<IntList> &rc) {
    auto *obj = Tcl_NewListObj(0, nullptr);

    for (const auto &l : rc) {
      auto *obj1 = Tcl_NewListObj(0, nullptr);

      for (const auto &i : l) {
        auto *iobj = Tcl_NewIntObj(i);

        Tcl_ListObjAppendElement(interp(), obj1, iobj);
      }

      Tcl_ListObjAppendElement(interp(), obj, obj1);
    }

    Tcl_SetObjResult(interp(), obj);
  }

  void setResult(const std::vector<RealList> &rc) {
    auto *obj = Tcl_NewListObj(0, nullptr);

    for (const auto &l : rc) {
      auto *obj1 = Tcl_NewListObj(0, nullptr);

      for (const auto &r : l) {
        auto *robj = Tcl_NewDoubleObj(r);

        Tcl_ListObjAppendElement(interp(), obj1, robj);
      }

      Tcl_ListObjAppendElement(interp(), obj, obj1);
    }

    Tcl_SetObjResult(interp(), obj);
  }

  void setResult(const std::vector<StringList> &rc) {
    auto *obj = Tcl_NewListObj(0, nullptr);

    for (const auto &l : rc) {
      auto *obj1 = Tcl_NewListObj(0, nullptr);

      for (const auto &s : l) {
        auto *sobj = Tcl_NewStringObj(s.c_str(), int(s.size()));

        Tcl_ListObjAppendElement(interp(), obj1, sobj);
      }

      Tcl_ListObjAppendElement(interp(), obj, obj1);
    }

    Tcl_SetObjResult(interp(), obj);
  }

  //---

  std::string getResult() {
    Tcl_Obj *res = Tcl_GetObjResult(interp());

    Tcl_Obj *res1 = Tcl_DuplicateObj(res);

    auto str = CTclUtil::stringFromObj(res);

    Tcl_SetObjResult(interp(), res1);

    return str;
  }

  bool isDomainError(int rc) const {
    return CTclUtil::isDomainError(interp(), rc);
  }

  std::string errorInfo(int rc) const {
    return CTclUtil::errorInfo(interp(), rc);
  }

  std::string errorInfo() const {
    return CTclUtil::errorInfo(interp(), lastError_);
  }

 public:
  // Tk stuff
  unsigned long nameToWindow(const std::string &name);

  void mapWindow(unsigned long win);

  int winId(unsigned long win);

 private:
  Tcl_Command createObjCommandI(const std::string &name, ObjCmdProc proc, ObjCmdData data) {
    return Tcl_CreateObjCommand(interp(), const_cast<char *>(name.c_str()),
                                proc, data, nullptr);
  }

 private:
  static char *traceProc(ClientData data, Tcl_Interp *, const char *name1,
                         const char *, int flags) {
    auto *th = static_cast<CTcl *>(data);
    assert(th);

    th->handleTrace(name1, flags);

    return nullptr;
  }

 private:
  Tcl_Interp* interp_ { nullptr };
  Traces      traces_;
  StringList  commandNames_;
  int         lastError_ { 0 };
};

//---

class CTclObjectProc {
 public:
  CTclObjectProc(CTcl *tcl, const char *name) {
    tcl->createObjCommand(name, cmdProc, this);
  }

  virtual ~CTclObjectProc() { }

  static int cmdProc(ClientData clientData, Tcl_Interp* /*interp*/,
                     int objc, Tcl_Obj * const *objv) {
    auto *t = reinterpret_cast<CTclObjectProc *>(clientData);

    auto args = CTclUtil::getObjArgs(objc, objv);

    return t->argsProc(args);
  }

  virtual int argsProc(CTclUtil::StringList &) = 0;
};

#define CTCL_DCL_OBJECT_PROC(TYPE, NAME, PROC, DATA) \
template<typename TYPE> \
class CTcl##NAME##ObjectProc : public CTclObjectProc { \
 public: \
  CTcl##NAME##ObjectProc<TYPE>(CTcl *tcl, TYPE *data) : \
   CTclObjectProc(tcl, #NAME), data_(data) { } \
\
  int argsProc(CTclUtil::StringList &args) override { \
    return data_->PROC(args); \
  } \
\
 private: \
  TYPE* data_ { nullptr }; \
};

#define CTCL_OBJECT_PROC(TCL, PROC, TYPE, DATA) \
static_cast<void *>(new CTcl##PROC##ObjectProc<TYPE>(TCL, DATA));

//---

class CTclObjProc {
 public:
  CTclObjProc(CTcl *tcl, const char *name) {
    tcl->createObjCommand(name, cmdProc, this);
  }

  virtual ~CTclObjProc() { }

  static int cmdProc(ClientData clientData, Tcl_Interp* /*interp*/,
                     int objc, Tcl_Obj * const *objv) {
    auto *t = reinterpret_cast<CTclObjProc *>(clientData);

    cmd_ = CTclUtil::stringFromObj(objv[0]);

    std::vector<Tcl_Obj *> objs;

    for (int i = 1; i < objc; ++i)
      objs.push_back(objv[i]);

    return t->objProc(objs);
  }

  virtual int objProc(const std::vector<Tcl_Obj *> &objs) = 0;

 protected:
  static std::string cmd_;
};

#define CTCL_DCL_TCL_OBJ_PROC(TYPE, NAME, PROC, DATA) \
template<typename TYPE> \
class CTcl##NAME##TclObjProc : public CTclObjProc { \
 public: \
  CTcl##NAME##TclObjProc<TYPE>(CTcl *tcl, TYPE *data) : \
   CTclObjProc(tcl, #NAME), data_(data) { } \
\
  int objProc(const std::vector<Tcl_Obj *> &objs) override { \
    return data_->PROC(objs); \
  } \
\
 private: \
  TYPE* data_ { nullptr }; \
};

#define CTCL_TCL_OBJ_PROC(TCL, PROC, TYPE, DATA) \
static_cast<void *>(new CTcl##PROC##TclObjProc<TYPE>(TCL, DATA));

#endif
