#ifndef CQBBox3DEdit_H
#define CQBBox3DEdit_H

#include <CBBox3D.h>

#include <QFrame>

class CQPoint3DEdit;

class QLineEdit;
class QBoxLayout;

class CQBBox3DEdit : public QFrame {
  Q_OBJECT

 public:
  enum class Mode {
    TEXT_EDIT,
    MIN_MAX_POINT
  };

  static CBBox3D defValue() { return CBBox3D(CPoint3D(0, 0, 0), CPoint3D(1, 1, 1)); }

 public:
  CQBBox3DEdit(QWidget *parent, const CBBox3D &value=defValue());
  CQBBox3DEdit(const CBBox3D &value=defValue());

  virtual ~CQBBox3DEdit() { }

  void setValue(const CBBox3D &rect);

  const CBBox3D &getValue() const;

 private:
  void init(const CBBox3D &value);

 private Q_SLOTS:
  void editingFinishedI();

 signals:
  void valueChanged();

 private:
  void updateRange();

  void rectToWidget();
  bool widgetToPoint();

 private:
  CBBox3D rect_ { defValue() };
  Mode    mode_ { Mode::MIN_MAX_POINT };

  QLineEdit*     edit_    { nullptr };
  CQPoint3DEdit* minEdit_ { nullptr };
  CQPoint3DEdit* maxEdit_ { nullptr };

  QBoxLayout*  layout_         { nullptr };
  mutable bool disableSignals_ { false };
};

#endif
