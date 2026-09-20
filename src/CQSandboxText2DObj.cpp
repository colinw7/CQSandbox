#include <CQSandboxText2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

#include <CQHtmlTextPainter.h>

#include <QPainter>

namespace CQSandbox {

bool
Text2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  auto *tcl = canvas->tcl();

  Point2D pos;
  QString text;

  if      (args.size() >= 2) {
    if (! Util::stringToPoint2D(tcl, args[0], pos))
      return false;

    text = args[1];
  }
  else if (args.size() >= 1) {
    if (! Util::stringToPoint2D(tcl, args[0], pos))
      return false;
  }

  //---

  auto pos1 = canvas->pointToWindow(pos).point();

  auto *obj = new Text2DObj(canvas, pos1, text);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

Text2DObj::
Text2DObj(Canvas2D *canvas, const CPoint2D &pos, const QString &text) :
 Object2D(canvas, Type::TEXT), text_(text)
{
  position_ = pos;
  font_     = canvas->font();
}

bool
Text2DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "position") {
    value = Util::point2DToString(Point2D::makeWindow(position_));
  }
  else if (name == "text") {
    value = text_;
  }
  else if (name == "align") {
    value = Util::alignToString(align_);
  }
  else if (name == "html") {
    value = html_;
  }
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
Text2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

  if      (name == "position") {
    Point2D p;
    if (! Util::stringToPoint2D(tcl, value, p))
      return false;

    position_ = canvas()->pointToWindow(p).point();
  }
  else if (name == "text") {
    text_ = value;
  }
  else if (name == "align") {
    align_ = Util::stringToAlign(value);
  }
  else if (name == "html") {
    html_ = Util::stringToBool(value);
  }
  else if (name == "border.color") {
    border_.setColor(Util::stringToColor(tcl, value));
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

Rect2D
Text2DObj::
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

  auto p = pointToWindow(Point2D::makeWindow(position_));

  double x = p.x.value;
  double y = p.y.value;

  if      (align_ & Qt::AlignRight  ) x -= s.width();
  else if (align_ & Qt::AlignHCenter) x -= s.width()/2.0;

  if      (align_ & Qt::AlignBottom ) y -= s.height();
  else if (align_ & Qt::AlignVCenter) y -= s.height()/2.0;

  auto ll = Point2D(Coord(x            ), Coord(y             ));
  auto ur = Point2D(Coord(x + s.width()), Coord(y + s.height()));

  return Rect2D(ll, ur);
}

void
Text2DObj::
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
