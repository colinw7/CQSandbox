#ifndef CCOLOR_RANGE_H
#define CCOLOR_RANGE_H

#include <CRGBA.h>

#include <map>
#include <vector>

class CColorRange {
 public:
  enum class Type {
    NONE,
    MAGMA,
    MORELAND,
    PLASMA,
    RGB_RANGE,
    VIRIDIS
  };

  enum class DataType {
    ARRAY,
    MAP
  };

 public:
  CColorRange();

 ~CColorRange() { }

  const Type &type() const { return type_; }
  void setType(const Type &t) { type_ = t; init(); }

  const CRGBA &getColor(uint i) const {
    return colors_[i];
  }

  CRGBA interpColor(double r) const;

  std::vector<std::string> typeNames() const;

  Type nameToType(const std::string &name) const;

 private:
  void init();

  void initRGBRange();
  void initMagma();
  void initMoreland();
  void initPlasma();
  void initViridis();

 private:
  using Colors   = std::vector<CRGBA>;
  using ColorMap = std::map<double, CRGBA>;

  Type     type_     { Type::RGB_RANGE };
  DataType dataType_ { DataType::ARRAY };
  Colors   colors_;
  ColorMap colorMap_;
};

#endif
