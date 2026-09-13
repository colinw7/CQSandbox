#ifndef CSHLIB_H
#define CSHLIB_H

#include <map>
#include <string>

#ifdef OS_UNIX
  typedef void *CShLibPtr;
#else
  typedef HINSTANCE CShLibPtr;
#endif

typedef int  (*CShLibProc)();
typedef void  *CShLibData;

class CShLib;
class CShLibImpl;

#define CShLibMgrInst CShLibMgr::getInstance()

class CShLibMgr {
 public:
  static CShLibMgr *getInstance();

 ~CShLibMgr() { }

  CShLib *lookup(const std::string &filename);

 private:
  CShLibMgr() { }

 private:
  static CShLibMgr *instance_;

  std::map<std::string, CShLib *> libs_;
};

//---

class CShLib {
 public:
  CShLib(const std::string &filename);
 ~CShLib();

  bool open();
  bool getProc(const std::string &name, CShLibProc *proc);
  bool getData(const std::string &name, CShLibData *data);
  void close();

 private:
  bool        open_ { false };
  CShLibImpl *impl_ { nullptr };
};

#endif
