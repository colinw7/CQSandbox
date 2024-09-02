#include <CTclUtil.h>
#include <CStrParse.h>
#include <vector>

#include <tk.h>
#ifdef TIX_SUPPORT
#include <tix.h>
#endif

class CTclUtilP {
 public:
  CTclUtilP() { }

  bool isCompleteLine(char endChar);

  void startStringParse(const std::string &str);
  void endParse();

 private:
  using ParseStack = std::vector<CStrParse *>;

  CStrParse* parse_ { nullptr };
  ParseStack parseStack_;
};

//---

bool
CTclUtil::
isCompleteLine(const std::string &line)
{
  CTclUtilP util;

  util.startStringParse(line);

  bool rc = util.isCompleteLine('\0');

  util.endParse();

  return rc;
}

//---

void
CTclUtilP::
startStringParse(const std::string &str)
{
  parseStack_.push_back(parse_);

  parse_ = new CStrParse(str);
}

void
CTclUtilP::
endParse()
{
  delete parse_;

  parse_ = parseStack_.back();

  parseStack_.pop_back();
}

bool
CTclUtilP::
isCompleteLine(char endChar)
{
  while (! parse_->eof()) {
    if      (parse_->isChar('[')) {
      parse_->skipChar();

      if (! isCompleteLine(']'))
        return false;

      if (! parse_->isChar(']'))
        return false;

      parse_->skipChar();
    }
    else if (parse_->isChar('{')) {
      parse_->skipChar();

      if (! isCompleteLine('}'))
        return false;

      if (! parse_->isChar('}'))
        return false;

      parse_->skipChar();
    }
    else if (parse_->isChar(endChar)) {
      return true;
    }
    else if (parse_->isChar('\"')) {
      parse_->skipChar();

      if (! isCompleteLine('\"'))
        return false;

      if (! parse_->isChar('\"'))
        return false;

      parse_->skipChar();
    }
    else if (parse_->isChar('\'')) {
      parse_->skipChar();

      if (! isCompleteLine('\''))
        return false;

      if (! parse_->isChar('\''))
        return false;

      parse_->skipChar();
    }
    else if (parse_->isChar('\\')) {
      parse_->skipChar();
      parse_->skipChar();
    }
    else
      parse_->skipChar();
  }

  return true;
}

//---

CTcl::
CTcl()
{
  interp_ = Tcl_CreateInterp();
}

CTcl::
~CTcl() {
  Tcl_DeleteInterp(interp_);
}

bool
CTcl::
init()
{
  if (Tcl_Init(interp()) == TCL_ERROR)
    return false;

  return true;
}

bool
CTcl::
initTk()
{
  if (Tk_Init(interp()) == TCL_ERROR)
    return false;

  return true;
}

#ifdef TIX_SUPPORT
bool
CTcl::
initTix()
{
  if (Tix_Init(interp()) == TCL_ERROR)
    return false;

  return true;
}
#endif

unsigned long
CTcl::
nameToWindow(const std::string &name)
{
  return reinterpret_cast<unsigned long>(
    Tk_NameToWindow(interp(), name.c_str(), Tk_MainWindow(interp())));
}

void
CTcl::
mapWindow(unsigned long win)
{
  auto win1 = reinterpret_cast<Tk_Window>(win);

  if (! Tk_IsMapped(win1))
    Tk_MapWindow(win1);
}

int
CTcl::
winId(unsigned long win)
{
  auto win1 = reinterpret_cast<Tk_Window>(win);

  return Tk_WindowId(win1);
}
