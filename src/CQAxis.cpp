#include <CQAxis.h>
#include <CMathGen.h>
#include <CMathRound.h>
#include <CStrUtil.h>
#include <QPainter>

#include <cstring>
#include <algorithm>

#define MIN_GOOD_TICKS 4
#define MAX_GOOD_TICKS 12
#define OPTIMUM_TICKS  10

#define MAX_GAP_TESTS 14

struct AxisIncrementTest {
  double factor;
  uint   numTicks;
  double incFactor;
};

static AxisIncrementTest
axesIncrementTests[MAX_GAP_TESTS] = {
  {  1.0, 5, 0 },
  {  1.2, 3, 0 },
  {  2.0, 4, 0 },
  {  2.5, 5, 0 },
  {  4.0, 4, 0 },
  {  5.0, 5, 0 },
  {  6.0, 3, 0 },
  {  8.0, 4, 0 },
  { 10.0, 5, 0 },
  { 12.0, 3, 0 },
  { 20.0, 4, 0 },
  { 25.0, 5, 0 },
  { 40.0, 4, 0 },
  { 50.0, 5, 0 }
};

CQAxis::
CQAxis(Direction direction, double start, double end) :
 direction_(direction),
 start_    (start),
 end_      (end),
 start1_   (start),
 end1_     (end)
{
  calc();
}

void
CQAxis::
setRange(double start, double end)
{
  start_ = start;
  end_   = end;

  calc();
}

void
CQAxis::
setTickIncrement(uint tickIncrement)
{
  tickIncrement_ = tickIncrement;

  calc();
}

void
CQAxis::
setTickSpaces(double *tickSpaces, uint numTickSpaces)
{
  tickSpaces_.resize(numTickSpaces);

  memcpy(&tickSpaces_[0], tickSpaces, numTickSpaces*sizeof(double));
}

void
CQAxis::
setLabel(const std::string &str)
{
  label_ = str;
}

bool
CQAxis::
calc()
{
  numTicks1_ = 1;
  numTicks2_ = 0;

  //------

  /* Ensure Axis Start and End are in the Correct Order */

  double minAxis = std::min(start_, end_);
  double maxAxis = std::max(start_, end_);

  //------

  /* Calculate Length */

  double length = fabs(maxAxis - minAxis);

  if (length == 0.0)
    return false;

  //------

  /* Calculate nearest Power of Ten to Length */

  int power = CMathRound::RoundDown(log10(length));

  //------

  /* Set Default Increment to 0.1 * Power of Ten */

  double increment = 0.1;

  if      (power < 0) {
    for (int i = 0; i < -power; i++)
      increment /= 10.0;
  }
  else if (power > 0) {
    for (int i = 0; i <  power; i++)
      increment *= 10.0;
  }

  //------

  /* Calculate other test Increments */

  for (int i = 0; i < MAX_GAP_TESTS; i++)
    axesIncrementTests[i].incFactor = increment*axesIncrementTests[i].factor;

  //------

  /* Set Default Start/End to Force Update */

  start1_ = 0.0;
  end1_   = 0.0;

  //------

  /* Test each Increment in turn */

  uint numGaps, numGapTicks;

  for (int i = 0; i < MAX_GAP_TESTS; i++) {
    if (tickIncrement_ > 0) {
      int incFactor1 = int(axesIncrementTests[i].incFactor);

      if (double(incFactor1) != axesIncrementTests[i].incFactor)
        continue;

      if ((incFactor1 % int(tickIncrement_)) != 0)
        continue;
    }

    testAxisGaps(minAxis, maxAxis,
                 axesIncrementTests[i].incFactor,
                 axesIncrementTests[i].numTicks,
                 &start1_, &end1_, &increment,
                 &numGaps, &numGapTicks);
  }

  //------

  /* Set the Gap Positions */

  numTicks1_ = uint(CMathRound::RoundDown((end1_ - start1_)/increment + 0.5));
  numTicks2_ = numGapTicks;

  return true;
}

