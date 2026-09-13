#include <CShLibI.h>
#include <CFile.h>
#include <dlfcn.h>

CShLibSun::
CShLibSun(const std::string &filename) :
 CShLibImpl(filename)
{
}

CShLibSun::
~CShLibSun()
{
}

bool
CShLibSun::
open()
{
  if (filename_[0] != '/') {
    pathname_ = searchLib("LD_LIBRARY_PATH", filename_);

    if (pathname_ == "")
      pathname_ = searchLib("", filename_);

    if (pathname_ == "" && CFile::isRegular(filename_))
      pathname_ = filename_;
  }
  else {
    if (CFile::isRegular(filename_))
      pathname_ = filename_;
    else
      pathname_ = "";
  }

  if (pathname_ == "") {
    fprintf(stderr, "Failed to Find Shared Library %s\n", filename_.c_str());
    return false;
  }

  address_ = dlopen(filename_.c_str(), RTLD_NOW | RTLD_GLOBAL);
//address_ = dlopen(filename_.c_str(), RTLD_LAZY | RTLD_GLOBAL);

  if (address_ == NULL) {
    fprintf(stderr, "Failed to Load Shared Library %s - %s\n",
            filename_.c_str(), dlerror());
    return false;
  }

  return true;
}

bool
CShLibSun::
getProc(const std::string &name, CShLibProc *proc)
{
  CShLibData data;

  if (! getData(name, &data))
    return false;

  *proc = reinterpret_cast<CShLibProc>(reinterpret_cast<long>(data));
//*proc = (CShLibProc) data;

  return true;
}

bool
CShLibSun::
getData(const std::string &name, CShLibData *data)
{
  void *symbol = dlsym(address_, name.c_str());

  auto *error_msg = dlerror();

  if (error_msg) {
    fprintf(stderr, "Failed to Find Symbol %s in Library %s - %s\n",
            name.c_str(), filename_.c_str(), error_msg);
    return false;
  }

  *data = reinterpret_cast<CShLibData>(symbol);

  return true;
}

void
CShLibSun::
close()
{
  dlclose(address_);
}
