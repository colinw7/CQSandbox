#include <CShLibI.h>

bool
CShLibSGI::
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
    fprintf(stderr, "Failed to Find Shared Library %s\n", filename_.c_str());
    return false;
  }

  return true;
}

bool
CShLibSGI::
getProc(const std::string &name, CShLibProc *proc)
{
  CShLibData *data;

  if (! getData(name, &data))
    return false;

  *proc = (CShLibProc) data;

  return true;
}

bool
CShLibSGI::
getData(const std::string &name, CShLibData *data)
{
  void *symbol = dl_loadmod(__argv[0], pathname_.c_str(), name.c_str());

  if (symbol == NULL) {
    fprintf(stderr, "Failed to Find Symbol %s in Library %s - %s\n",
            name.c_str(), filename_.c_str(), strerror(errno));
    return false;
  }

  *data = (CShLibData) symbol;

  return true;
}

void
CShLibSGI::
close()
{
}
