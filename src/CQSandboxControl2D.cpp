#include <CQSandboxControl2D.h>
#include <CQSandboxCanvas.h>

#include <QListWidget>
#include <QCheckBox>
#include <QVBoxLayout>

namespace CQSandbox {

Control2D::
Control2D(Canvas *canvas) :
 canvas_(canvas)
{
  auto *layout = new QVBoxLayout(this);

  list_ = new QListWidget;

  list_->setSelectionMode(QAbstractItemView::SingleSelection);

  layout->addWidget(list_);

  visibleCheck_ = new QCheckBox("Visible");

  layout->addWidget(visibleCheck_);

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
listItemSlot(QListWidgetItem *, QListWidgetItem *)
{
  updateCurrent();
}

void
Control2D::
updateObjects()
{
  disconnect(list_, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
             this, SLOT(listItemSlot(QListWidgetItem *, QListWidgetItem *)));

  list_->clear();

  if (canvas_) {
    for (auto *obj : canvas_->objects())
      list_->addItem(obj->calcId());
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

}
