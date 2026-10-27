#include <CQSandboxObject3D.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxGroup3DObj.h>
#include <CQSandboxBBox3DObj.h>
#include <CQSandboxCamera.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLBuffer.h>

namespace CQSandbox {

Object3D::
Object3D(Canvas3D *canvas, Type type) :
 canvas_(canvas), type_(type)
{
}

//---

QString
Object3D::
getCommandName() const
{
  return QString("sb3d::%1.%2").arg(typeName()).arg(ind_);
}

QString
Object3D::
calcId() const
{
  auto id = this->id();

  if (id == "")
    id = getCommandName();

  return id;
}

//----

void
Object3D::
setSelected(bool b)
{
  selected_ = b;

  canvas_->update();
}

//---

const CPoint3D &
Object3D::
angles() const
{
  return angles_;
}

void
Object3D::
setAngles(const CPoint3D &p)
{
  angles_ = p;

  updateModelMatrix();

  setNeedsUpdate();

  Q_EMIT transformChanged();
}

const CPoint3D &
Object3D::
position() const
{
  return position_.value();
}

void
Object3D::
setPosition(const CPoint3D &p)
{
  position_.setValue(p);

  updateModelMatrix();

  setNeedsUpdate();

  Q_EMIT transformChanged();
}

const CPoint3D &
Object3D::
scales() const
{
  return scales_;
}

void
Object3D::
setScales(const CPoint3D &p)
{
  scales_ = p;

  updateModelMatrix();

  setNeedsUpdate();

  Q_EMIT transformChanged();
}

CPoint3D
Object3D::
origin() const
{
  return origin_.value_or(position());
}

void
Object3D::
setOrigin(const CPoint3D &p)
{
  origin_ = p;

  updateModelMatrix();

  setNeedsUpdate();

  Q_EMIT transformChanged();
}

void
Object3D::
applyTransform()
{
  updateModelMatrix();

  applyMatrix(modelMatrix());

  angles_ = CPoint3D::makeZero();

  position_.setValue(CPoint3D::makeZero());

  scales_ = CPoint3D(1.0, 1.0, 1.0);

  updateModelMatrix();

  setNeedsUpdate();

  Q_EMIT transformChanged();
}

//---

void
Object3D::
setNeedsUpdate()
{
  needsUpdate_ = true;
  bboxValid_   = false;

  canvas_->update();
}

void
Object3D::
init()
{
  modelMatrix_ = CMatrix3DH::identity();
}

void
Object3D::
updateModelMatrix()
{
  setModelMatrix();
}

void
Object3D::
setModelMatrix()
{
  // object centered at (0, 0). Moved to specified position

  auto pos = this->position();
  auto o   = this->origin(); // rotation center

  auto mt1 = CMatrix3DH::translation( o.getX(),  o.getY(),  o.getZ());
  auto mt2 = CMatrix3DH::translation(-o.getX(), -o.getY(), -o.getZ());
  auto mt3 = CMatrix3DH::translation(pos.getX(), pos.getY(), pos.getZ());

  auto ms = CMatrix3DH::scale(xScale(), yScale(), zScale());

  auto mr = anglesMatrix();

  modelMatrix_ = mt1*mr*mt2*mt3*ms;
}

CMatrix3DH
Object3D::
anglesMatrix() const
{
  if (xAngle() != 0 || yAngle() != 0 || zAngle() != 0) {
    auto mrx = CMatrix3DH::rotation(xAngle(), CVector3D(1.0, 0.0, 0.0));
    auto mry = CMatrix3DH::rotation(yAngle(), CVector3D(0.0, 1.0, 0.0));
    auto mrz = CMatrix3DH::rotation(zAngle(), CVector3D(0.0, 0.0, 1.0));

    return mrx*mry*mrz;
  }
  else
    return CMatrix3DH::identity();
}

//---

bool
Object3D::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *app = canvas()->app();

  if      (name == "id")
    value = id();
  else if (name == "type_name")
    value = typeName();

  // state
  else if (name == "visible") {
    value = isVisible();
  }
  else if (name == "selected") {
    value = isSelected();
  }
  else if (name == "pseudo") {
    value = isPseudo();
  }
  else if (name == "layer") {
    value = layer();
  }

