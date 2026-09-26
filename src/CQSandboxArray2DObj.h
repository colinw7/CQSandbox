#ifndef CQSandboxArray2DObj_H
#define CQSandboxArray2DObj_H

#include <CQSandboxObject2D.h>

namespace CQSandbox {

class IntArray2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  IntArray2DObj(Canvas2D *canvas, uint dim);

  const char *typeName() const override { return "int_array"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  using Values = std::vector<int>;

  Values values_;
};

class RealArray2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  RealArray2DObj(Canvas2D *canvas, uint dim);

  const char *typeName() const override { return "real_array"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  using Values = std::vector<double>;

  Values values_;
};

class IntMatrix2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  IntMatrix2DObj(Canvas2D *canvas, uint dim0, uint dim1);

  const char *typeName() const override { return "int_matrix"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  using Values      = std::vector<int>;
  using ValuesArray = std::vector<Values>;

  ValuesArray values_;
};

class RealMatrix2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  RealMatrix2DObj(Canvas2D *canvas, uint dim0, uint dim1);

  const char *typeName() const override { return "real_matrix"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  using Values      = std::vector<double>;
  using ValuesArray = std::vector<Values>;

  ValuesArray values_;
};

}

#endif
