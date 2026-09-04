#ifndef CQSandboxObject3D_H
#define CQSandboxObject3D_H

#include <CQSandboxGeom.h>
#include <CQSandboxFaceData.h>

#include <CVector3D.h>
#include <CMatrix3DH.h>
#include <CBBox3D.h>
#include <CPoint3D.h>

#include <QObject>
#include <QVariant>
#include <QStringList>

#include <optional>
#include <set>

class CQGLBuffer;

namespace CQSandbox {

class Object3D;
class Canvas3D;
class Group3DObj;
class BBox3DObj;

//---

class ObjectMgr3D {
 public:
  ObjectMgr3D() { }

  virtual ~ObjectMgr3D() { }

  virtual const char *typeName() const = 0;

  virtual void initRender(Canvas3D *) { }
  virtual void termRender(Canvas3D *) { }

  void addObject(Object3D *object) {
    objects_.push_back(object);
  }

 private:
  std::vector<Object3D *> objects_;
};

//---

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
  Q_PROPERTY(double  xpos     READ xPos       WRITE setXPos)
  Q_PROPERTY(double  ypos     READ yPos       WRITE setYPos)
  Q_PROPERTY(double  zpos     READ zPos       WRITE setZPos)
  Q_PROPERTY(double  xscale   READ xScale     WRITE setXScale)
  Q_PROPERTY(double  yscale   READ yScale     WRITE setYScale)
  Q_PROPERTY(double  zscale   READ zScale     WRITE setZScale)

 public:
  enum class Type {
    NONE,
    ARRAY,
    AXIS,
    BBOX,
    CSV,
    DUNGEON,
    FIELD_RUNNERS,
    GRAPH,
    GROUP,
    MODEL,
    OTHELLO,
    PARTICLE_LIST,
    PATH,
    PLANE,
    QUAD_TREE,
    SHADER,
    SHADER_SHAPE,
    SHAPE,
    SKYBOX,
    SPRITE,
    SURFACE,
    TEXT,
    VECTOR
  };

  enum ModelMatrixFlags : unsigned int {
    NONE      = 0,
    TRANSLATE = (1<<0),
    SCALE     = (1<<1),
    ROTATE    = (1<<2),
    ALL       = (TRANSLATE | SCALE | ROTATE)
  };

  using SelectedPoints = std::set<int>;
  using SelectedFaces  = std::set<int>;

  using FaceDatas = std::vector<FaceData>;

 public:
  Object3D(Canvas3D *canvas, Type type);

  Object3D(const Object3D &) = delete;
  Object3D &operator=(const Object3D &) = delete;

  Canvas3D *canvas() const { return canvas_; }

  //---

  virtual const char *typeName() const = 0;

  virtual ObjectMgr3D *mgr() { return nullptr; }

  //---

  Type type() const { return type_; }

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

  double xPos() const { return position_.x; }
  void setXPos(double a);

  double yPos() const { return position_.y; }
  void setYPos(double a);

  double zPos() const { return position_.z; }
  void setZPos(double a);

  const CPoint3D &position() const { return position_; }
  virtual void setPosition(const CPoint3D &p);

  virtual CPoint3D origin() const;
  virtual void setOrigin(const CPoint3D &p);

  //---

  double xScale() const { return xscale_; }
  void setXScale(double s) { setScales(s, yscale_, zscale_); }

  double yScale() const { return yscale_; }
  void setYScale(double s) { setScales(xscale_, s, zscale_); }

  double zScale() const { return zscale_; }
  void setZScale(double s) { setScales(xscale_, yscale_, s); }

  void setScale(double s) { setScales(s, s, s); }

  virtual void setScales(double xs, double ys, double zs);

  //---

  const CMatrix3DH &modelMatrix() const { return modelMatrix_; }

  const CMatrix3DH &meshMatrix() const { return meshMatrix_; }

  //---

  Group3DObj *group() const { return group_; }
  void setGroup(Group3DObj *group) { group_ = group; }

  //---

  bool isNeedsUpdate() const { return needsUpdate_; }

  void setNeedsUpdate();

  //---

  virtual void init();

  virtual void setModelMatrix(uint flags=ModelMatrixFlags::ALL);

  //---

  virtual bool getValue(const QString &name, const QStringList &args, QVariant &value);
  virtual bool setValue(const QString &name, const QString &value, const QStringList &args);

  virtual bool exec(const QString &name, const QStringList &args, QVariant &res);

  //---

  virtual void updateModelMatrix();

  const CBBox3D &bbox() {
    if (! bboxValid_) {
      auto *th = const_cast<Object3D *>(this);

      th->bbox_ = th->calcBBox();

      th->bboxValid_ = true;
    }

    return bbox_;
  }

  virtual CBBox3D calcBBox() = 0;

  virtual Rect getBBox() const { return Rect(); } // TODO: for Quad Tree

  virtual CQGLBuffer *getBuffer() const { return buffer_; }

  virtual const FaceDatas &getFaceDatas() const { return faceDatas_; }

  //---

  virtual void tick();

  virtual void preRender() { }

  virtual void render();

  virtual bool intersect(const CVector3D &, const CVector3D &, CPoint3D &, CPoint3D &) const {
    return false;
  }

  QString getCommandName() const;

  //---

  void createBBoxObj();

  BBox3DObj *bboxObj() const { return bboxObj_; }

  //---

  void clearSelection();

  void selectPoint(int i);
  void selectFace (int i);

  const SelectedPoints &selectedPoints() const { return selectedPoints_; }
  const SelectedFaces  &selectedFaces () const { return selectedFaces_; }

  //---

  CPoint3D  getFaceCenter(int i) const;
  CVector3D getFaceNormal(int i) const;

 protected:
  using OptPoint = std::optional<CPoint3D>;

  Canvas3D* canvas_ { nullptr };
  Type      type_   { Type::NONE };
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

  CMatrix3DH modelMatrix_ { CMatrix3DH::identity() };
  CMatrix3DH meshMatrix_  { CMatrix3DH::identity() };

  int    ticks_   { 0 };
  int    dt_      { 1 };
  double elapsed_ { 0.0 };

  Group3DObj *group_ { nullptr };

  CBBox3D    bbox_;
  BBox3DObj *bboxObj_   { nullptr };
  bool       bboxValid_ { false };

  bool needsUpdate_ { true };

  CQGLBuffer* buffer_ { nullptr };

  FaceDatas faceDatas_;

  SelectedPoints selectedPoints_;
  SelectedFaces  selectedFaces_;
};

}

#endif
