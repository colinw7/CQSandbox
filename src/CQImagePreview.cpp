#include <CQImagePreview.h>
#include <CQScrollArea.h>
#include <CQImageUtil.h>
#include <CQUtil.h>
#include <CQUtilRGBA.h>
#include <CQUtilEvent.h>

#include <control.xpm>

#include <QPainter>
#include <QHBoxLayout>
#include <QToolButton>

CQImagePreview::
CQImagePreview(QWidget *parent) :
 QWidget(parent)
{
  setObjectName("preview");

  auto *layout = new QHBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  canvas_ = new CQImagePreviewCanvas(this);

  sarea_ = new CQScrollArea(canvas_);

  connect(sarea_, SIGNAL(updateArea()), this, SLOT(canvasUpdateSlot()));

  auto *button = new QToolButton;

  button->setAutoRaise(true);

  button->setIcon(QIcon(QPixmap(control_data)));

  sarea_->setCornerWidget(button);

  layout->addWidget(sarea_);
}

CQImagePreview::
~CQImagePreview()
{
}

void
CQImagePreview::
canvasUpdateSlot()
{
  canvas_->update();
}

void
CQImagePreview::
setFit(bool b)
{
  fit_ = b;

  setImage();
}

void
CQImagePreview::
sendCancelSignal()
{
  Q_EMIT cancelSignal();
}

CImagePtr
CQImagePreview::
getCImage() const
{
  return canvas_->getImage().cimage;
}

QImage
CQImagePreview::
getQImage() const
{
  return canvas_->getImage().qimage;
}

void
CQImagePreview::
clear()
{
  datas_.clear();

  imageNum_ = 0;
}

void
CQImagePreview::
addImage(CImagePtr cimage)
{
  CQImagePreviewData data;

  data.cimage = cimage;

  datas_.push_back(data);

  if (datas_.size() == 1) {
    imageNum_ = 0;

    setImage();
  }
}

void
CQImagePreview::
addImage(QImage qimage)
{
  CQImagePreviewData data;

  data.qimage = qimage;

  datas_.push_back(data);

  if (datas_.size() == 1) {
    imageNum_ = 0;

    setImage();
  }
}

void
CQImagePreview::
setBackground(const CRGBA &bg)
{
  canvas_->setBackground(bg);
}

void
CQImagePreview::
scrollUp(bool page)
{
  sarea_->scrollUp(page);
}

void
CQImagePreview::
scrollDown(bool page)
{
  sarea_->scrollDown(page);
}

void
CQImagePreview::
scrollLeft(bool page)
{
  sarea_->scrollLeft(page);
}

void
CQImagePreview::
scrollRight(bool page)
{
  sarea_->scrollRight(page);
}

void
CQImagePreview::
nextImage()
{
  if (imageNum_ < int(datas_.size()) - 1) {
    ++imageNum_;

    setImage();
  }
}

void
CQImagePreview::
prevImage()
{
  if (! datas_.empty() && imageNum_ > 0) {
    --imageNum_;

    setImage();
  }
}

void
CQImagePreview::
updateScrollbars()
{
  getScrollArea()->updateScrollbars();
}

void
CQImagePreview::
setImage()
{
  if (imageNum_ >= 0 && imageNum_ < int(datas_.size()))
    canvas_->setImage(datas_[imageNum_]);

  canvas_->update();
}

//----------

CQImagePreviewCanvas::
CQImagePreviewCanvas(CQImagePreview *view) :
 QWidget(nullptr), view_(view)
{
  setObjectName("canvas");

  setFocusPolicy(Qt::StrongFocus);
}

CQImagePreviewCanvas::
~CQImagePreviewCanvas()
{
}

void
CQImagePreviewCanvas::
setImage(const CQImagePreviewData &data)
{
  data_ = data;

  resizeEvent(nullptr);

  update();
}

void
CQImagePreviewCanvas::
setBackground(const CRGBA &bg)
{
  bg_ = bg;

  update();
}

void
CQImagePreviewCanvas::
setZoomFactor(double factor)
{
  zoomFactor_ = factor;

  update();
}

void
CQImagePreviewCanvas::
zoomIncrease()
{
  setZoomFactor(2.0*getZoomFactor());
}

void
CQImagePreviewCanvas::
zoomDecrease()
{
  setZoomFactor(0.5*getZoomFactor());
}

void
CQImagePreviewCanvas::
resizeEvent(QResizeEvent *)
{
  if (data_.cimage)
    qimage_ = CQImageUtil::toQImage(data_.cimage);
  else
    qimage_ = data_.qimage;

  if (view_->isFit())
    qimage_ = qimage_.scaled(width(), height(), Qt::KeepAspectRatio);

  view_->updateScrollbars();
}

void
CQImagePreviewCanvas::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.fillRect(rect(), CQUtil::rgbaToColor(bg_));

  //---

  QImage qimage;

  if (data_.cimage)
    qimage = CQImageUtil::toQImage(data_.cimage);
  else
    qimage = data_.qimage;

  if (qimage_.isNull())
    return;

  //---

  int dx = 0, dy = 0;

  auto iw = qimage_.width ();
  auto ih = qimage_.height();

  if (int(iw) > width())
    dx = view_->getScrollArea()->getXOffset();
  else
    dx = (width() - int(iw))/2;

  if (int(ih) > height())
    dy = view_->getScrollArea()->getYOffset();
  else
    dy = (height() - int(ih))/2;

  painter.drawImage(QPoint(dx, dy), qimage_);

  view_->getScrollArea()->setXSize(int(iw));
  view_->getScrollArea()->setYSize(int(ih));
}

void
CQImagePreviewCanvas::
keyPressEvent(QKeyEvent *e)
{
  auto *ke = CQUtil::convertEvent(e);

  if      (ke->getType() == CKEY_TYPE_Escape)
    view_->sendCancelSignal();
  else if (ke->getType() == CKEY_TYPE_Up)
    view_->scrollUp(ke->isShiftKey());
  else if (ke->getType() == CKEY_TYPE_Down)
    view_->scrollDown(ke->isShiftKey());
  else if (ke->getType() == CKEY_TYPE_Left)
    view_->scrollLeft(ke->isShiftKey());
  else if (ke->getType() == CKEY_TYPE_Right)
    view_->scrollRight(ke->isShiftKey());
  else if (ke->getType() == CKEY_TYPE_n)
    view_->nextImage();
  else if (ke->getType() == CKEY_TYPE_p)
    view_->prevImage();
  else if (ke->getType() == CKEY_TYPE_f)
    view_->setFit(! view_->isFit());
}
