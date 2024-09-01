#ifndef CQAxis_H
#define CQAxis_H

#include <sys/types.h>

#include <string>
#include <vector>

#include <QColor>

class QPainter;

class CQAxis {
 public:
  enum Direction {
    DIR_HORIZONTAL,
    DIR_VERTICAL
  };

  enum Side {
    SIDE_BOTTOM_LEFT,
    SIDE_TOP_RIGHT
  };

 public:
  CQAxis(Direction direction=DIR_HORIZONTAL, double start=0.0, double end=1.0);

 ~CQAxis() { }

  const Direction &getDirection() const { return direction_; }
  void setDirection(const Direction &v) { direction_ = v; }

  const Side &getSide() const { return side_; }
  void setSide(Side side) { side_ = side; }

  double getStart() const { return start1_; }
  void setStart(double start) { start1_ = start; }

  double getEnd() const { return end1_ ; }
  void setEnd(double end) { end1_ = end; }

  uint getNumMajorTicks() const { return numTicks1_; }
  void setNumMajorTicks(uint n) { numTicks1_ = n; }

  uint getNumMinorTicks() const { return numTicks2_; }
  void setNumMinorTicks(uint n) { numTicks2_ = n; }

  uint getTickIncrement() const { return tickIncrement_; }

  const double *getTickSpaces   () const { return &tickSpaces_[0]; }
  uint          getNumTickSpaces() const { return uint(tickSpaces_.size()); }

  double getTickSpace(int i) const { return tickSpaces_[uint(i)]; }

  const std::string &getLabel() const { return label_; }

  void setRange(double start, double end);

  void setTickIncrement(uint tickIncrement);

  void setTickSpaces(double *tickSpaces, uint numTickSpaces);

  void setLabel(const std::string &str);

  double getMajorIncrement() const;
  double getMinorIncrement() const;

  std::string getValueStr(double pos) const;

  bool getLineDisplayed() const { return lineDisplayed_; }
  void setLineDisplayed(bool b) { lineDisplayed_ = b; }

  bool getLabelDisplayed() const { return labelDisplayed_; }
  void setLabelDisplayed(bool b) { labelDisplayed_ = b; }

  const QColor &getLineColor() const { return lineColor_; }
  void setLineColor(const QString &c) { lineColor_ = c; }

  void draw(QPainter *p, double px, double py, double len);

  double valueToPos(double v, double p1, double p2);

 private:
  bool calc();

  bool testAxisGaps(double start, double end, double testIncrement,
                    uint testNumGapTicks, double *start1, double *end1,
                    double *increment, uint *numGaps, uint *numGapTicks);

 private:
  Direction           direction_      { DIR_HORIZONTAL };
  Side                side_           { SIDE_BOTTOM_LEFT };
  double              start_          { 0.0 };
  double              end_            { 1.0 };
  double              start1_         { 0.0 };
  double              end1_           { 1.0 };
  uint                numTicks1_      { 1 };
  uint                numTicks2_      { 0 };
  uint                tickIncrement_  { 0 };
  std::vector<double> tickSpaces_;
  std::string         label_;
  bool                lineDisplayed_  { true };
  QColor              lineColor_      { 0, 0, 0 };
  bool                labelDisplayed_ { true };
};

#endif
