#include <CShLibMgr.h>

CShLibMgr *CShLibMgr::instance_ = NULL;

CShLibMgr *
CShLibMgr::
getInstance()
{
  if (instance_ == NULL)
    instance_ = new CShLibMgr();

  return instance_;
}

CShLib *
CShLibMgr::
lookup(const std::string &filename)
{
  if (libs_.find(filename) != libs_.end())
    return libs_[filename];

  CShLib *shlib = new CShLib(filename);

  if (! shlib->open()) {
    delete shlib;
    return NULL;
  }

  libs_[filename] = shlib;

  return NULL;
}
