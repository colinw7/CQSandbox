#ifndef CQSandboxTextObj_H
#define CQSandboxTextObj_H

#include <CQSandboxObject.h>

#include <QFont>

class CQTextModel;

namespace CQSandbox {

class TextObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  TextObj(Canvas *canvas, const Point &pos, const QString &text);

  const char *typeName() const override { return "text"; }

  const Point &position() const { return pos_; }
  void setPosition(const Point &v) { pos_ = v; }

  const QString &text() const { return text_; }
  void setText(const QString &s) { text_ = s; }

  const QFont &font() const { return font_; }
  void setFont(const QFont &f) { font_ = f; }

  bool isHtml() const { return html_; }
  void setHtml(bool b) { html_ = b; }

  const Qt::Alignment &align() const { return align_; }
  void setAlign(const Qt::Alignment &v) { align_ = v; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point         pos_;
  QString       text_;
  QFont         font_;
  QPen          border_;
  Qt::Alignment align_ { Qt::AlignCenter };
  bool          html_  { false };
};

}

#endif
