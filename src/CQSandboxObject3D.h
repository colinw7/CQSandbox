#ifndef CQSandboxObject3D_H
#define CQSandboxObject3D_H

#include <CQSandboxGeom.h>
#include <CQSandboxFaceData.h>
#include <CQSandboxUtil.h>
#include <CQSandboxAnim.h>

#include <CVector3D.h>
#include <CMatrix3DH.h>
#include <CBBox3D.h>
#include <CPoint3D.h>
#include <CPolygonOrientation.h>

#include <QObject>
#include <QVariant>
#include <QStringList>

#include <optional>
#include <set>

class CQGLBuffer;

namespace CQSandbox {

class Canvas3D;

class BBox3DObj;
class Group3DObj;
class Modifier3DObj;
class Object3D;

//---

struct ProgramMatrixData {
  ProgramMatrixData() { }

  bool ignoreCamera { false };
  bool ignoreWorld  { false };
};

//---

class ObjectMgr3D {
 public:
  ObjectMgr3D() { }

  virtual ~ObjectMgr3D() { }

  virtual const char *typeName() const = 0;

  virtual void initRender(Canvas3D *) { }
  virtual void termRender(Canvas3D *) { }

  const std::vector<Object3D *> &objects() const { return objects_; }

  void addObject(Object3D *object) {
    objects_.push_back(object);
  }

  virtual void tick() { }