  else if (name == "position") {
    value = Util::point3DToString(position());
  }
  else if (name == "x_angle") {
    value = Util::realToString(Util::radToDeg(xAngle()));
  }
  else if (name == "y_angle") {
    value = Util::realToString(Util::radToDeg(yAngle()));
  }
  else if (name == "z_angle") {
    value = Util::realToString(Util::radToDeg(zAngle()));
  }
  else if (name == "angles") {
    auto a  = angles();
    auto a1 = CPoint3D(Util::radToDeg(a.x), Util::radToDeg(a.y), Util::radToDeg(a.z));

    value = Util::point3DToString(a1);
  }
  else if (name == "group")
    value = (group() ? group()->calcId() : "");
  else if (name == "bbox") {
    QStringList strs;

    strs << QString::number(bbox_.getXMin());
    strs << QString::number(bbox_.getYMin());
    strs << QString::number(bbox_.getZMin());
    strs << QString::number(bbox_.getXMax());
    strs << QString::number(bbox_.getYMax());
    strs << QString::number(bbox_.getZMax());

    value = strs.join(" ");
  }
  else if (name == "bbox.center")
    value = Util::point3DToString(bbox_.getCenter());
  else if (name == "faces") {
    const auto &faces = getFaceDatas();

    QStringList faceIds;

    for (uint i = 0; i < faces.size(); ++i)
      faceIds.push_back(QString::number(i));

    value = faceIds;
  }
  else if (name == "face.center") {
    if (args.size() < 1)
      return false;

    int ind;
    if (! Util::stringToInt(args[0], ind))
      return false;

    auto center = getFaceCenter(ind);

    value = Util::point3DToString(center);
  }
  else if (name == "face.normal") {
    if (args.size() < 1)
      return false;

    int ind;
    if (! Util::stringToInt(args[0], ind))
      return false;

    auto normal = getFaceNormal(ind);

    value = Util::vector3DToString(normal);
  }
  else if (name == "face.orient") {
    if (args.size() < 1)
      return false;

    int ind;
    if (! Util::stringToInt(args[0], ind))
      return false;

    auto orient = getFaceOrient(ind);

    value = (orient == CPolygonOrientation::CLOCKWISE ? "clockwise" : "anticlockwise");
  }
  else if (name == "face.points") {
    if (args.size() < 1)
      return false;

    int ind;
    if (! Util::stringToInt(args[0], ind))
      return false;

    const auto &faceDatas = this->getFaceDatas();

    if (ind < 0 || ind >= int(faceDatas.size()))
      return false;

    const auto &faceData = faceDatas[ind];

    loadBuffer();

    QStringList facePoints;

    for (int i = 0; i < faceData.len; ++i) {
      CQGLBuffer::PointData pointData;
      faceData.buffer->getPointData(i, pointData);

      auto str = Util::point3DToString(pointData.point->point());

      facePoints.push_back(str);
    }

    value = facePoints;
  }
  else if (name == "lines") {
    const auto &lines = getLineDatas();

    QStringList lineIds;

    for (uint i = 0; i < lines.size(); ++i)
      lineIds.push_back(QString::number(i));

    value = lineIds;
  }
  else if (name == "line.start" || name == "line.end") {
    if (args.size() < 1)
      return false;

    int ind;
    if (! Util::stringToInt(args[0], ind))
      return false;

    const auto &lines = getLineDatas();

    if (ind < 0 || ind >= int(lines.size()))
      return false;

    const auto &line = lines[ind];

    if (! line.buffer)
      return false;

    auto lind = (name == "line.start" ? line.ind1 : line.ind2);

    loadBuffer();

    CQGLBuffer::PointData pointData;
    line.buffer->getPointData(lind, pointData);

    value = Util::point3DToString(pointData.point->point());
  }
  else if (name == "points") {
    const auto &points = getPointDatas();

    QStringList pointIds;

    for (uint i = 0; i < points.size(); ++i)
      pointIds.push_back(QString::number(i));

    value = pointIds;
  }
  else if (name == "point.position") {
    if (args.size() < 1)
      return false;

    int ind;
    if (! Util::stringToInt(args[0], ind))
      return false;

    const auto &points = getPointDatas();

    if (ind < 0 || ind >= int(points.size()))
      return false;

    const auto &point = points[ind];

    if (! point.buffer)
      return false;

    loadBuffer();

    CQGLBuffer::PointData pointData;
    point.buffer->getPointData(point.ind, pointData);

    value = Util::point3DToString(pointData.point->point());
  }
  else
    return app->errorMsg(QString("Invalid get name '%1'").arg(name));

