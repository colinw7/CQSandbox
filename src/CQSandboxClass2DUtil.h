#ifndef CQSandboxClass2DUtil_H
#define CQSandboxClass2DUtil_H

int
Tcl_InvokeMethod(Tcl_Interp *interp, std::vector<std::string> &argNames,
                 std::vector<std::string> &argValues, const std::string &body,
                 Tcl_Obj **res);

#endif
