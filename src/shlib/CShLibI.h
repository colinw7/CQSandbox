#include <CShLib.h>
#include <string>

class CShLibImpl {
 public:
  CShLibImpl(const std::string &filename);

  virtual ~CShLibImpl();

  virtual bool open() = 0;
  virtual bool getProc(const std::string &name, CShLibProc *proc) = 0;
  virtual bool getData(const std::string &name, CShLibData *data) = 0;
  virtual void close() = 0;

 protected:
  std::string searchLib(const std::string &envname, const std::string &name);

 protected:
  std::string filename_;
  std::string pathname_;
  CShLibPtr   address_;
};

#ifdef OS_UNIX
# if defined(OS_LINUX) || defined(OS_SUN) || defined(OS_OSX)
#  include <CShLibSun.h>
# elif defined(OS_HP700)
#  include <CShLibHP.h>
# elif defined(OS_SGI)
#  include <CShLibSGI.h>
# else
#  error "OS not supported"
# endif
#else
# include <CShLibWin.h>
#endif
