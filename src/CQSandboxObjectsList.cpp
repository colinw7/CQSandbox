#include <CQSandboxObjectsList.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>

#include <CQUtil.h>
#include <CQTreeWidget.h>
#include <CQPixmapCache.h>

#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QContextMenuEvent>
#include <QPainter>

namespace CQSandbox {

ObjectsList::
ObjectsList(Canvas3D *canvas) :
 QFrame(nullptr), canvas_(canvas)
{
  setObjectName("objectsList");

  //---

  auto *layout = new QVBoxLayout(this);

  tree_ = new CQTreeWidget;

  tree_->setObjectName("tree");
  tree_->setSelectionMode(QTreeWidget::SingleSelection);

  tree_->setHeaderLabels(QStringList() << "Name" << "Selected" << "Visible");

  layout->addWidget(tree_);

  //---

  selectedIcon_   = CQPixmapCacheInst->getPixmap("SELECTED");
  deselectedIcon_ = CQPixmapCacheInst->getPixmap("DESELECTED");

  visibleIcon_   = CQPixmapCacheInst->getPixmap("VISIBLE");
  invisibleIcon_ = CQPixmapCacheInst->getPixmap("INVISIBLE");

  //---

  delegate_ = new ObjectsListDelegate(this);

  tree_->setItemDelegate(delegate_);

  header_ = new ObjectsListHeader(this);

  tree_->setHeader(header_);

  header_->setSectionsClickable(true);

  //---

  connectSlots(true);
}

void
ObjectsList::
resizeEvent(QResizeEvent *)
{
  header_->updateColumnSizes();
}

void
ObjectsList::
updateObjects()
{
  connectSlots(false);

  //---

  tree_->clear();

  objectItem_.clear();

  auto *item = new QTreeWidgetItem(tree_, QStringList());

  tree_->addTopLevelItem(item);

  item->setData(0, Qt::UserRole, -1);

  for (auto *object : canvas_->objects()) {
    auto *item = createObjectItem(object);

    item->setData(0, Qt::UserRole, int(object->ind()));
  }

  //---

  header_->updateColumnSizes();

  //---

  connectSlots(true);
}

QTreeWidgetItem *
ObjectsList::
createObjectItem(Object3D *object)
{
  auto pi = objectItem_.find(object);

  if (pi != objectItem_.end())
    return (*pi).second;

  auto name = object->id();

  name += QString(" (%1)").arg(object->ind());

  auto *parent = object->parent();

  QTreeWidgetItem *item;

  if (parent) {
    auto *parentItem = createObjectItem(parent);

    item = new QTreeWidgetItem(parentItem, QStringList() << name);

    parentItem->addChild(item);
  }
  else {
    item = new QTreeWidgetItem(tree_, QStringList() << name);

    tree_->addTopLevelItem(item);
  }

  objectItem_[object] = item;

  //---

  for (auto *child : object->children())
    createObjectItem(child);

  return item;
}

void
ObjectsList::
paintEvent(QPaintEvent *e)
{
  updateState();

  QFrame::paintEvent(e);
}

void
ObjectsList::
updateState()
{
  anyVisible_  = false;
  anySelected_ = false;

  for (auto *object : canvas_->objects()) {
    if (object->isVisible())
      anyVisible_ = true;

    if (object->isSelected())
      anySelected_ = true;
  }
}

void
ObjectsList::
connectSlots(bool b)
{
  CQUtil::connectDisconnect(b, canvas_,
    SIGNAL(objectsChanged()), this, SLOT(updateObjects()));

  CQUtil::connectDisconnect(b, tree_,
    SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
    this, SLOT(itemSelectedSlot(QTreeWidgetItem *, QTreeWidgetItem *)));

  CQUtil::connectDisconnect(b, tree_,
    SIGNAL(clicked(const QModelIndex &)), this, SLOT(treeClickSlot(const QModelIndex &)));
}

void
ObjectsList::
itemSelectedSlot(QTreeWidgetItem *item, QTreeWidgetItem *)
{
  selectedInd_ = (item ? item->data(0, Qt::UserRole).toInt() : -1);
//std::cerr << selectedInd_ << "\n";

  Q_EMIT currentItemChanged();
}

void
ObjectsList::
treeClickSlot(const QModelIndex &index)
{
  auto *object = objectFromIndex(index);
  if (! object) return;

  if      (index.column() == 1) {
    if (! object->isSelected())
      canvas_->selectObject(object, /*clear*/false, /*update*/true);
    else
      canvas_->deselectObject(object, /*update*/true);
  }
  else if (index.column() == 2)
    object->setVisible(! object->isVisible());
  else
    return;

  updateAll();
}

void
ObjectsList::
updateAll()
{
  tree_->update();
  tree_->viewport()->update();

  header_->viewport()->update();
  header_->update();

  canvas_->update();
}

Object3D *
ObjectsList::
getSelectedIndObject() const
{
  if (selectedInd_ < 0) return nullptr;

  return canvas_->getObjectByInd(selectedInd_);
}

Object3D *
ObjectsList::
getObjectListSelected() const
{
  auto selectedItems = tree_->selectedItems();

  int ind = -1;

  for (const auto *item : selectedItems) {
    auto ind1 = item->data(0, Qt::UserRole).toInt();

    if (ind1 >= 0) {
      ind = ind1;
      break;
    }
  }

  if (ind < 0) return nullptr;

  return canvas_->getObjectByInd(ind);
}

Object3D *
ObjectsList::
objectFromIndex(const QModelIndex &index) const
{
  auto *item = tree_->itemFromIndex(index);

  auto ind = item->data(0, Qt::UserRole).toInt();
  if (ind < 0) return nullptr;

  return canvas_->getObjectByInd(ind);
}

void
ObjectsList::
selectAllObjects(bool select)
{
  if (select)
    canvas_->selectAllObjects();
  else
    canvas_->deselectAll();

  updateAll();
}

void
ObjectsList::
showAllObjects(bool visible)
{
  for (auto *object : canvas_->objects()) {
    object->setVisible(visible);
  }

  updateAll();
}

//---

ObjectsListDelegate::
ObjectsListDelegate(ObjectsList *list) :
 QItemDelegate(list->tree()), list_(list)
{
  setObjectName("delegate");
}

void
ObjectsListDelegate::
paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  auto *object = list_->objectFromIndex(index);