  return true;
}

bool
Object3D::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (name == "id")
    setId(value);

  // state
  else if (name == "visible") {
    bool b;
    if (! Util::stringToBool(value, b))
      return false;

    setVisible(b);

    setNeedsUpdate();
  }
  else if (name == "selected") {
    bool b;
    if (! Util::stringToBool(value, b))
      return false;

    setSelected(b);
  }
  else if (name == "pseudo") {
    bool b;
    if (! Util::stringToBool(value, b))
      return false;

    setPseudo(b);

    setNeedsUpdate();
  }
  else if (name == "layer") {
    int i;
    if (! Util::stringToInt(value, i))
      return false;

    setLayer(i);
  }

  else if (name == "position") {
    CPoint3D p;
    if (! Util::stringToPoint3D(tcl, value, p))
      return false;

    setPosition(p);
  }
  else if (name == "position.target") {
    CPoint3D p;
    if (! Util::stringToPoint3D(tcl, value, p))
      return false;

    position_.setTarget(p);
  }
  else if (name == "position.steps") {
    int i;
    if (! Util::stringToInt(value, i))
      return false;

    position_.setSteps(i);
  }
  else if (name == "angles") {
    CPoint3D p;
    if (! Util::stringToPoint3D(tcl, value, p))
      return false;

    setAngles(CPoint3D(Util::degToRad(p.x), Util::degToRad(p.y), Util::degToRad(p.z)));
  }
  else if (name == "x_angle") {
    double a;
    if (! Util::stringToReal(value, a))
      return false;

    setXAngle(Util::degToRad(a));
  }
  else if (name == "y_angle") {
    double a;
    if (! Util::stringToReal(value, a))
      return false;

    setYAngle(Util::degToRad(a));
  }
  else if (name == "z_angle") {
    double a;
    if (! Util::stringToReal(value, a))
      return false;

    setZAngle(Util::degToRad(a));
  }
  else if (name == "scales") {
    CPoint3D p;
    if (! Util::stringToPoint3D(tcl, value, p))
      return false;

    setScales(p);
  }
  else if (name == "scale") {
    double s;
    if (! Util::stringToReal(value, s))
      return false;

    setScale(s);
  }
  else if (name == "group") {
    auto *group = dynamic_cast<Group3DObj *>(canvas()->getObjectByName(value));

    if (group != group_) {
      if (group_)
        group_->removeObject(this);
      else
        canvas()->removeObject(this);

      if (group)
        group->addObject(this);
      else
        canvas()->addObject(this);
    }
  }
  else if (name == "cull_face") {
    bool b;
    if (! Util::stringToBool(value, b))
      return false;

    setCullFace(b);
  }
  else if (name == "ignore_camera") {
    bool b;
    if (! Util::stringToBool(value, b))
      return false;

    setIgnoreCamera(b);
  }
  else if (name == "ignore_world") {
    bool b;
    if (! Util::stringToBool(value, b))
      return false;

    setIgnoreWorld(b);
  }
  else if (name == "point.position") {
    if (args.size() < 1)
      return false;

    int ind;
    if (! Util::stringToInt(value, ind))
      return false;

    CPoint3D p;
    if (! Util::stringToPoint3D(tcl, args[0], p))
      return false;

    if (! setPointPosition(ind, p))
      return false;
  }
  else if (name == "face.points") {
    if (args.size() < 1)
      return false;

    int ind;
    if (! Util::stringToInt(value, ind))
      return false;

    std::vector<CPoint3D> points;
    if (! Util::stringToPoints3D(tcl, args[0], points))
      return false;

    if (! setFacePoints(ind, points))
      return false;

    return true;
  }
  else if (name == "line.start" || name == "line.end") {
    if (args.size() < 1)
      return false;

    int ind;
    if (! Util::stringToInt(value, ind))
      return false;

    CPoint3D p;
    if (! Util::stringToPoint3D(tcl, args[0], p))
      return false;

    const auto &lines = getLineDatas();

    if (ind < 0 || ind >= int(lines.size()))
      return false;

    const auto &line = lines[ind];

    if (! line.buffer)
      return false;

    auto lind = (name == "line.start" ? line.ind1 : line.ind2);

    CQGLBuffer::PointData pointData;
    line.buffer->getPointData(lind, pointData);

    auto p1 = p;
    auto p2 = pointData.point->point();

    if (name == "line.end")
      std::swap(p1, p2);

    if (! setLinePoints(ind, p1, p2))
      return false;
  }
  else
    return app->errorMsg(QString("Invalid set name '%1'").arg(name));

  return true;
}

