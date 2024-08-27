#ifndef CQSandboxEditor_H
#define CQSandboxEditor_H

#include <QFrame>

class QListWidget;
class QListWidgetItem;
class QCheckBox;

namespace CQSandbox {

class Canvas;
class Object;

class Editor : public QFrame {
  Q_OBJECT

 public:
  Editor(Canvas *canvas);

 public Q_SLOTS:
  void updateObjects();

 private Q_SLOTS:
  void listItemSlot(QListWidgetItem *, QListWidgetItem *);

  void visibleSlot(int);

 private:
  void updateCurrent();

  Object *getCurrentObject() const;

 private:
  Canvas* canvas_ { nullptr };

  QListWidget* list_ { nullptr };
  QCheckBox*   visibleCheck_ { nullptr };
};

}

#endif
