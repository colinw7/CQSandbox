#include <CQSandboxTextObj.h>
#include <CQSandboxCanvas.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

#include <CQHtmlTextPainter.h>

#include <QPainter>

namespace CQSandbox {

bool
TextObj::
create(Canvas *canvas, const QStringList &args)
{
  auto *tcl = canvas->app()->tcl();

  Point   pos;
  QString text;

  if      (args.size() >= 2) {
    pos  = Util::stringToPoint(tcl, args[0]);
    text = args[1];
  }
  else if (args.size() >= 1) {
    pos= Util::stringToPoint(tcl, args[0]);
  }

  auto *obj = new TextObj(canvas, pos, text);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

TextObj::
TextObj(Canvas *canvas, const Point &pos, const QString &text) :
 Object(canvas), pos_(pos), text_(text)
{
  font_ = canvas->font();
}

bool
TextObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "position")
    value = Util::pointToString(pos_);
  else if (name == "text")
    value = text_;
  else if (name == "align")
    value = Util::alignToString(align_);
  else if (name == "html")
    value = html_;
  else
    return Object::getValue(name, args, value);

  return true;
}

bool
TextObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->app()->tcl();

  if      (name == "position")
    pos_ = Util::stringToPoint(tcl, value);
  else if (name == "text")
    text_ = value;
  else if (name == "align")
    align_ = Util::stringToAlign(value);
  else if (name == "html")
    html_ = Util::stringToBool(value);
  else if (name == "border.color")
    border_.setColor(Util::stringToColor(tcl, value));
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
TextObj::
calcRect() const
{
  QSizeF s;

  if (html_) {
    CQHtmlTextPainter textPainter;

    textPainter.setText(text_);

    textPainter.setMargin(4);
    textPainter.setFont(font_);

    s = canvas()->pixelSizeToWindow(textPainter.textSize());
  }
  else {
    QFontMetrics fm(font_);

    int w = fm.horizontalAdvance(text_);
    int h = fm.height();

    s = canvas()->pixelSizeToWindow(QSizeF(w, h));
  }

  auto p = pointToWindow(pos_);

  double x = p.x.value;
  double y = p.y.value;

  if      (align_ & Qt::AlignRight  ) x -= s.width();
  else if (align_ & Qt::AlignHCenter) x -= s.width()/2.0;

  if      (align_ & Qt::AlignBottom ) y -= s.height();
  else if (align_ & Qt::AlignVCenter) y -= s.height()/2.0;

  auto ll = Point(Coord(x            ), Coord(y             ));
  auto ur = Point(Coord(x + s.width()), Coord(y + s.height()));

  return Rect(ll, ur);
}

void
TextObj::
draw(QPainter *painter)
{
  auto rect  = calcRect();
  auto prect = canvas()->rectToPixel(rect).qrect();

  // draw border
  painter->setPen(border_);
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(prect);

  // draw text
  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  if (isHtml()) {
    CQHtmlTextPainter textPainter;

    textPainter.setText(text_);

    textPainter.setMargin(4);
    textPainter.setAlignment(align_);
    textPainter.setTextColor(brush_.value().color());
    textPainter.setFont(font_);

    textPainter.drawInRect(painter, prect.toRect());
  }
  else {
    QFontMetrics fm(font_);

    painter->setFont(font_);

    painter->drawText(prect.left(), prect.top() + fm.ascent(), text_);
  }
}

}
