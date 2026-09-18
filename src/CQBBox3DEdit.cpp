#include <CQBBox3DEdit.h>

#include <CQPoint3DEdit.h>

#include <QLabel>

CQBBox3DEdit::
CQBBox3DEdit(QWidget *parent, const CBBox3D &value) :
 QFrame(parent)
{
  init(value);
}

CQBBox3DEdit::
CQBBox3DEdit(const CBBox3D &value) :
 QFrame(nullptr)
{
  init(value);
}

void
CQBBox3DEdit::
init(const CBBox3D &value)
{
  setObjectName("edit");

  setFrameStyle(uint(QFrame::NoFrame) | uint(QFrame::Plain));

  layout_ = new QVBoxLayout(this);
  layout_->setMargin(0); layout_->setSpacing(0);

  //---

  if      (mode_ == Mode::TEXT_EDIT) {
    edit_ = new QLineEdit;

    edit_->setObjectName("edit");
    edit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    connect(edit_, SIGNAL(editingFinished()), this, SLOT(editingFinishedI()));

    layout_->addWidget(edit_);
  }
  else if (mode_ == Mode::MIN_MAX_POINT) {
    auto addEdit = [&](const char *name, const char *text) {
      auto *elayout = new QHBoxLayout;

      auto *label = new QLabel(text);
      auto *edit  = new CQPoint3DEdit;

      edit->setObjectName(name);
      edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

      connect(edit, SIGNAL(editingFinished()), this, SLOT(editingFinishedI()));

      elayout->addWidget(label);
      elayout->addWidget(edit);

      layout_->addLayout(elayout);

      return edit;
    };

    minEdit_ = addEdit("minEdit", "Min");
    maxEdit_ = addEdit("maxEdit", "Max");
  }

  //---

  setValue(value);
}

void
CQBBox3DEdit::
setValue(const CBBox3D &rect)
{
  rect_ = rect;

  rectToWidget();
}

const CBBox3D &
CQBBox3DEdit::
getValue() const
{
  return rect_;
}

void
CQBBox3DEdit::
editingFinishedI()
{
  if (disableSignals_) return;

  widgetToPoint();

  Q_EMIT valueChanged();
}

void
CQBBox3DEdit::
rectToWidget()
{
  disableSignals_ = true;

  if      (mode_ == Mode::TEXT_EDIT) {
    auto str = QString("%1 %2 %3 %4 %5 %6").
      arg(rect_.getXMin()).arg(rect_.getYMin()).arg(rect_.getZMin()).
    arg(rect_.getXMax()).arg(rect_.getYMax()).arg(rect_.getZMax());

    edit_->setText(str);
  }
  else if (mode_ == Mode::MIN_MAX_POINT) {
    minEdit_->setValue(rect_.getMin());
    maxEdit_->setValue(rect_.getMax());
  }

  disableSignals_ = false;
}

bool
CQBBox3DEdit::
widgetToPoint()
{
  if      (mode_ == Mode::TEXT_EDIT) {
    auto toReal = [](const std::string &s, double *r) {
      bool ok = true;

      *r = 0.0;

      try {
        *r = std::stod(s);
      }
      catch (...) {
        ok = false;
      }

      return ok;
    };

    auto strs = edit_->text().split(" ", Qt::SkipEmptyParts);

    if (strs.length() != 6)
      return false;

    double x1, y1, z1, x2, y2, z2;

    if (! toReal(strs[0].toStdString(), &x1)) return false;
    if (! toReal(strs[1].toStdString(), &y1)) return false;
    if (! toReal(strs[2].toStdString(), &z1)) return false;
    if (! toReal(strs[3].toStdString(), &x2)) return false;
    if (! toReal(strs[4].toStdString(), &y2)) return false;
    if (! toReal(strs[5].toStdString(), &z2)) return false;

    rect_ = CBBox3D(CPoint3D(x1, y1, z1), CPoint3D(x2, y2, z2));
  }
  else if (mode_ == Mode::MIN_MAX_POINT) {
    rect_ = CBBox3D(minEdit_->getValue(), maxEdit_->getValue());
  }

  return true;
}