bool
Object3D::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  auto *app = canvas()->app();

  if (op == "intersect") {
    if (args.size() < 1)
      return false;

    auto *obj = canvas()->getObjectByName(args[0]);
    if (! obj)
      return app->errorMsg(QString("Failed to find object '%1'").arg(args[0]));

    auto bbox1 = this->bbox();
    auto bbox2 = obj ->bbox();

    auto overlap = bbox1.overlaps(bbox2);

    res = overlap;
  }
  else
    return false;

  return true;
}

bool
Object3D::
getTclValue(const QString &name, const TclObjs &args, Tcl_Obj* &res)
{
  auto *tcl = canvas()->tcl();

  QStringList args1;
  for (auto *arg : args)
    args1.push_back(tcl->variantFromObj(arg).toString());

  QVariant res1;
  if (! getValue(name, args1, res1))
    return false;

  res = tcl->variantToObj(res1);

  return true;
}

bool
Object3D::
setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &args)
{
  auto *tcl = canvas()->tcl();

  QStringList args1;
  for (auto *arg : args)
    args1.push_back(tcl->variantFromObj(arg).toString());

  if (! setValue(name, tcl->qstringFromObj(value), args1))
    return false;

  return true;
}

bool
Object3D::
execTcl(const QString &op, const TclObjs &args, Tcl_Obj* &res)
{
  auto *tcl = canvas()->tcl();

  QStringList args1;
  for (auto *arg : args)
    args1.push_back(tcl->variantFromObj(arg).toString());

  QVariant res1;
  if (! exec(op, args1, res1))
    return false;

  res = tcl->variantToObj(res1);

  return true;
}

//----

bool
Object3D::
setPointPosition(int ind, const CPoint3D &p)
{
  const auto &points = getPointDatas();

  if (ind < 0 || ind >= int(points.size()))
    return false;

  const auto &point = points[ind];

  if (! point.buffer)
    return false;

  loadBuffer();

  point.buffer->setPoint(point.ind, p);

  point.buffer->load();

  return true;
}

bool
Object3D::
setLinePoints(int ind, const CPoint3D &p1, const CPoint3D &p2)
{
  const auto &lines = getLineDatas();

  if (ind < 0 || ind >= int(lines.size()))
    return false;

  const auto &line = lines[ind];

  if (! line.buffer)
    return false;

  loadBuffer();

  line.buffer->setPoint(line.ind1, p1, /*invalidate*/false);
  line.buffer->setPoint(line.ind2, p2);

  line.buffer->load();

  return true;
}

bool
Object3D::
setFacePoints(int ind, const std::vector<CPoint3D> &points)
{
  const auto &faces = getFaceDatas();

  if (ind < 0 || ind >= int(faces.size()))
    return false;

  const auto &face = faces[ind];

  if (! face.buffer)
    return false;

  loadBuffer();

  for (int i = 0; i < face.len; ++i) {
    face.buffer->setPoint(face.pos + i, points[i], /*invalidate*/ i == face.len - 1);
  }

  face.buffer->load();

  return true;
}

//---

CQGLBuffer *
Object3D::
getBufferByInd(uint ind) const
{
  assert(ind == buffer_->ind());

  return buffer_;
}

//---

void
Object3D::
tick()
{
  ticks_ += dt_;

  elapsed_ += canvas_->redrawTimeOut()/1000.0;

  position_.step();

  //canvas()->runTclCmd("tick");
}

void
Object3D::
render()
{
}

CPoint3D
Object3D::
getFaceCenter(int i) const
{
  CPoint3D c;

  std::vector<CPoint3D> points;
  if (! getFacePoints(i, points))
    return c;

  c = Util::pointsCenter(points);

  return  c;
}

