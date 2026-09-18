#ifndef CQSandboxControl2D_H
#define CQSandboxControl2D_H

#include <QFrame>

class CQPropertyViewTree;
class CQXml;

class QListWidget;
class QListWidgetItem;
class QCheckBox;
class QFrame;

namespace CQSandbox {

class Canvas2D;
class Object2D;

class Control2D : public QFrame {
  Q_OBJECT

 public:
  Control2D(Canvas2D *canvas);
 ~Control2D() override;

  Canvas2D *canvas() const { return canvas_; }

  QFrame *uiFrame() const { return uiFrame_; }

  bool isActive() const { return active_; }
  void setActive(bool b);

  bool isShown() const { return shown_; }
  void setShown(bool b);

  bool createUi(const QString &ui);
  bool getUiValue(const QString &name, QVariant &value) const;
  bool setUiValue(const QString &name, const QVariant &value);
  bool getUiWidgetValue(const QString &widget, const QString &name, QVariant &value) const;
  bool setUiWidgetValue(const QString &widget, const QString &name, const QVariant &value);

  void toggleShown();

 Q_SIGNALS:
  void shownStateChanged();

 public Q_SLOTS:
  void updateWidgets();

 private Q_SLOTS:
  void objectSelectedSlot(QListWidgetItem *, QListWidgetItem *);
  void objectChangedSlot(QObject *object, const QString &name);

 private:
  QFrame *addObjectsFrame();

  void connectObjects(bool);

  void updateCurrent();

  Object2D *getCurrentObject() const;

 private:
  Canvas2D* canvas_ { nullptr };

  struct ObjectsData {
    QListWidget*        list { nullptr };
    CQPropertyViewTree* tree { nullptr };
  };

  ObjectsData objectsData_;

  QFrame* uiFrame_ { nullptr };
  CQXml*  xml_     { nullptr };

  bool active_ { true };
  bool shown_  { false };
};

}

#endif
