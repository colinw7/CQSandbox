#include <CQSandboxXML3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CXML.h>
#include <CXMLToken.h>
#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
Xml3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  if (args.size() != 1)
    return nullptr;

  auto *tcl = canvas->tcl();

  auto filename = args[0];

  auto *obj = new Xml3DObj(canvas, filename);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Xml3DObj::
Xml3DObj(Canvas3D *canvas, const QString &filename) :
 Object3D(canvas, Type::CSV), filename_(filename)
{
  xml_ = new CXML;
}

Xml3DObj::
~Xml3DObj()
{
  delete xml_;
}

void
Xml3DObj::
init()
{
  Object3D::init();
}

bool
Xml3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if (! xml_)
    return false;

  //auto *app = canvas_->app();

  auto getTag = [&](CXMLTag* &tag) {
    tag = nullptr;

    int tagInd = -1;

    if (args.size() >= 1) {
      if (args[0] == "root")
        return true;

      tagInd = Util::stringToInt(args[0]);
    }

    CXML::TokenArray tokens;

    tag = tag_;

    if (tagInd > 0)
      tag = xml_->getTagByInd(tagInd);

    if (! tag)
      return false;

    return true;
  };

  if      (name == "filename")
    value = filename_;
  else if (name == "name") {
    CXMLTag *tag;
    if (! getTag(tag))
      return false;

    if (tag)
      value = QString::fromStdString(tag->getName());
    else
      value = "root";
  }
  else if (name == "tag_names" || name == "tag_inds") {
    CXMLTag *tag;
    if (! getTag(tag))
      return false;

    CXML::TokenArray tokens;

    if (! tag)
      tokens = xml_->getTokens();
    else
      tokens = tag->getChildren();

    QStringList names;

    for (auto *token : tokens) {
      if (token->isTag()) {
        auto *tag1 = token->getTag();

        if (name == "tag_names")
          names.push_back(QString::fromStdString(tag1->getName()));
        else
          names.push_back(QString::number(tag1->ind()));
      }
    }

    value = names;
  }
  else if (name == "tag_options") {
    CXMLTag *tag;
    if (! getTag(tag))
      return false;

    if (! tag)
      return false;

    QVariantList nameValues;

    const auto &options = tag->getOptions();

    for (auto *option : options) {
      auto name  = option->getName();
      auto value = option->getValue();

      QStringList nameValue;

      nameValue.push_back(QString::fromStdString(name));
      nameValue.push_back(QString::fromStdString(value));

      nameValues.push_back(nameValue);
    }

    value = nameValues;
  }
  else
    return Object3D::getValue(name, args, value);

  return true;
}

bool
Xml3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "filename")
    filename_ = value;
  else
    return Object3D::setValue(name, value, args);

  return true;
}

bool
Xml3DObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  if (op == "load") {
    bool b = xml_->read(filename_.toStdString(), &tag_);

    res = QVariant(b);
  }
  else
    return Object3D::exec(op, args, res);

  return true;
}

}
