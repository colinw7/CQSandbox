#ifndef ObjectsList_H
#define ObjectsList_H

#include <QFrame>
#include <QItemDelegate>
#include <QHeaderView>

class CQTreeWidget;
class QTreeWidgetItem;

namespace CQSandbox {

class Canvas3D;
class Object3D;

class ObjectsListDelegate;
class ObjectsListHeader;

class ObjectsList : public QFrame {
  Q_OBJECT

 public:
  ObjectsList(Canvas3D *canvas);

  CQTreeWidget *tree() const { return tree_; }

  const QIcon &selectedIcon() const { return selectedIcon_; }
  const QIcon &deselectedIcon() const { return deselectedIcon_; }

  const QIcon &visibleIcon() const { return visibleIcon_; }
  const QIcon &invisibleIcon() const { return invisibleIcon_; }

  void connectSlots(bool b);

  int selectedInd() const { return selectedInd_; }

  bool anyVisible() const { return anyVisible_; }
  bool anySelected() const { return anySelected_; }

  Object3D *getSelectedIndObject() const;
  Object3D *getObjectListSelected() const;

  Object3D *objectFromIndex(const QModelIndex &index) const;

  void resizeEvent(QResizeEvent *) override;

  void paintEvent(QPaintEvent *e) override;

  void updateState();

  void selectAllObjects(bool);
  void showAllObjects  (bool);

  void updateAll();

 public Q_SLOTS:
  void updateObjects();

 private Q_SLOTS:
  void itemSelectedSlot(QTreeWidgetItem *, QTreeWidgetItem *);
  void treeClickSlot(const QModelIndex &);

 private:
  QTreeWidgetItem *createObjectItem(Object3D *object);

 Q_SIGNALS:
  void currentItemChanged();

 private:
  using ObjectItem = std::map<Object3D *, QTreeWidgetItem *>;

  Canvas3D*            canvas_      { nullptr };
  CQTreeWidget*        tree_        { nullptr };
  ObjectsListDelegate* delegate_    { nullptr };
  ObjectsListHeader*   header_      { nullptr };
  ObjectItem           objectItem_;
  int                  selectedInd_ { -1 };

  bool anyVisible_  { false };
  bool anySelected_ { false };

  QIcon selectedIcon_;
  QIcon deselectedIcon_;

  QIcon visibleIcon_;
  QIcon invisibleIcon_;
};

//---

class ObjectsListDelegate : public QItemDelegate {
  Q_OBJECT

 public:
  ObjectsListDelegate(ObjectsList *list);

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override;

 private:
  ObjectsList *list_ { nullptr };
};

//---

class ObjectsListHeader : public QHeaderView {
  Q_OBJECT

 public:
  ObjectsListHeader(ObjectsList *list);

  int iconWidth() const { return iconWidth_; }

  void paintEvent(QPaintEvent *e) override;

  void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

  QSize sectionSizeFromContents(int logicalIndex) const override;

  void contextMenuEvent(QContextMenuEvent *event) override;

 public Q_SLOTS:
  void clickSlot(int);

  void updateColumnSizes();

 private:
  ObjectsList* list_      { nullptr };
  mutable int            iconWidth_ { 16 };

  int menuSection_ { -1 };
};

}

#endif
