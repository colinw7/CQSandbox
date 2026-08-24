#ifndef CQSandboxControl2D_H
#define CQSandboxControl2D_H

#include <QFrame>

class CQXml;

class QListWidget;
class QListWidgetItem;
class QCheckBox;
class QFrame;

namespace CQSandbox {

class Canvas;
class Object;

class Control2D : public QFrame {
  Q_OBJECT

 public:
  Control2D(Canvas *canvas);

  Canvas *canvas() const { return canvas_; }

  QFrame *uiFrame() const { return uiFrame_; }

  bool isActive() const { return active_; }
  void setActive(bool b);

  bool setUi(const QString &ui);

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

  QListWidget* list_         { nullptr };
  QCheckBox*   visibleCheck_ { nullptr };
  QFrame*      uiFrame_      { nullptr };
  CQXml*       xml_          { nullptr };
  bool         active_       { true };
};

}

#endif
