#include <CJulia.h>

#include <cmath>
#include <cassert>

CJulia::
CJulia(double cr, double ci) :
 cr_(cr), ci_(ci)
{
}

void
CJulia::
setTime(double t)
{
  double a = 2.0*t*M_PI;

  cr_ = 0.5*(1.0 + std::cos(a));
  ci_ = 0.5*(1.0 + std::sin(a));
}

void
CJulia::
initCalc(int pixel_xmin, int pixel_ymin, int pixel_xmax, int pixel_ymax,
         double xmin, double ymin, double xmax, double ymax, int max_iterations)
{
  if (! initialized_ || max_iterations_ != max_iterations) {
    save_x_.resize(size_t(max_iterations + 1));
    save_y_.resize(size_t(max_iterations + 1));

    d_ = std::max(xmax - xmin, ymax - ymin)/4.0;

    initialized_    = true;
    max_iterations_ = max_iterations;
  }

  pixel_xmin_ = pixel_xmin;
  pixel_ymin_ = pixel_ymin;
  pixel_xmax_ = pixel_xmax;
  pixel_ymax_ = pixel_ymax;

  xmin_ = xmin;
  ymin_ = ymin;
  xmax_ = xmax;
  ymax_ = ymax;

  xf_ = (xmax_ - xmin_)/double(pixel_xmax_ - pixel_xmin_);
  yf_ = (ymin_ - ymax_)/double(pixel_ymax_ - pixel_ymin_);
}

int
CJulia::
calc(double x, double y) const
{
  if (isAutoColor()) {
    if (! distance_)
      return calcIterations(x, y);
    else
      return calcDistance(x, y);
  }
  else {
    int num_iterations = iterate(x, y);

    if (num_iterations > max_iterations_)
      num_iterations = max_iterations_;

    return num_iterations;
  }
}

int
CJulia::
calcIterations(double x, double y) const
{
  int num_iterations = iterate(x, y);

  if (num_iterations >= max_iterations_)
    num_iterations = max_iterations_;

  // map interations to color (0 - 255)
  int color = 254*num_iterations/(max_iterations_ - 1) + 1;

  if (getShowVector()) {
    auto angle = std::atan2(zr_, zi_);

    if (angle >= M_PI)
      color = 256 - color;
  }

  return color;
}

int
CJulia::
calcDistance(double x, double y) const
{
  int num_iterations = iterate(x, y);

  if (num_iterations >= max_iterations_)
    num_iterations = max_iterations_;

  // map interations to color (0 - 255)
  double dist = calcDistance(x, y, num_iterations);

  int color = std::min(int(254*fabs(dist)/d_), 254) + 1;

  return color;
}

int
CJulia::
iterate(double x, double y) const
{
  int num_iterations = -1;

  zr_ = x;
  zi_ = y;

  double zr2 = zr_*zr_;
  double zi2 = zi_*zi_;

  double zri;

  while (zi2 + zr2 < 4.0 && num_iterations < max_iterations_) {
    zri = zr_*zi_;

    zr_ = zr2 - zi2 + cr_;
    zi_ = zri + zri + ci_;

    zr2 = zr_*zr_;
    zi2 = zi_*zi_;

    ++num_iterations;

    save_x_[size_t(num_iterations)] = zr_;
    save_y_[size_t(num_iterations)] = zi_;
  }

  return num_iterations;
}

double
CJulia::
calcDistance(double, double, int iterations) const
{
  if (iterations == 0)
    return 0.0;

  double x1 = 0;
  double y1 = 0;

  for (int i = 0; i < iterations; ++i) {
    auto xi = save_x_[size_t(i)];
    auto yi = save_y_[size_t(i)];

    auto x2 = 2*(xi*x1 - yi*y1) + 1;
    auto y2 = 2*(yi*x1 + xi*y1);

    x1 = x2;
    y1 = y2;

    if (std::max(::fabs(x1), ::fabs(y1)) > 1e14)
      return 0.0;
  }

  double zr2 = zr_*zr_;
  double zi2 = zi_*zi_;

  auto dist = std::log(zr2 + zi2)*std::sqrt((zr2 + zi2)/(x1*x1 + y1*y1));

  return dist;
}

double
CJulia::
pixelXToUser(int x) const
{
  return double(x - pixel_xmin_)*xf_ + xmin_;
}

double
CJulia::
pixelYToUser(int y) const
{
  return double(y - pixel_ymin_)*yf_ + ymax_;
}
