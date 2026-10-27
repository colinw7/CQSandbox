#ifndef CQSandboxXml_H
#define CQSandboxXml_H

#include <CQXml.h>

#include <CQTclCheckBox.h>
#include <CQTclIntegerSpin.h>
#include <CQTclRealSpin.h>
#include <CQTclUtil.h>

namespace CQSandbox {

class Xml : public CQXml {
 public:
  Xml(CQTcl *tcl) :
   CQXml(), tcl_(tcl) {
    CQXmlAddWidgetFactoryT(this, CQTclCheckBox);
    CQXmlAddWidgetFactoryT(this, CQTclIntegerSpin);
    CQXmlAddWidgetFactoryT(this, CQTclRealSpin);
  }

  void createNotify(QWidget *w) override {
    auto *check = qobject_cast<CQTclCheckBox *>(w);
    auto *ispin = qobject_cast<CQTclIntegerSpin *>(w);
    auto *rspin = qobject_cast<CQTclRealSpin *>(w);

    if (check || ispin || rspin) {
      auto *interp = tcl_->interp();

      if      (check) check->setInterp(interp);
      else if (ispin) ispin->setInterp(interp);
      else if (rspin) rspin->setInterp(interp);
    }
  }

  void execSlot(const QString &value, const QStringList &args) override {
    auto text = getExecData("text").toString();
    tcl_->createVar("execText", text);

    tcl_->createVar("execArgs", args);

    auto cmd = value;

    for (const auto &arg : args)
      cmd += QString(" {%1}").arg(arg);

    runTclCmd(cmd);
  }

 private:
  bool runTclCmd(const QString &cmd) {
    auto rc = tcl_->eval(cmd, /*showError*/true, /*showResult*/false);

    if (! rc)
      std::cerr << QString("Command '%1' failed").arg(cmd).toStdString() << "\n";

    return rc;
  }

 private:
  CQTcl* tcl_ { nullptr };
};

}

#endif