  if (! object)
    return QItemDelegate::paint(painter, option, index);

  QPixmap pm;

  if      (index.column() == 1) {
    bool selected = (object && object->isSelected());

    if (selected)
      pm = list_->selectedIcon().pixmap(option.rect.size());
    else
      pm = list_->deselectedIcon().pixmap(option.rect.size());
  }
  else if (index.column() == 2) {
    bool visible = (object && object->isVisible());

    if (visible)
      pm = list_->visibleIcon().pixmap(option.rect.size());
    else
      pm = list_->invisibleIcon().pixmap(option.rect.size());
  }
  else
    QItemDelegate::paint(painter, option, index);

  if (! pm.isNull())
    painter->drawPixmap(option.rect.topLeft(), pm);
}

//---

ObjectsListHeader::
ObjectsListHeader(ObjectsList *list) :
 QHeaderView(Qt::Horizontal, list->tree()), list_(list)
{
  setObjectName("objectListHeader");

  QFontMetrics fm(font());
  iconWidth_ = fm.horizontalAdvance("X") + 12;

  setMinimumSectionSize(iconWidth_);

  connect(this, SIGNAL(sectionClicked(int)), this, SLOT(clickSlot(int)));

  setContextMenuPolicy(Qt::DefaultContextMenu);
}

void
ObjectsListHeader::
paintEvent(QPaintEvent *e)
{
  list_->updateState();

  QHeaderView::paintEvent(e);
}

void
ObjectsListHeader::
paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
  if (logicalIndex == 0)
    return QHeaderView::paintSection(painter, rect, logicalIndex);

  QPixmap pm;

  if (logicalIndex == 1) {
    if (list_->anySelected())
      pm = list_->selectedIcon().pixmap(rect.size());
    else
      pm = list_->deselectedIcon().pixmap(rect.size());
  }
  else {
    if (list_->anyVisible())
      pm = list_->visibleIcon().pixmap(rect.size());
    else
      pm = list_->invisibleIcon().pixmap(rect.size());
  }

  if (! pm.isNull())
    painter->drawPixmap(rect.topLeft(), pm);
}

QSize
ObjectsListHeader::
sectionSizeFromContents(int logicalIndex) const
{
  auto s = QHeaderView::sectionSizeFromContents(logicalIndex);

  iconWidth_ = s.height() - 4;

  if (logicalIndex != 0)
    s.setWidth(iconWidth_);

  return s;
}

void
ObjectsListHeader::
updateColumnSizes()
{
  int w = width() - 2*iconWidth();

  resizeSection(1, iconWidth());
  resizeSection(2, iconWidth());
  resizeSection(0, w);
}

void
ObjectsListHeader::
clickSlot(int section)
{
  if      (section == 1) {
    list_->selectAllObjects(! list_->anySelected());
  }
  else if (section == 2) {
    list_->showAllObjects(! list_->anyVisible());
  }
}

void
ObjectsListHeader::
contextMenuEvent(QContextMenuEvent *event)
{
  QMenu menu(this);

  menu.addAction("Fit", this, SLOT(updateColumnSizes()));

  menuSection_ = logicalIndexAt(event->pos());

  menu.exec(event->globalPos());
}

}
