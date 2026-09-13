#include <CShLibI.h>

bool
CShLibHP::
open()
{
  if (filename_[0] != '/') {
    pathname_ = searchLib("LD_LIBRARY_PATH", filename_);

    if (pathname_ == "")
      pathname_ = searchLib("", filename_);

    if (pathname_ == "" && FileIsRegular(filename_))
      pathname_ = filename_;
  }
  else {
    if (FileIsRegular(filename_))
      pathname_ = filename_;
    else
      pathname_ = "";
  }

  if (pathname_ == "") {
    fprintf(stderr, "Failed to Find Shared Library %s\n", filename_);
    return false;
  }

  address_ = shl_load(filename_.c_str(), BIND_IMMEDIATE | BIND_NONFATAL | BIND_VERBOSE, 0);

  if (address_ == NULL) {
    fprintf(stderr, "Failed to Load Shared Library %s - %s\n", filename_.c_str(), dlerror());
    return false;
  }

  return true;
}

bool
CShLibHP::
getProc(const std::string &name, CShLibProc *proc)
{
  CShLibData *data;

  if (! getData(name, &data))
    return false;

  *proc = (CShLibProc) data;

  return true;
}

bool
CShLibHP::
getData(const std::string &name, CShLibData *data)
{
  void *symbol;

  int flag = shl_findsym((shl_t) address_, name.c_str() TYPE_UNDEFINED, &symbol);

  if (flag != 0) {
    fprintf(stderr, "Failed to Find Symbol %s in Library %s - %s\n",
            name.c_str(), filename_.c_str(), strerror(errno));
    return false;
  }

  *data = (CShLibData) symbol;

  return true;
}

void
CShLibHP::
close()
{
  shl_unload((shl_t) address_);
}
