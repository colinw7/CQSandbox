#ifndef CQSandboxPalette2DObj_H
#define CQSandboxPalette2DObj_H

#include <CQSandboxObject2D.h>

#include <CColorRange.h>

namespace CQSandbox {

class Palette2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  Palette2DObj(Canvas2D *canvas);

  const char *typeName() const override { return "palette"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  CColorRange colorRange_;
};

}

#endif
