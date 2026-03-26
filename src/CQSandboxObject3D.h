#ifndef CQSandboxObject3D_H
#define CQSandboxObject3D_H

#include <CGLMatrix3D.h>
#include <CBBox3D.h>
#include <CPoint3D.h>

#include <QObject>
#include <QVariant>
#include <QStringList>

#include <optional>

namespace CQSandbox {

class Canvas3D;
class Group3DObj;
class BBox3DObj;

class Object3D : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString id       READ id)
  Q_PROPERTY(bool    visible  READ isVisible  WRITE setVisible )
  Q_PROPERTY(bool    selected READ isSelected WRITE setSelected)
  Q_PROPERTY(bool    inside   READ isInside   WRITE setInside  )
  Q_PROPERTY(bool    pseudo   READ isPseudo)
  Q_PROPERTY(double  xangle   READ xAngle     WRITE setXAngle)
  Q_PROPERTY(double  yangle   READ yAngle     WRITE setYAngle)
  Q_PROPERTY(double  zangle   READ zAngle     WRITE setZAngle)
  Q_PROPERTY(double  xscale   READ xscale     WRITE setXScale)
  Q_PROPERTY(double  yscale   READ yscale     WRITE setYScale)
  Q_PROPERTY(double  zscale   READ zscale     WRITE setZScale)

 public:
  enum ModelMatrixFlags : unsigned int {
    NONE      = 0,
    TRANSLATE = (1<<0),
    SCALE     = (1<<1),
    ROTATE    = (1<<2),
    ALL       = (TRANSLATE | SCALE | ROTATE)
  };

 public:
  Object3D(Canvas3D *canvas);

  Canvas3D *canvas() const { return canvas_; }

  virtual const char *typeName() const = 0;

  //---

  size_t ind() const { return ind_; }
  void setInd(size_t ind) { ind_ = ind; }

  const QString &id() const { return id_; }
  void setId(const QString &s) { id_ = s; }

  QString calcId() const;

  //---

  bool isVisible() const { return visible_; }
  void setVisible(bool b) { visible_ = b; }

  bool isSelected() const { return selected_; }
  void setSelected(bool b);

  bool isInside() const { return inside_; }
  void setInside(bool b) { inside_ = b; }

  //---

  bool isPseudo() const { return pseudo_; }
  void setPseudo(bool b) { pseudo_ = b; }

  //---

  double xAngle() const { return xAngle_; }
  void setXAngle(double a);

  double yAngle() const { return yAngle_; }
  void setYAngle(double a);

  double zAngle() const { return zAngle_; }
  void setZAngle(double a);

  virtual void setAngles(double xa, double ya, double za);

  //---

  const CPoint3D &position() const { return position_; }
  virtual void setPosition(const CPoint3D &p);

  virtual CPoint3D origin() const;
  virtual void setOrigin(const CPoint3D &p);

  //---

  double xscale() const { return xscale_; }
  void setXScale(double s) { setScales(s, yscale_, zscale_); }

  double yscale() const { return yscale_; }
  void setYScale(double s) { setScales(xscale_, s, zscale_); }

  double zscale() const { return zscale_; }
  void setZScale(double s) { setScales(xscale_, yscale_, s); }

  void setScale(double s) { setScales(s, s, s); }

  virtual void setScales(double xs, double ys, double zs);

  //---

  const CGLMatrix3D &modelMatrix() const { return modelMatrix_; }

  //---

  Group3DObj *group() const { return group_; }
  void setGroup(Group3DObj *group) { group_ = group; }

  //---

  void setNeedsUpdate();

  //---

  virtual void init();

  virtual void setModelMatrix(uint flags=ModelMatrixFlags::ALL);

  virtual QVariant getValue(const QString &name, const QStringList &args);
  virtual bool setValue(const QString &name, const QString &value, const QStringList &args);

  virtual QVariant exec(const QString &, const QStringList &) { return QVariant(); }

  //---

  virtual void updateModelMatrix();

  virtual void tick();

  virtual void preRender() { }

  virtual void render();

  virtual bool intersect(const CGLVector3D &, const CGLVector3D &, CPoint3D &, CPoint3D &) const {
    return false;
  }

  QString getCommandName() const;

  const CBBox3D &bbox() { return bbox_; }

  virtual CBBox3D calcBBox() { return bbox_; }

  void createBBoxObj();

 protected:
  using OptPoint = std::optional<CPoint3D>;

  Canvas3D* canvas_ { nullptr };
  size_t    ind_    { 0 };

  QString id_;
  bool    visible_  { true };
  bool    selected_ { false };
  bool    inside_   { false };
  bool    pseudo_   { false };

  double   xAngle_   { 0.0 };
  double   yAngle_   { 0.0 };
  double   zAngle_   { 0.0 };
  CPoint3D position_ { 0, 0, 0 };
  OptPoint origin_;
  double   xscale_   { 1.0 };
  double   yscale_   { 1.0 };
  double   zscale_   { 1.0 };

  CGLMatrix3D modelMatrix_;

  int    ticks_   { 0 };
  int    dt_      { 1 };
  double elapsed_ { 0.0 };

  Group3DObj *group_ { nullptr };

  CBBox3D    bbox_;
  BBox3DObj *bboxObj_   { nullptr };
  bool       bboxValid_ { false };

  bool needsUpdate_ { true };
};

}

#endif