 private:
  std::vector<Object3D *> objects_;
};

//---

class Object3D : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString id          READ id)
  Q_PROPERTY(int     ind         READ indI)
  Q_PROPERTY(QString typeName    READ typeNameStr)
  Q_PROPERTY(QString commandName READ getCommandName)

  Q_PROPERTY(bool visible  READ isVisible  WRITE setVisible)
  Q_PROPERTY(bool selected READ isSelected WRITE setSelected)
  Q_PROPERTY(bool inside   READ isInside   WRITE setInside)
  Q_PROPERTY(bool pseudo   READ isPseudo   WRITE setPseudo)
  Q_PROPERTY(int  layer    READ layer      WRITE setLayer)

  Q_PROPERTY(double xangle  READ xAngleDeg WRITE setXAngleDeg)
  Q_PROPERTY(double yangle  READ yAngleDeg WRITE setYAngleDeg)
  Q_PROPERTY(double zangle  READ zAngleDeg WRITE setZAngleDeg)
  Q_PROPERTY(double xpos    READ xPos      WRITE setXPos)
  Q_PROPERTY(double ypos    READ yPos      WRITE setYPos)
  Q_PROPERTY(double zpos    READ zPos      WRITE setZPos)
  Q_PROPERTY(double xscale  READ xScale    WRITE setXScale)
  Q_PROPERTY(double yscale  READ yScale    WRITE setYScale)
  Q_PROPERTY(double zscale  READ zScale    WRITE setZScale)

 public:
  enum class Type {
    NONE,
    ANIM_REAL,
    ARRAY,
    ASTAR,
    AXIS,
    BASIS,
    BBOX,
    CSV,
    DUNGEON,
    FIELD_RUNNERS,
    GIZMO,
    GRAPH,
    GRID,
    GROUP,
    JSON,
    LINE_LIST,
    MODEL,
    MODIFIER,
    OTHELLO,
    PARTICLE_LIST,
    PATH,
    PLANE,
    POINT,
    POINT_LIST,
    QUAD_TREE,
    SHADER,
    SHADER_SHAPE,
    SHAPE,
    SHLIB,
    SKYBOX,
    SPRITE,
    SURFACE,
    TEXT,
    VECTOR
  };

  using LineDatas  = std::vector<LineData>;
  using PointDatas = std::vector<PointData>;

  struct PointInd {
    uint bufferInd { 0 };
    uint pointInd  { 0 };

    friend bool operator<(const PointInd &lhs, const PointInd &rhs) {
      if (lhs.bufferInd != rhs.bufferInd)
        return (lhs.bufferInd < rhs.bufferInd);

      return (lhs.pointInd < rhs.pointInd);
    }
  };

  struct FaceInd {
    uint bufferInd { 0 };
    uint faceInd  { 0 };

    friend bool operator<(const FaceInd &lhs, const FaceInd &rhs) {
      if (lhs.bufferInd != rhs.bufferInd)
        return (lhs.bufferInd < rhs.bufferInd);

      return (lhs.faceInd < rhs.faceInd);
    }
  };

  using PointSet       = std::set<uint>;
  using SelectedPoints = std::map<uint, PointSet>;

  using LineSet       = std::set<uint>;
  using SelectedLines = std::map<uint, LineSet>;

  using FaceSet        = std::set<uint>;
  using SelectedFaces  = std::map<uint, FaceSet>;

  using TclObjs = std::vector<Tcl_Obj *>;

  using Modifiers = std::vector<Modifier3DObj *>;

 public:
  Object3D(Canvas3D *canvas, Type type);

  Object3D(const Object3D &) = delete;
  Object3D &operator=(const Object3D &) = delete;

  Canvas3D *canvas() const { return canvas_; }

  //---

  virtual ObjectMgr3D *mgr() { return nullptr; }

  //---

  virtual const char *typeName() const = 0;

  QString typeNameStr() const { return QString(typeName()); }

  Type type() const { return type_; }

  //---

  size_t ind() const { return ind_; }
  void setInd(size_t ind) { ind_ = ind; }

  int indI() const { return int(ind()); }

  const QString &id() const { return id_; }
  void setId(const QString &s) { id_ = s; }

  QString getCommandName() const;

  QString calcId() const;

  //---

  bool isVisible() const { return visible_; }
  void setVisible(bool b) { visible_ = b; }

  bool isSelected() const { return selected_; }
  virtual void setSelected(bool b);

  bool isInside() const { return inside_; }
  void setInside(bool b) { inside_ = b; }

  bool isPseudo() const { return pseudo_; }
  void setPseudo(bool b) { pseudo_ = b; }

  bool isLighted() const { return lighted_; }
  void setLighted(bool b) { lighted_ = b; }

  int layer() const { return layer_; }
  void setLayer(int i) { layer_ = i; }

  //---

  bool isCullFace() const { return cullFace_; }
  void setCullFace(bool b) { cullFace_ = b; }

  bool isFrontFace() { return frontFace_; }
  void setFrontFace(bool b) { frontFace_ = b; }

  //---

  bool isIgnoreCamera() const { return programMatrixData_.ignoreCamera; }
  void setIgnoreCamera(bool b) { programMatrixData_.ignoreCamera = b; }

  bool isIgnoreWorld() const { return programMatrixData_.ignoreWorld; }
  void setIgnoreWorld(bool b) { programMatrixData_.ignoreWorld = b; }

  const ProgramMatrixData &programMatrixData() const { return programMatrixData_; }

  //---

  double xAngle() const { return angles().x; }
  void setXAngle(double a) { setAngles(CPoint3D(a, yAngle(), zAngle())); }

  double yAngle() const { return angles().y; }
  void setYAngle(double a) { setAngles(CPoint3D(xAngle(), a, zAngle())); }

  double zAngle() const { return angles().z; }
  void setZAngle(double a) { setAngles(CPoint3D(xAngle(), yAngle(), a)); }

  double xAngleDeg() const { return Util::radToDeg(xAngle()); }
  void setXAngleDeg(double a) { setXAngle(Util::degToRad(a)); }

  double yAngleDeg() const { return Util::radToDeg(yAngle()); }
  void setYAngleDeg(double a) { setYAngle(Util::degToRad(a)); }

  double zAngleDeg() const { return Util::radToDeg(zAngle()); }
  void setZAngleDeg(double a) { setZAngle(Util::degToRad(a)); }

  virtual const CPoint3D &angles() const;
  virtual void setAngles(const CPoint3D &p);

  CPoint3D anglesDeg() const {
    const auto &a = angles();
    return CPoint3D(Util::radToDeg(a.x), Util::radToDeg(a.y), Util::radToDeg(a.z));
  }

  void setAnglesDeg(const CPoint3D &p) {
    setAngles(CPoint3D(Util::degToRad(p.x), Util::degToRad(p.y), Util::degToRad(p.z)));
  }

  //---

  double xPos() const { return position().x; }
  void setXPos(double x) { setPosition(CPoint3D(x, yPos(), zPos())); }

  double yPos() const { return position().y; }
  void setYPos(double y) { setPosition(CPoint3D(xPos(), y, zPos())); }

  double zPos() const { return position().z; }
  void setZPos(double z) { setPosition(CPoint3D(xPos(), yPos(), z)); }

  virtual const CPoint3D &position() const;
  virtual void setPosition(const CPoint3D &p);

  //---

  double xScale() const { return scales().x; }
  void setXScale(double s) { setScales(CPoint3D(s, yScale(), zScale())); }

  double yScale() const { return scales().y; }
  void setYScale(double s) { setScales(CPoint3D(xScale(), s, zScale())); }

  double zScale() const { return scales().z; }
  void setZScale(double s) { setScales(CPoint3D(xScale(), yScale(), s)); }

  void setScale(double s) { setScales(CPoint3D(s, s, s)); }

  virtual const CPoint3D &scales() const;
  virtual void setScales(const CPoint3D &p);

  //---

  virtual CPoint3D origin() const;
  virtual void setOrigin(const CPoint3D &p);

  //---

  virtual void applyTransform();

  virtual void applyMatrix(const CMatrix3DH &) { }

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

  virtual void setModelMatrix();

  CMatrix3DH anglesMatrix() const;

  //---

  virtual bool isTclCmd() const { return false; }

  virtual void init();

  virtual bool getValue(const QString &name, const QStringList &args, QVariant &res);
  virtual bool setValue(const QString &name, const QString &value, const QStringList &args);

  virtual bool exec(const QString &op, const QStringList &args, QVariant &res);

  virtual bool getTclValue(const QString &name, const TclObjs &args, Tcl_Obj* &res);
  virtual bool setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &args);

  virtual bool execTcl(const QString &op, const TclObjs &args, Tcl_Obj* &res);

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

  virtual CBBox3D calcBBox() { return bbox_; }

  virtual Rect2D getBBox() const { return Rect2D(); } // TODO: for Quad Tree

  virtual CBBox3D getRawBBox() { return rawBBox_; }

  //---

  virtual bool setPointPosition(int ind, const CPoint3D &p);

  virtual bool setLinePoints(int ind, const CPoint3D &p1, const CPoint3D &p2);

  virtual bool setFacePoints(int ind, const std::vector<CPoint3D> &points);

  //---

  virtual CQGLBuffer *getBuffer() const { return buffer_; }

  virtual CQGLBuffer *getBufferByInd(uint ind) const;

  virtual const FaceDatas  &getFaceDatas () const { return faceDataList_.faceDatas; }
  virtual const LineDatas  &getLineDatas () const { return lineDatas_; }
  virtual const PointDatas &getPointDatas() const { return pointDatas_; }


  //---

  virtual void tick();

  virtual void preRender() { }

  virtual void render() = 0;

  virtual void loadBuffer() = 0;

  virtual bool intersectRay(const CVector3D &p1, const CVector3D &p2,
                            CPoint3D &pi1, CPoint3D &pi2) const;

  //---

  BBox3DObj *createBBoxObj();

  BBox3DObj *bboxObj() const { return bboxObj_; }

  //---

  virtual void clearSelection();

  virtual void selectPoint(uint bufferInd, uint pointInd);
  virtual void selectLine (uint bufferInd, uint lineInd);
  virtual void selectFace (uint bufferInd, uint faceInd);

  const SelectedPoints &selectedPoints() const { return selectedPoints_; }
  const SelectedLines  &selectedLines () const { return selectedLines_; }
  const SelectedFaces  &selectedFaces () const { return selectedFaces_; }

  bool isPointSelected(uint bufferInd, uint pointInd) const;
  bool isFaceSelected (uint bufferInd, uint faceInd) const;

  //---

  CPoint3D            getFaceCenter(int i) const;
  CVector3D           getFaceNormal(int i) const;
  CPolygonOrientation getFaceOrient(int i) const;

  bool getFacePoints(int i, std::vector<CPoint3D> &points) const;

  //---

  const Modifiers &modifiers() const { return modifiers_; }

  void addModifier(Modifier3DObj *obj);
  void removeModifier(Modifier3DObj *obj);

 Q_SIGNALS:
  void transformChanged();

 protected:
  using OptPoint = std::optional<CPoint3D>;

  Canvas3D* canvas_ { nullptr };
  Type      type_   { Type::NONE };
  size_t    ind_    { 0 };

  QString id_;

  bool visible_  { true };
  bool selected_ { false };
  bool inside_   { false };
  bool pseudo_   { false };
  bool lighted_  { true };

  int layer_ { -1 };

  bool cullFace_  { true };
  bool frontFace_ { true };

  ProgramMatrixData programMatrixData_;

  CPoint3D       angles_   { 0.0, 0.0, 0.0 };
  AnimatePoint3D position_ { CPoint3D::makeZero() };
  CPoint3D       scales_   { 1.0, 1.0, 1.0 };
  OptPoint       origin_;

  CMatrix3DH modelMatrix_ { CMatrix3DH::identity() };
  CMatrix3DH meshMatrix_  { CMatrix3DH::identity() };

  int    ticks_   { 0 };
  double elapsed_ { 0.0 };
  int    dt_      { 1 };

  Group3DObj *group_ { nullptr };

  CBBox3D bbox_;
  bool    bboxValid_ { false };
  CBBox3D rawBBox_;

  BBox3DObj *bboxObj_ { nullptr };

  bool needsUpdate_ { true };

  CQGLBuffer* buffer_ { nullptr };

  FaceDataList faceDataList_;
  LineDatas    lineDatas_;
  PointDatas   pointDatas_;

  SelectedPoints selectedPoints_;
  SelectedLines  selectedLines_;
  SelectedFaces  selectedFaces_;

  Modifiers modifiers_;
};

}

#endif
