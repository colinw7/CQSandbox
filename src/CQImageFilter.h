#ifndef CQImageFilter_H
#define CQImageFilter_H

#include <CRGBA.h>

#include <QImage>

namespace CQImageFilter {
  QImage unsharpMask(const QImage &src, double strength=2.0);

  QImage sobel(const QImage &src, bool feldman=false);

  QImage convolve(const QImage &src, const int *kernel, int size, int divisor=1);

  QImage gaussianBlur(const QImage &src, double bx=1, double by=1, int nx=0, int ny=0);

  QImage turbulence(const QImage &src, bool fractal, double baseFreq, int numOctaves, int seed);

  QImage erode(const QImage &src);
  QImage dilate(const QImage &src);

  QImage mask(const QImage &src, const QImage &imask);

  QImage tint(const QImage &src, const CRGBA &rgba);

  void getPixel(const QImage &image, int x, int y, CRGBA &rgba);

  void setPixel(QImage &image, int x, int y, const CRGBA &rgba);
};

#endif
