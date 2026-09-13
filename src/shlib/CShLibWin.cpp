#include <CShLibI.h>
#include <std_win.h>

bool
CShLibWin::
open()
{
  address_ = LoadLibrary(filename_.c_str());

  if (address_ == 0)
    return false;

  return true;
}

bool
CShLibWin::
getProc(const std::string &name, CShLibProc *proc)
{
  CShLibData *data;

  if (! getData(name, &data))
    return false;

  *proc = (CShLibProc) data;

  return true;
}

bool
CShLibWin::
getData(const std::string &name, CShLibData *data)
{
  FARPROC proc_address = GetProcAddress((HMODULE) address_, name.c_str());

  *data = (CShLibData) proc_address;

  return true;
}

void
CShLibWin::
close()
{
  FreeLibrary((HMODULE) address_);
}
