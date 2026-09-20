#include <CQSandboxImage2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

#include <QPainter>
#include <QBitmap>

namespace CQSandbox {

bool
Image2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  auto *tcl = canvas->tcl();

  Point2D pos;
  QImage  image;

  if (args.size() >= 1) {
    if (! Util::stringToPoint2D(tcl, args[0], pos)) {
      if (args.size() == 1) {
        if (! Util::stringToImage(args[0], image))
          return false;
      }
      else
        return false;
    }
  }

  if (args.size() >= 2) {
    if (args[1] != "") {
      if (! Util::stringToImage(args[1], image))
        return false;
    }
  }

  //---

  auto pos1 = canvas->pointToWindow(pos).point();

  auto *obj = new Image2DObj(canvas, pos1, image);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

Image2DObj::
Image2DObj(Canvas2D *canvas, const CPoint2D &pos, const QImage &image) :
 Object2D(canvas, Type::IMAGE), image_(image)
{
  position_ = pos;
}

bool
Image2DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *tcl = canvas()->tcl();

  if      (name == "position") {
    value = Util::point2DToString(Point2D::makeWindow(position_));
  }
  else if (name == "center") {
    auto ppos = pointToPixel(Point2D::makeWindow(position_));

    ppos.x.value += image_.width ()/2;
    ppos.y.value += image_.height()/2;

    value = Util::point2DToString(ppos);
  }
  else if (name == "size") {
    auto p = CPoint2D(image_.width(), image_.height());

    value = Util::point2DToString(p);
  }
  else if (name == "image") {
    value = Util::imageToString(image_);
  }
  else if (name == "sub_image") {
    // get new image from part of this this
    if (args.size() < 1)
      return false;

    CBBox2D bbox;
    if (! Util::stringToBBox2D(tcl, args[0], bbox))
      return false;

    auto image1 = image_.copy(bbox.getXMin(), bbox.getYMin(), bbox.getWidth(), bbox.getHeight());

    auto *obj = new Image2DObj(canvas(), position_, image1);

    auto name = canvas()->addNewObject(obj);

    value = name;
  }
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
Image2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (name == "position") {
    Point2D p;
    if (! Util::stringToPoint2D(tcl, value, p))
      return false;

    position_ = canvas()->pointToWindow(p).point();
    posType_  = Position::TOP_LEFT;
  }
  else if (name == "center") {
    Point2D p;
    if (! Util::stringToPoint2D(tcl, value, p))
      return false;

    position_ = canvas()->pointToWindow(p).point();
    posType_  = Position::CENTER;
  }
  else if (name == "rect") {
    rect_    = Util::stringToRect2D(tcl, value);
    posType_ = Position::RECT;
  }
  else if (name == "image") {
    if (value != "") {
      if (! Util::stringToImage(value, image_)) {
        auto *imageObj = dynamic_cast<Image2DObj *>(canvas()->getObjectByName(value));
        if (! imageObj)
          return app->errorMsg(QString("Failed to find object '%1'").arg(value));

        image_ = imageObj->image();
      }
    }
    else
      image_ = QImage();
  }
  else if (name == "flip_x") {
    image_ = image_.mirrored(true, false);
  }
  else if (name == "flip_y") {
    image_ = image_.mirrored(false, true);
  }
  else if (name == "scale") {
    Point2D size;
    if (! Util::stringToPoint2D(tcl, value, size))
      return false;

    image_ = image_.scaled(image_.width()*size.x.value, image_.height()*size.y.value);
  }
  else if (name == "size") {
    CPoint2D p;
    if (! Util::stringToPoint2D(tcl, value, p))
      return false;

    image_ = QImage(p.x, p.y, QImage::Format_ARGB32);
  }
  else if (name == "sub_image") {
    // draw specified image from at point in this image
    if (args.size() < 1)
      return false;

    CPoint2D p;
    if (! Util::stringToPoint2D(tcl, value, p))
      return false;

    auto *imageObj = dynamic_cast<Image2DObj *>(canvas()->getObjectByName(args[0]));
    if (! imageObj)
      return app->errorMsg(QString("Failed to find object '%1'").arg(args[0]));

    QPainter painter(&image_);

    painter.drawImage(p.x, p.y, imageObj->image_);
  }
  else if (name == "image_mask") {
    auto *imageObj = dynamic_cast<Image2DObj *>(canvas()->getObjectByName(value));

    if (! imageObj)
      return app->errorMsg(QString("Failed to find image '%1'").arg(value));

    imageMask_ = imageObj->image_;
  }
  else if (name == "stroke.width") {
    double w;
    if (! Util::stringToReal(value, w))
      return false;

    pen_.setWidthF(w);
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

bool
Image2DObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  auto *tcl = canvas()->tcl();

  if      (op == "resize") {
    if (args.size() != 1)
      return false;

    CPoint2D p;
    if (! Util::stringToPoint2D(tcl, args[0], p))
      return false;

    // TODO: keep original image ?
    image_ = image_.scaled(p.x, p.y);
  }
  else if (op == "stroke.rect") {
    QRect  rect;
    QColor c;

    if      (args.size() == 1) {
      if (! Util::stringToColor(tcl, args[0], c))
        return false;

      rect = QRect(0, 0, image_.width() - 1, image_.height() - 1);
    }
    else if (args.size() == 2) {
      if (! Util::stringToQRect(tcl, args[0], rect))
        return false;

      if (! Util::stringToColor(tcl, args[1], c))
        return false;
    }
    else
      return false;

    QPainter painter(&image_);

    pen_.setColor(c);

    painter.setPen(pen_);

    painter.drawRect(rect);
  }
  else if (op == "stroke.line") {
    QPoint p1, p2;
    QColor c;

    if (args.size() != 3)
      return false;

    if (! Util::stringToQPoint(tcl, args[0], p1))
      return false;

    if (! Util::stringToQPoint(tcl, args[1], p2))
      return false;

    if (! Util::stringToColor(tcl, args[2], c))
      return false;

    QPainter painter(&image_);

    pen_.setColor(c);

    painter.setPen(pen_);

    painter.drawLine(p1, p2);
  }
  else if (op == "fill.rect") {
    QRect  rect;
    QColor c;

    if      (args.size() == 1) {
      if (! Util::stringToColor(tcl, args[0], c))
        return false;

      rect = QRect(0, 0, image_.width() - 1, image_.height() - 1);
    }
    else if (args.size() == 2) {
      if (! Util::stringToQRect(tcl, args[0], rect))
        return false;

      if (! Util::stringToColor(tcl, args[1], c))
        return false;

      QPainter painter(&image_);

    }
    else
      return false;

    QPainter painter(&image_);

    brush_ = QBrush(c);

    painter.fillRect(rect, brush_);
  }
  else
    return Object2D::exec(op, args, res);

  return true;
}

Rect2D
Image2DObj::
calcRect() const
{
  if (posType_ == Position::RECT)
    return rect_;

  int w = image_.width ();
  int h = image_.height();

  auto s = canvas()->pixelSizeToWindow(QSizeF(w, h));

  auto pos = pointToPixel(Point2D::makeWindow(position_));

  if (posType_ == Position::CENTER) {
    pos.x.value -= w/2;
    pos.y.value -= h/2;
  }

  auto p = pointToWindow(pos);

  auto ll = Point2D(p.x.value            , p.y.value             );
  auto ur = Point2D(p.x.value + s.width(), p.y.value + s.height());

  return Rect2D(ll, ur);
}

void
Image2DObj::
draw(QPainter *painter)
{
  if (image_.isNull())
    return;

  QRectF prect;

  if (posType_ == Position::RECT) {
    prect = canvas()->rectToPixel(rect_).qrect();
  }
  else {
    int w = image_.width ();
    int h = image_.height();

    auto pos = pointToPixel(Point2D::makeWindow(position_)).qpoint();

    if (posType_ == Position::CENTER) {
      pos.setX(pos.x() - w/2);
      pos.setY(pos.y() - h/2);
    }

    prect = QRectF(pos.x(), pos.y(), w, h);
  }

  if (! imageMask_.isNull()) {
    painter->save();

    auto bitmapMask = QBitmap::fromImage(imageMask_.createAlphaMask());

    QRegion clipRegion(bitmapMask);

    clipRegion.translate(prect.left(), prect.top());

    painter->setClipRegion(clipRegion);

    painter->drawImage(prect, image_);

    painter->restore();
  }

  if (isSelected()) {
    painter->setBrush(canvas()->selectedColor());

    painter->drawRect(prect);
  }
}

}
