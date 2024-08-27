#include <CQSandboxEditor.h>
#include <CQSandbox.h>

#include <QListWidget>
#include <QCheckBox>
#include <QVBoxLayout>

namespace CQSandbox {

Editor::
Editor(Canvas *canvas) :
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

  connect(canvas_, SIGNAL(objectsChanged()), this, SLOT(updateObjects()));

  connect(list_, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
          this, SLOT(listItemSlot(QListWidgetItem *, QListWidgetItem *)));

  connect(visibleCheck_, SIGNAL(stateChanged(int)), this, SLOT(visibleSlot(int)));
}

void
Editor::
listItemSlot(QListWidgetItem *, QListWidgetItem *)
{
  updateCurrent();
}

void
Editor::
updateObjects()
{
  disconnect(list_, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
             this, SLOT(listItemSlot(QListWidgetItem *, QListWidgetItem *)));

  list_->clear();

  for (auto *obj : canvas_->objects())
    list_->addItem(obj->calcId());

  connect(list_, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
          this, SLOT(listItemSlot(QListWidgetItem *, QListWidgetItem *)));

  updateCurrent();
}

void
Editor::
updateCurrent()
{
  auto *obj = getCurrentObject();

  visibleCheck_->setEnabled(obj);

  if (obj)
    visibleCheck_->setChecked(obj->isVisible());
}

void
Editor::
visibleSlot(int i)
{
  auto *obj = getCurrentObject();

  if (obj)
    obj->setVisible(i);
}


Object *
Editor::
getCurrentObject() const
{
  auto *item = list_->currentItem();
  if (! item) return nullptr;

  auto id = item->text();

  return canvas_->getObjectByName(id);
}

}
