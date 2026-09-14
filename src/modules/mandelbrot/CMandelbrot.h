#ifndef CMANDELBROT_H
#define CMANDELBROT_H

#include <CPointFractal.h>
#include <vector>

class CMandelbrot : public CPointFractalCalc {
 public:
  CMandelbrot();
  CMandelbrot(const CMandelbrot &m);

  CMandelbrot *dup() const override;

  bool isAutoColor() const { return autoColor_; }
  void setAutoColor(bool b) { autoColor_ = b; }

  double getXMin() const override { return -2.0; }
  double getYMin() const override { return -1.2; }
  double getXMax() const override { return  1.2; }
  double getYMax() const override { return  1.2; }

  void setDistance(bool distance) { distance_ = distance; }

  void initCalc(int pixel_xmin, int pixel_ymin, int pixel_xmax, int pixel_ymax,
                double xmin=-2.0, double ymin=-1.2, double xmax=1.2, double ymax=1.2,
                int max_iterations=1000) override;

  int calc(double x, double y, int max_iterations=1000) const override;

  int calc_iterations(double x, double y, int max_iterations) const;
  int calc_distance(double x, double y, int max_iterations) const;

  int iterate(double x, double y, int max_iterations) const;

  double distance(double x, double y, int iterations) const;

  double pixelXToUser(int x) const;
  double pixelYToUser(int y) const;

 private:
  bool initialized_    { false };
  bool autoColor_      { false };
  int  max_iterations_ { 0 };

  int pixel_xmin_ { 0 };
  int pixel_ymin_ { 0 };
  int pixel_xmax_ { 100 };
  int pixel_ymax_ { 100 };;

  double xmin_ { -2.0 };
  double ymin_ { -1.2 };
  double xmax_ {  1.2 };
  double ymax_ {  1.2 };

  bool   distance_ { false };
  double d_        { 0.0 };

  mutable double zr_ { 0.0 }, zi_ { 0.0 };

  mutable std::vector<double> save_x_;
  mutable std::vector<double> save_y_;
};

#endif
