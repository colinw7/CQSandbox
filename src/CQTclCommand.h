#ifndef CQTclCommand_H
#define CQTclCommand_H

#include <CQCommand.h>

class CQTcl;

// forward declare
namespace CQTclCmd {
class Mgr;
class CmdProc;
}

//---

namespace CQTclCommand {

class ScrollArea;

//---

// command widget (terminal prompt and output)
class CommandWidget : public CQCommand::CommandWidget {
  Q_OBJECT

 public:
  using OptionValues = std::map<std::string, std::string>;

 public:
  CommandWidget(ScrollArea *area);

 ~CommandWidget();

  CQTclCmd::Mgr *tclCmdMgr() const;

  void setTcl(CQTcl *tcl);

  void addCommand(const QString &name, CQTclCmd::CmdProc *proc);

  void executeCommand(const QString &line);

  bool eval(const QString &line, bool showError=true, bool showResult=false);

  // override to handle text changes (before enter)
  void textChanged() override;

  // override to provide syntax highlighting
  QColor posColor(int pos) const override;

  bool isCompleteLine(const QString &str) const override;

  // override to provide command completion
  bool complete(const QString &text, int pos, QString &newText,
                CompleteMode completeMode) const override;

  bool completeCommand(const QString &lhs, const QString &str, const QString &rhs,
                       bool interactive, QString &command) const;

  bool completeOption(const QString &cmdName, const QString &lhs, const QString &str,
                      const QString &rhs, bool interactive, QString &option) const;

  bool completeArg(const QString &cmdName, const QString &option, const OptionValues &optionValues,
                   const QString &lhs, const QString &str, const QString &rhs,
                   bool interactive, QString &argText) const;

 protected:
  using Colors = std::vector<QColor>;

  CQTcl*         qtcl_      { nullptr };
  bool           qtclOwner_ { true };
  CQTclCmd::Mgr* mgr_       { nullptr };
  Colors         colors_;
};

//---

// Scroll Area containing command widget
class ScrollArea : public CQCommand::ScrollArea {
  Q_OBJECT

 public:
  ScrollArea(QWidget *parent=nullptr);

  CQTclCmd::Mgr *tclCmdMgr() const;

  void setTcl(CQTcl *tcl);

  CommandWidget *getCommandWidget();

  CQCommand::CommandWidget *createCommandWidget() const override;

  void executeCommand(const QString &line);

  bool eval(const QString &line, bool showError=true, bool showResult=false);

  void addCommand(const QString &name, CQTclCmd::CmdProc *proc);

 protected:
  CommandWidget* widget_ { nullptr };
};

//---

}

//------

#include <CQTclCmd.h>

namespace CQTclCommand {
class ScrollArea;
}

//---

namespace CQTclCommand {

// override tcl command proc to store scroll area and data
class TclCmdProc : public CQTclCmd::CmdProc {
 public:
  enum class ArgType {
    None    = int(CQTclCmd::CmdArg::Type::None),
    Boolean = int(CQTclCmd::CmdArg::Type::Boolean),
    Integer = int(CQTclCmd::CmdArg::Type::Integer),
    Real    = int(CQTclCmd::CmdArg::Type::Real),
    String  = int(CQTclCmd::CmdArg::Type::String),
    SBool   = int(CQTclCmd::CmdArg::Type::SBool),
    Enum    = int(CQTclCmd::CmdArg::Type::Enum)
  };

  using CmdArg = CQTclCmd::CmdArg;

 public:
  TclCmdProc(ScrollArea *area) :
   CQTclCmd::CmdProc(area->tclCmdMgr()), area_(area) {
  }

  //! get/set data
  void *data() const { return data_; }
  void setData(void *p) { data_ = p; }

  CmdArg &addArg(CQTclCmd::CmdArgs &args, const QString &name, ArgType type,
                 const QString &argDesc="", const QString &desc="") {
    return args.addCmdArg(name, int(type), argDesc, desc);
  }

 protected:
  ScrollArea *area_ { nullptr };
  void*       data_ { nullptr };
};

}

//---

// auto generate command instance
#define CQTCL_AREA_CMD(NAME, AREA) \
class NAME##TclCmd : public TclCmdProc { \
 public: \
  NAME##TclCmd(AREA *clientData=nullptr) : \
   TclCmdProc(clientData) { \
  } \
\
  AREA *area() const { return dynamic_cast<AREA *>(area_); } \
\
  bool exec(CQTclCmd::CmdArgs &args) override; \
\
  void addArgs(CQTclCmd::CmdArgs &args) override; \
\
  QStringList getArgValues(const QString &arg, \
                           const NameValueMap &nameValueMap=NameValueMap()) override; \
};

#define CQTCL_CMD(NAME) \
class NAME##TclCmd : public TclCmdProc { \
 public: \
  NAME##TclCmd(ScrollArea *clientData=nullptr) : \
   TclCmdProc(clientData) { \
  } \
\
  ScrollArea *area() const { return dynamic_cast<ScrollArea *>(area_); } \
\
  bool exec(CQTclCmd::CmdArgs &args) override; \
\
  void addArgs(CQTclCmd::CmdArgs &args) override; \
\
  QStringList getArgValues(const QString &arg, \
                           const NameValueMap &nameValueMap=NameValueMap()) override; \
};

// auto generate command instance with id
#define CQTCL_INST_AREA_CMD(NAME, AREA) \
class NAME##InstTclCmd : public TclCmdProc { \
 public: \
  NAME##InstTclCmd(AREA *clientData=nullptr, const QString &id) : \
   TclCmdProc(clientData), id_(id) { } \
\
  AREA *area() const { return dynamic_cast<AREA *>(area_); } \
\
  bool exec(CQTclCmd::CmdArgs &args) override; \
\
  void addArgs(CQTclCmd::CmdArgs &args) override; \
\
  QStringList getArgValues(const QString &name, \
                           const NameValueMap &nameValueMap=NameValueMap()) override; \
\
 public: \
  QString id_; \
};

#define CQTCL_INST_CMD(NAME) \
class NAME##InstTclCmd : public TclCmdProc { \
 public: \
  NAME##InstTclCmd(ScrollArea *clientData=nullptr, const QString &id) : \
   TclCmdProc(clientData), id_(id) { } \
\
  ScrollArea *area() const { return dynamic_cast<ScrollArea *>(area_); } \
\
  bool exec(CQTclCmd::CmdArgs &args) override; \
\
  void addArgs(CQTclCmd::CmdArgs &args) override; \
\
  QStringList getArgValues(const QString &name, \
                           const NameValueMap &nameValueMap=NameValueMap()) override; \
\
 public: \
  QString id_; \
};

#endif
