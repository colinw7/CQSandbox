#include <CShLibI.h>
#include <CStrUtil.h>
#include <CEnv.h>
#include <CFile.h>

//---------------------

CShLib::
CShLib(const std::string &filename)
{
  open_ = false;

#ifdef OS_UNIX
# if defined(OS_LINUX) || defined(OS_SUN) || defined(OS_OSX)
   impl_ = new CShLibSun(filename);
# elif defined(OS_HP700)
   impl_ = new CShLibHP(filename);
# elif defined(OS_SGI)
   impl_ = new CShLibSGI(filename);
# else
#  error "OS not supported"
# endif
#else
  impl_ = new CShLibWin(filename);
#endif
}

CShLib::
~CShLib()
{
  if (open_)
    impl_->close();
}

bool
CShLib::
open()
{
  if (open_)
    impl_->close();

  if (impl_->open())
    open_ = true;
  else
    open_ = false;

  return open_;
}

bool
CShLib::
getProc(const std::string &name, CShLibProc *proc)
{
  if (! open_)
    return false;

  return impl_->getProc(name, proc);
}

bool
CShLib::
getData(const std::string &name, CShLibData *data)
{
  if (! open_)
    return false;

  return impl_->getData(name, data);
}

void
CShLib::
close()
{
  if (open_)
    impl_->close();
}

//---------------------

CShLibImpl::
CShLibImpl(const std::string &filename) :
 filename_(filename)
{
}

CShLibImpl::
~CShLibImpl()
{
}

bool
CShLibImpl::
open()
{
  return false;
}

bool
CShLibImpl::
getProc(const std::string &, CShLibProc *proc)
{
  *proc = nullptr;

  return false;
}

bool
CShLibImpl::
getData(const std::string &, CShLibData *data)
{
  *data = nullptr;

  return false;
}

void
CShLibImpl::
close()
{
}

std::string
CShLibImpl::
searchLib(const std::string &envname, const std::string &name)
{
  std::string envval;

  if (CEnvInst.exists(envname))
    envval = CEnvInst.get(envname);
  else
    envval = "/usr/lib:/lib";

  std::vector<std::string> fields;

  CStrUtil::addFields(envval, fields, ":");

  auto num_fields = fields.size();

  for (size_t i = 0; i < num_fields; ++i) {
    std::string filename = fields[i] + "/" + name;

    if (CFile::isRegular(filename))
      return filename;
  }

  return "";
}
