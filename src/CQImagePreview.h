#ifndef CQIMAGE_PREVIEW_H
#define CQIMAGE_PREVIEW_H

#include <CImageLib.h>
#include <CRGBA.h>

#include <QWidget>

class QResizeEvent;
class QPaintEvent;

class CQImagePreviewCanvas;
class CQScrollArea;

//---

struct CQImagePreviewData {
  CImagePtr cimage;
  QImage    qimage;
};

//---

class CQImagePreview : public QWidget {
  Q_OBJECT

 public:
  CQImagePreview(QWidget *parent=nullptr);

 ~CQImagePreview();

  CImagePtr getCImage() const;
  QImage getQImage() const;

  void clear();

  void addImage(CImagePtr image);
  void addImage(QImage image);

  void setBackground(const CRGBA &rgba);

  CQScrollArea *getScrollArea() const { return sarea_; }

  bool isFit() const { return fit_; }
  void setFit(bool b);

  void sendCancelSignal();

  void scrollUp   (bool page=false);
  void scrollDown (bool page=false);
  void scrollLeft (bool page=false);
  void scrollRight(bool page=false);

  void nextImage();
  void prevImage();

  void updateScrollbars();
  void setImage();

 Q_SIGNALS:
  void cancelSignal();

 private Q_SLOTS:
  void canvasUpdateSlot();

 private:
  using Datas = std::vector<CQImagePreviewData>;

  CQImagePreviewCanvas* canvas_   { nullptr };
  CQScrollArea*         sarea_    { nullptr };
  Datas                 datas_;
  int                   imageNum_ { -1 };
  bool                  fit_      { false };
};

//---

class CQImagePreviewCanvas : public QWidget {
  Q_OBJECT

 public:
  CQImagePreviewCanvas(CQImagePreview *view);

  virtual ~CQImagePreviewCanvas();

  const CQImagePreviewData &getImage() const { return data_; }
  void setImage(const CQImagePreviewData &data);

  double getZoomFactor() const { return zoomFactor_; }

  void setZoomFactor(double factor);

  void setFillScreen(bool fillScreen) { fillScreen_ = fillScreen; }
  void setKeepAspect(bool keepAspect) { keepAspect_ = keepAspect; }

  bool getFillScreen() const { return fillScreen_; }
  bool getKeepAspect() const { return keepAspect_; }

  void setBackground(const CRGBA &rgba);

  void zoomIncrease();
  void zoomDecrease();

  void resizeEvent(QResizeEvent *e) override;
  void paintEvent(QPaintEvent *e) override;

  void keyPressEvent(QKeyEvent *e) override;

 private:
  CQImagePreview*    view_       { nullptr };
  CQImagePreviewData data_;
  double             zoomFactor_ { 1.0 };
  bool               fillScreen_ { false };
  bool               keepAspect_ { true };
  CRGBA              bg_;
  QImage             qimage_;
};

#endif