bool
CQAxis::
testAxisGaps(double start, double end, double testIncrement, uint testNumGapTicks,
             double *start1, double *end1, double *increment, uint *numGaps, uint *numGapTicks)
{
  /* Calculate New Start and End implied by the Test Increment */

  double newStart = CMathRound::RoundDown(start/testIncrement)*testIncrement;
  double newEnd   = CMathRound::RoundUp  (end  /testIncrement)*testIncrement;

  while (newStart > start)
    newStart -= testIncrement;

  while (newEnd < end)
    newEnd += testIncrement;

  uint testNumGaps = uint(CMathRound::RoundUp((newEnd - newStart)/testIncrement));

  //------

  /* If nothing set yet just update values and return */

  if (*start1 == 0.0 && *end1 == 0.0) {
    *start1 = newStart;
    *end1   = newEnd;

    *increment   = testIncrement;
    *numGaps     = testNumGaps;
    *numGapTicks = testNumGapTicks;

    return true;
  }

  //------

  /* If the current number of gaps is not within the acceptable range
     and the new number of gaps is within the acceptable range then
     update current */

  if ((   *numGaps <  MIN_GOOD_TICKS ||    *numGaps >  MAX_GOOD_TICKS) &&
      (testNumGaps >= MIN_GOOD_TICKS && testNumGaps <= MAX_GOOD_TICKS)) {
    *start1 = newStart;
    *end1   = newEnd;

    *increment   = testIncrement;
    *numGaps     = testNumGaps;
    *numGapTicks = testNumGapTicks;

    return true;
  }

  //------

  /* If the current number of gaps is not within the acceptable range
     and the new number of gaps is not within the acceptable range then
     consider it for update of current if better fit */

  if ((   *numGaps < MIN_GOOD_TICKS ||    *numGaps > MAX_GOOD_TICKS) &&
      (testNumGaps < MIN_GOOD_TICKS || testNumGaps > MAX_GOOD_TICKS)) {
    /* Calculate how close fit is to required range */

    double delta1 = fabs(newStart - start) + fabs(newEnd - end);

    //------

    /* If better fit than current fit or equally good fit and
       number of gaps is nearer to optimum (OPTIMUM_TICKS) then
       update current */

    double delta2 = fabs(*start1 - start) + fabs(*end1 - end);

    if (((fabs(delta1 - delta2) < 1E-6) &&
         (abs(int(testNumGaps - OPTIMUM_TICKS)) < abs(int(*numGaps - OPTIMUM_TICKS)))) ||
        delta1 < delta2) {
      *start1 = newStart;
      *end1   = newEnd;

      *increment   = testIncrement;
      *numGaps     = testNumGaps;
      *numGapTicks = testNumGapTicks;

      return true;
    }
  }

  //------

  /* If the current number of gaps is within the acceptable range
     and the new number of gaps is within the acceptable range then
     consider it for update of current if better fit */

  if ((   *numGaps >= MIN_GOOD_TICKS &&    *numGaps <= MAX_GOOD_TICKS) &&
      (testNumGaps >= MIN_GOOD_TICKS && testNumGaps <= MAX_GOOD_TICKS)) {
    /* Calculate how close fit is to required range */

    double delta1 = fabs(newStart - start) + fabs(newEnd - end);

    //------

    /* If better fit than current fit or equally good fit and
       number of gaps is nearer to optimum (OPTIMUM_TICKS) then
       update current */

    double delta2 = fabs(*start1 - start) + fabs(*end1 - end);

    if (((fabs(delta1 - delta2) < 1E-6) &&
         (abs(int(testNumGaps - OPTIMUM_TICKS)) < abs(int(*numGaps - OPTIMUM_TICKS)))) ||
        delta1 < delta2) {
      *start1 = newStart;
      *end1   = newEnd;

      *increment   = testIncrement;
      *numGaps     = testNumGaps;
      *numGapTicks = testNumGapTicks;

      return true;
    }
  }

  return false;
}

double
CQAxis::
getMajorIncrement() const
{
  if (numTicks1_ > 0)
    return (end1_ - start1_)/numTicks1_;
  else
    return 0.0;
}

double
CQAxis::
getMinorIncrement() const
{
  if (numTicks1_ > 0 && numTicks2_ > 0)
    return (end1_ - start1_)/(numTicks1_*numTicks2_);
  else
    return 0.0;
}

std::string
CQAxis::
getValueStr(double pos) const
{
  return CStrUtil::toString(pos);
}

void
CQAxis::
draw(QPainter *p, double px, double py, double len)
{
  QFontMetrics fm(p->font());

  //int w = p->device()->width ();
  //int h = p->device()->height();

  double p1 = (direction_ == DIR_HORIZONTAL ? px : py);
  double p2 = p1 + len;

  if (getLineDisplayed()) {
    p->setPen(lineColor_);

    if (direction_ == DIR_HORIZONTAL)
      p->drawLine(int(p1), int(py), int(p2), int(py));
    else
      p->drawLine(int(px), int(p1), int(px), int(p2));
  }

  double inc  = getMajorIncrement();
  double inc1 = inc/getNumMinorTicks();

  double pos1 = getStart();

  for (uint i = 0; i < getNumMajorTicks() + 1; i++) {
    double pp = valueToPos(pos1, p1, p2);

    int dt1 = (getSide() == SIDE_BOTTOM_LEFT ? 8 : -8);

    if (direction_ == DIR_HORIZONTAL)
      p->drawLine(int(pp), int(py), int(pp), int(py + dt1));
    else
      p->drawLine(int(px), int(pp), int(px - dt1), int(pp));

    if (i < getNumMajorTicks()) {
      for (uint j = 1; j < getNumMinorTicks(); j++) {
        double pos2 = pos1 + j*inc1;

        double pp = valueToPos(pos2, p1, p2);

        int dt2 = (getSide() == SIDE_BOTTOM_LEFT ? 4 : -4);

        if (direction_ == DIR_HORIZONTAL)
          p->drawLine(int(pp), int(py), int(pp), int(py + dt2));
        else
          p->drawLine(int(px), int(pp), int(px - dt2), int(pp));
      }
    }

    if (getLabelDisplayed()) {
      QString text = getValueStr(pos1).c_str();

      int tw = fm.horizontalAdvance(text);
      int ta = fm.ascent();
      int td = fm.descent();

      if (direction_ == DIR_HORIZONTAL) {
        if (getSide() == SIDE_BOTTOM_LEFT)
          p->drawText(int(pp - tw/2), int(py + 10 + ta), text);
        else
          p->drawText(int(pp - tw/2), int(py - 10 - td), text);
      }
      else {
        if (getSide() == SIDE_BOTTOM_LEFT)
          p->drawText(int(px - tw - 10), int(pp + ta/2), text);
        else
          p->drawText(int(px + tw + 10), int(pp + ta/2), text);
      }
    }

    pos1 += inc;
  }
}

double
CQAxis::
valueToPos(double v, double p1, double p2)
{
  return (p2 - p1)*(v - getStart())/(getEnd() - getStart()) + p1;
}
