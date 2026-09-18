#include <CQSandboxControl2D.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxViewport.h>

#include <CQXml.h>
#include <CQTclUtil.h>
#include <CQTclIntegerSpin.h>
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

  auto *objectsFrame  = new QFrame;
  auto *objectsLayout = new QVBoxLayout(objectsFrame);

  tab->addTab(objectsFrame, "Objects");

  list_ = new QListWidget;

  list_->setSelectionMode(QAbstractItemView::SingleSelection);

  objectsLayout->addWidget(list_);

  visibleCheck_ = new QCheckBox("Visible");

  objectsLayout->addWidget(visibleCheck_);

  //---

  uiFrame_ = new QFrame;

  tab->addTab(uiFrame_, "UI");

  //---

  updateWidgets();

  //---

  if (canvas_)
    connect(canvas_, &Canvas2D::objectsChanged, this, &Control2D::updateWidgets);

  connect(list_, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
          this, SLOT(listItemSlot(QListWidgetItem *, QListWidgetItem *)));

  connect(visibleCheck_, SIGNAL(stateChanged(int)), this, SLOT(visibleSlot(int)));
}

Control2D::
~Control2D()
{
  delete xml_;
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
listItemSlot(QListWidgetItem *, QListWidgetItem *)
{
  updateCurrent();
}

void
Control2D::
updateWidgets()
{
  if (! active_)
    return;

  disconnect(list_, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
             this, SLOT(listItemSlot(QListWidgetItem *, QListWidgetItem *)));

  list_->clear();

  if (canvas_) {
    auto *viewport = canvas_->currentViewport();

    if (viewport) {
      for (auto *obj : viewport->objects)
        list_->addItem(obj->calcId());
    }
  }

  connect(list_, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
          this, SLOT(listItemSlot(QListWidgetItem *, QListWidgetItem *)));

  updateCurrent();
}

void
Control2D::
updateCurrent()
{
  auto *obj = getCurrentObject();

  visibleCheck_->setEnabled(obj);

  if (obj)
    visibleCheck_->setChecked(obj->isVisible());
}

void
Control2D::
visibleSlot(int i)
{
  auto *obj = getCurrentObject();

  if (obj)
    obj->setVisible(i);
}

Object2D *
Control2D::
getCurrentObject() const
{
  auto *item = list_->currentItem();
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
