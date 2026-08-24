#include <CQSandboxControl2D.h>
#include <CQSandboxCanvas.h>
#include <CQSandboxApp.h>
#include <CQSandboxViewport.h>

#include <CQXml.h>

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

  void execSlot(const QString &value, const QString &) override {
    control_->canvas()->app()->runTclCmd(value);
  }

 private:
  Control2D* control_ { nullptr };
};

}

namespace CQSandbox {

Control2D::
Control2D(Canvas *canvas) :
 canvas_(canvas)
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

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

  updateObjects();

  if (canvas_)
    connect(canvas_, &Canvas::objectsChanged, this, &Control2D::updateObjects);

  connect(list_, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
          this, SLOT(listItemSlot(QListWidgetItem *, QListWidgetItem *)));

  connect(visibleCheck_, SIGNAL(stateChanged(int)), this, SLOT(visibleSlot(int)));
}

void
Control2D::
setActive(bool b)
{
  active_ = b;
}

void
Control2D::
listItemSlot(QListWidgetItem *, QListWidgetItem *)
{
  updateCurrent();
}

void
Control2D::
updateObjects()
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

Object *
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
setUi(const QString &ui)
{
  if (! xml_)
    xml_ = new Xml2D(this);

  return xml_->createWidgetsFromString(uiFrame_, ui.toStdString());
}

}
