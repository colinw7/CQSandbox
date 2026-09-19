#ifndef CJulia_H
#define CJulia_H

#include <vector>

class CJulia {
 public:
  CJulia(double cr = 0.238498, double ci = 0.519198);

  bool isAutoColor() const { return autoColor_; }
  void setAutoColor(bool b) { autoColor_ = b; }

  bool getShowVector() const { return showVector_; }
  void setShowVector(bool showVector) { showVector_ = showVector; }

  bool getDistance() const { return distance_; }
  void setDistance(bool distance) { distance_ = distance; }

  double getXMin() const { return -1.6; }
  double getYMin() const { return -1.2; }
  double getXMax() const { return  1.6; }
  double getYMax() const { return  1.2; }

  void setTime(double t);

  void initCalc(int pixel_xmin, int pixel_ymin, int pixel_xmax, int pixel_ymax,
                double xmin=-2.0, double ymin=-1.2, double xmax=1.2, double ymax=1.2,
                int max_iterations=1000);

  int calc(double x, double y) const;

  int calcIterations(double x, double y) const;
  int calcDistance(double x, double y) const;

  int iterate(double x, double y) const;

  double calcDistance(double x, double y, int iterations) const;

  double pixelXToUser(int x) const;
  double pixelYToUser(int y) const;

 private:
  bool initialized_ { false };
  bool autoColor_   { false };
  bool showVector_  { false };
  bool distance_    { false };

  int max_iterations_ { 0 };

  int pixel_xmin_ { 0 };
  int pixel_ymin_ { 0 };
  int pixel_xmax_ { 100 };
  int pixel_ymax_ { 100 };;

  double xmin_ { -2.0 };
  double ymin_ { -1.2 };
  double xmax_ {  1.2 };
  double ymax_ {  1.2 };

  double xf_ { 1.0 };
  double yf_ { 1.0 };

  double d_ { 0.0 };

  double cr_ { 0.0 };
  double ci_ { 0.0 };

  mutable double zr_ { 0.0 };
  mutable double zi_ { 0.0 };

  mutable std::vector<double> save_x_;
  mutable std::vector<double> save_y_;
};

#endif
