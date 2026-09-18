#include <CQSandboxControl2D.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxViewport.h>

#include <CQXml.h>
#include <CQTclUtil.h>
#include <CQTclIntegerSpin.h>
#include <CQPropertyViewTree.h>
#include <CQUtil.h>

#include <QTabWidget>
#include <QListWidget>
#include <QCheckBox>
#include <QVBoxLayout>

namespace CQSandbox {

class Xml2D : public CQXml {
 public:
  Xml2D(Control2D *control) :
   CQXml(), control_(control) {
  }

  void createNotify(QWidget *w) override {
    auto *ispin = qobject_cast<CQTclIntegerSpin *>(w);

    if (ispin) {
      auto *canvas = control_->canvas();

      ispin->setInterp(canvas->tcl()->interp());
    }
  }

  void execSlot(const QString &value, const QStringList &args) override {
    auto *canvas = control_->canvas();

    auto text = getExecData("text").toString();
    canvas->tcl()->createVar("execText", text);

    canvas->tcl()->createVar("execArgs", args);

    auto cmd = value;

    for (const auto &arg : args)
      cmd += QString(" {%1}").arg(arg);

    canvas->runTclCmd(cmd);
  }

 private:
  Control2D* control_ { nullptr };
};

}

//---

namespace CQSandbox {

Control2D::
Control2D(Canvas2D *canvas) :
 canvas_(canvas)
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  //---

  auto *layout = new QVBoxLayout(this);

  auto *tab = new QTabWidget;

  layout->addWidget(tab);

  //---

  auto *controlFrame = addObjectsFrame();

  tab->addTab(controlFrame, "Objects");

  //---

  uiFrame_ = new QFrame;

  tab->addTab(uiFrame_, "UI");

  //---

  updateWidgets();

  //---

  if (canvas_)
    connect(canvas_, &Canvas2D::objectsChanged, this, &Control2D::updateWidgets);
}

Control2D::
~Control2D()
{
  delete xml_;
}

QFrame *
Control2D::
addObjectsFrame()
{
  auto *frame  = new QFrame;
  auto *layout = new QVBoxLayout(frame);

  //---

  objectsData_.list = new QListWidget;

  objectsData_.list->setSelectionMode(QAbstractItemView::SingleSelection);

  layout->addWidget(objectsData_.list);

  //---

  objectsData_.tree = new CQPropertyViewTree(this);

  layout->addWidget(objectsData_.tree);

  //---

  connectObjects(true);

  return frame;
}

void
Control2D::
connectObjects(bool b)
{
  if (b) {
    connect(objectsData_.list, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(objectSelectedSlot(QListWidgetItem *, QListWidgetItem *)));
    connect(objectsData_.tree, SIGNAL(valueChanged(QObject *, const QString &)),
            this, SLOT(objectChangedSlot(QObject *, const QString &)));
  }
  else {
    disconnect(objectsData_.list, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
               this, SLOT(objectSelectedSlot(QListWidgetItem *, QListWidgetItem *)));
    disconnect(objectsData_.tree, SIGNAL(valueChanged(QObject *, const QString &)),
               this, SLOT(objectChangedSlot(QObject *, const QString &)));
  }
}

void
Control2D::
setActive(bool b)
{
  active_ = b;
}

void
Control2D::
toggleShown()
{
  setShown(! isShown());
}

void
Control2D::
setShown(bool shown)
{
  if (shown == shown_)
    return;

  shown_ = ! shown_;

  //---

  auto *app = canvas_->app();

  auto geom = app->geometry();

  int w = this->sizeHint().width();

  QRect geom1;

  if (shown_) {
    geom1 = QRect(geom.x(), geom.y(), geom.width() + w + 6, geom.height());

    this->updateWidgets();
    this->show();
  }
  else {
    geom1 = QRect(geom.x(), geom.y(), geom.width() - w - 6, geom.height());

    this->hide();
  }

  app->setGeometry(geom1);

  if (shown_)
    this->setFixedWidth(w);
  else {
    this->setMinimumWidth(0);
    this->setMaximumWidth(QWIDGETSIZE_MAX);
  }

  Q_EMIT shownStateChanged();
}

void
Control2D::
objectSelectedSlot(QListWidgetItem *, QListWidgetItem *)
{
  updateCurrent();
}

void
Control2D::
objectChangedSlot(QObject *, const QString &)
{
  canvas_->update();
}

void
Control2D::
updateWidgets()
{
  if (! active_)
    return;

  connectObjects(false);

  objectsData_.list->clear();

  if (canvas_) {
    auto *viewport = canvas_->currentViewport();

    if (viewport) {
      for (auto *obj : viewport->objects)
        objectsData_.list->addItem(obj->calcId());
    }
  }

  connectObjects(true);

  updateCurrent();
}

void
Control2D::
updateCurrent()
{
  auto *obj = getCurrentObject();

  auto skipPropeties = QStringList();

  objectsData_.tree->clear();

  if (obj) {
    auto properties = CQUtil::getPropertyList(obj);

    for (auto &prop : properties) {
      if (skipPropeties.contains(prop))
        continue;

      objectsData_.tree->addProperty("", obj, prop);
    }
  }
}

Object2D *
Control2D::
getCurrentObject() const
{
  auto *item = objectsData_.list->currentItem();
  if (! item) return nullptr;

  auto id = item->text();

  if (canvas_)
    return canvas_->getObjectByName(id);

  return nullptr;
}

//---

bool
Control2D::
createUi(const QString &ui)
{
  if (! xml_) {
    xml_ = new Xml2D(this);

    CQXmlAddWidgetFactoryT(xml_, CQTclIntegerSpin);
  }

  return xml_->createWidgetsFromString(uiFrame_, ui.toStdString());
}

bool
Control2D::
getUiValue(const QString &name, QVariant &value) const
{
  if (! xml_) return false;

  value = xml_->getExecData(name);

  return true;
}

bool
Control2D::
setUiValue(const QString &name, const QVariant &value)
{
  if (! xml_) return false;

  xml_->setExecData(name, value);

  return true;
}

bool
Control2D::
getUiWidgetValue(const QString &widget, const QString &name, QVariant &value) const
{
  if (! xml_) return false;

  auto *w = xml_->getWidget(widget);
  if (! w) return false;

  if (! xml_->getWidgetData(w, name, value))
    return false;

  return true;
}

bool
Control2D::
setUiWidgetValue(const QString &widget, const QString &name, const QVariant &value)
{
  if (! xml_) return false;

  auto *w = xml_->getWidget(widget);
  if (! w) return false;

  if (! xml_->setWidgetData(w, name, value))
    return false;

  return true;
}

}