CVector3D
Object3D::
getFaceNormal(int i) const
{
  CVector3D n;

  std::vector<CPoint3D> points;
  if (! getFacePoints(i, points))
    return n;

  n = Util::pointsNormal(points);

  return n;
}

CPolygonOrientation
Object3D::
getFaceOrient(int i) const
{
  std::vector<CPoint3D> points;
  if (! getFacePoints(i, points))
    return CPolygonOrientation::UNKNOWN;

  auto *camera = canvas_->currentCamera();

  const auto &projectionMatrix = camera->perspectiveMatrix();
  const auto &viewMatrix       = camera->viewMatrix();

  auto pvMatrix = projectionMatrix*viewMatrix;

  std::vector<CPoint3D> ppoints;
  for (const auto &p : points)
    ppoints.push_back(pvMatrix*p);

  return Util::pointsOrientation(ppoints);
}

bool
Object3D::
getFacePoints(int i, std::vector<CPoint3D> &points) const
{
  auto *buffer = this->getBuffer();
  if (! buffer) return false;

  const auto &faceDatas = this->getFaceDatas();

  if (i < 0 || i >= int(faceDatas.size()))
    return false;

  const_cast<Object3D *>(this)->loadBuffer();

  const auto &modelMatrix = this->modelMatrix();
  const auto &meshMatrix  = this->meshMatrix();

  auto matrix = modelMatrix*meshMatrix;

  const auto &faceData = faceDatas[i];

  for (int i = 0; i < faceData.len; ++i) {
    CQGLBuffer::PointData pointData;
    buffer->getPointData(faceData.pos + i, pointData);

    auto pp = matrix*pointData.point->point();

    points.push_back(pp);
  }

  return true;
}

//---

bool
Object3D::
intersectRay(const CVector3D &, const CVector3D &, CPoint3D &, CPoint3D &) const
{
  return false;
}

//---

BBox3DObj *
Object3D::
createBBoxObj()
{
  if (! bboxObj_) {
    bboxObj_ = new BBox3DObj(canvas_);

    bboxObj_->init();

    bboxObj_->setParentObj(this);
  }

  if (bbox_ != bboxObj_->parentBBox()) {
    bboxObj_->setParentBBox(bbox_);

    bboxObj_->setPosition(bbox_.getCenter());

    auto s = 1.01;

    bboxObj_->setScales(CPoint3D(s*bbox_.getXSize(), s*bbox_.getYSize(), s*bbox_.getZSize()));

    bboxObj_->setNeedsUpdate();
  }

  return bboxObj_;
}

//---

void
Object3D::
addModifier(Modifier3DObj *obj)
{
  modifiers_.push_back(obj);
}

void
Object3D::
removeModifier(Modifier3DObj *obj)
{
  Modifiers modifiers;

  for (auto *obj1 : modifiers_)
    if (obj1 != obj)
      modifiers.push_back(obj1);

  std::swap(modifiers_, modifiers);
}

//---

void
Object3D::
clearSelection()
{
  selectedPoints_.clear();
  selectedLines_ .clear();
  selectedFaces_ .clear();
}

void
Object3D::
selectPoint(uint bufferInd, uint pointInd)
{
  selectedPoints_[bufferInd].insert(pointInd);
}

void
Object3D::
selectLine(uint bufferInd, uint lineInd)
{
  selectedLines_[bufferInd].insert(lineInd);
}

void
Object3D::
selectFace(uint bufferInd, uint faceInd)
{
  selectedFaces_[bufferInd].insert(faceInd);
}

bool
Object3D::
isPointSelected(uint bufferInd, uint faceInd) const
{
  auto pf = selectedPoints_.find(bufferInd);
  if (pf == selectedPoints_.end()) return false;

  auto pi = (*pf).second.find(faceInd);
  if (pi == (*pf).second.end()) return false;

  return true;
}

bool
Object3D::
isFaceSelected(uint bufferInd, uint faceInd) const
{
  auto pf = selectedFaces_.find(bufferInd);
  if (pf == selectedFaces_.end()) return false;

  auto pi = (*pf).second.find(faceInd);
  if (pi == (*pf).second.end()) return false;

  return true;
}

}
