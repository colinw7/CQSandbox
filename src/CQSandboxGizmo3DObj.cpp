#include <CQSandboxGizmo3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxShape3DObj.h>
#include <CQSandboxCamera.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CInterval.h>

namespace CQSandbox {

ShaderProgram* Gizmo3DObj::s_program = nullptr;

Object3D *
Gizmo3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Gizmo3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Gizmo3DObj::
Gizmo3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::GIZMO)
{
  auto initShape = [&](Shape3DObj *shape) {
    shape->setPseudo (true);
    shape->setLighted(false);
  };

  auto createCylinder = [&](const QString &name, const CPoint3D &angles, const QColor &c) {
    Shape3DData::ConfigData configData;

    auto *cylinder = new Shape3DObj(canvas);

    cylinder->addCylinder(0.01, 1, configData);

    cylinder->setAngles(angles);

    cylinder->setDiffuseColor(c);

    cylinder->setId(name);

    initShape(cylinder);

    ShapeData shapeData;

    shapeData.shape  = cylinder;
    shapeData.angles = angles;
    shapeData.color  = c;

    shapes_.push_back(shapeData);

    return shapeData;
  };

  xCylinder_ = createCylinder("xCylinder", CPoint3D(0,        0, -M_PI/2.0), Qt::red  );
  yCylinder_ = createCylinder("yCylinder", CPoint3D(0,        0,         0), Qt::green);
  zCylinder_ = createCylinder("zCylinder", CPoint3D(M_PI/2.0, 0,         0), Qt::blue );

  //---

  auto createCone = [&](const QString &name, const CPoint3D &d,
                        const CPoint3D &angles, const QColor &c) {
    Shape3DData::ConfigData configData;

    auto *cone = new Shape3DObj(canvas);

    cone->addCone(0.05, 0.08, configData);

    cone->setPosition(CPoint3D(d.x, d.y, d.z));

    cone->setAngles(angles);

    cone->setDiffuseColor(c);

    cone->setId(name);

    initShape(cone);

    ShapeData shapeData;

    shapeData.shape  = cone;
    shapeData.angles = angles;
    shapeData.color  = c;
    shapeData.offset = d;

    shapes_.push_back(shapeData);

    return shapeData;
  };

  xCone_ = createCone("xCone", CPoint3D(1, 0, 0), CPoint3D(0       , 0, -M_PI/2.0), Qt::red  );
  yCone_ = createCone("yCone", CPoint3D(0, 1, 0), CPoint3D(0       , 0,         0), Qt::green);
  zCone_ = createCone("zCone", CPoint3D(0, 0, 1), CPoint3D(M_PI/2.0, 0,         0), Qt::blue );

  //---

  auto createCube = [&](const QString &name, const CPoint3D &d, const QColor &c) {
    Shape3DData::ConfigData configData;

    auto *cube = new Shape3DObj(canvas);

    auto s = 0.08;

    cube->addCube(s, s, s);

    cube->setPosition(CPoint3D(d.x, d.y, d.z));

    cube->setDiffuseColor(c);

    cube->setId(name);

    initShape(cube);

    ShapeData shapeData;

    shapeData.shape  = cube;
    shapeData.color  = c;
    shapeData.offset = d;

    shapes_.push_back(shapeData);

    return shapeData;
  };

  xCube_ = createCube("xCube", CPoint3D(1, 0, 0), Qt::red  );
  yCube_ = createCube("yCube", CPoint3D(0, 1, 0), Qt::green);
  zCube_ = createCube("zCube", CPoint3D(0, 0, 1), Qt::blue );

  //---

  auto createTorus = [&](const QString &name, const CPoint3D &d,
                         const CPoint3D &angles, const QColor &c) {
    Shape3DData::ConfigData configData;

    configData.num_patches = 10;
    configData.num_xy      = 40;

    auto *torus = new Shape3DObj(canvas);

    torus->addTorus(1, 0.02, 1, 1, configData);

    torus->setPosition(CPoint3D(d.x, d.y, d.z));

    torus->setAngles(angles);

    torus->setDiffuseColor(c);

    torus->setId(name);

    initShape(torus);

    ShapeData shapeData;

    shapeData.shape  = torus;
    shapeData.angles = angles;
    shapeData.color  = c;

    shapes_.push_back(shapeData);

    return shapeData;
  };

  xTorus_ = createTorus("xTorus", CPoint3D(0, 0, 0), CPoint3D(0       , 0,         0), Qt::red  );
  yTorus_ = createTorus("yTorus", CPoint3D(0, 0, 0), CPoint3D(0       , -M_PI/2.0, 0), Qt::green);
  zTorus_ = createTorus("zTorus", CPoint3D(0, 0, 0), CPoint3D(M_PI/2.0, 0,         0), Qt::blue );

  //---

  setEditType(EditType::SELECT);

  connect(canvas_, SIGNAL(editModeChanged()), this, SLOT(updateEditType()));
}

void
Gizmo3DObj::
init()
{
  if (! s_program) {
    auto *app = canvas_->app();

    s_program = new ShaderProgram(canvas_);

    s_program->addVertexFile  (app->buildDir() + "/shaders/gizmo.vs");
    s_program->addFragmentFile(app->buildDir() + "/shaders/gizmo.fs");

    s_program->link();
  }

  for (const auto &shape : shapes_)
    shape.shape->init();

  Object3D::init();
}

void
Gizmo3DObj::
updateEditType()
{
  if (parentObj()) {
    if      (canvas_->editMode() == Canvas3D::EditMode::SELECT)
      setEditType(EditType::SELECT);
    else if (canvas_->editMode() == Canvas3D::EditMode::MOVE)
      setEditType(EditType::MOVE);
    else if (canvas_->editMode() == Canvas3D::EditMode::SCALE)
      setEditType(EditType::SCALE);
    else if (canvas_->editMode() == Canvas3D::EditMode::ROTATE)
      setEditType(EditType::ROTATE);
  }
}

void
Gizmo3DObj::
setEditType(const EditType &t)
{
  editType_ = t;

  xCylinder_.shape->setVisible(editType_ == EditType::MOVE || editType_ == EditType::SCALE);
  yCylinder_.shape->setVisible(editType_ == EditType::MOVE || editType_ == EditType::SCALE);
  zCylinder_.shape->setVisible(editType_ == EditType::MOVE || editType_ == EditType::SCALE);

  xCone_.shape->setVisible(editType_ == EditType::MOVE);
  yCone_.shape->setVisible(editType_ == EditType::MOVE);
  zCone_.shape->setVisible(editType_ == EditType::MOVE);

  xCube_.shape->setVisible(editType_ == EditType::SCALE);
  yCube_.shape->setVisible(editType_ == EditType::SCALE);
  zCube_.shape->setVisible(editType_ == EditType::SCALE);

  xTorus_.shape->setVisible(editType_ == EditType::ROTATE);
  yTorus_.shape->setVisible(editType_ == EditType::ROTATE);
  zTorus_.shape->setVisible(editType_ == EditType::ROTATE);

  canvas_->update();
}

void
Gizmo3DObj::
setParentObj(Object3D *o)
{
  parentObj_ = o;

  updateEditType();
}

bool
Gizmo3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  return Object3D::getValue(name, args, value);
}

bool
Gizmo3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if (name == "edit_type") {
    auto lstr = value.toLower();

    if      (lstr == "select")
      setEditType(EditType::SELECT);
    else if (lstr == "move")
      setEditType(EditType::MOVE);
    else if (lstr == "scale")
      setEditType(EditType::SCALE);
    else if (lstr == "rotate")
      setEditType(EditType::ROTATE);
    else
      return false;
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Gizmo3DObj::
updateObjects()
{
  if (! isNeedsUpdate())
    return;

  needsUpdate_ = false;

  //---

  updateModelMatrix();
}

void
Gizmo3DObj::
updateModelMatrix()
{
  setModelMatrix();
}

//---

bool
Gizmo3DObj::
intersectRay(const CVector3D &p1, const CVector3D &p2, CPoint3D &pi1, CPoint3D &pi2) const
{
  for (const auto &shape : shapes_) {
    if (! shape.shape->isVisible())
      continue;

    if (shape.shape->intersectRay(p1, p2, pi1, pi2))
      return true;
  }

  return false;
}

bool
Gizmo3DObj::
pressPoint(const CPoint2D &p1) const
{
  QPointF p2(p1.x, p1.y);

  auto *camera = canvas_->currentCamera();

  Canvas3D::MinFaceData minFaceData;

  const auto &projectionMatrix = camera->perspectiveMatrix();
  const auto &viewMatrix       = camera->viewMatrix();

  auto pvMatrix = projectionMatrix*viewMatrix;

  for (const auto &shape : shapes_) {
    if (! shape.shape->isVisible())
      continue;

    shape.shape->updateModelMatrix();

    const auto &faceDatas = shape.shape->getFaceDatas();
    if (faceDatas.empty()) continue;

    const auto &modelMatrix = shape.shape->modelMatrix();
    const auto &meshMatrix  = shape.shape->meshMatrix();

    auto matrix = pvMatrix*modelMatrix*meshMatrix;

    uint ii = 0;

    for (const auto &faceData : faceDatas) {
      auto points = canvas_->getFacePoints(faceData, matrix);

      QPolygonF poly;

      for (const auto &p : points) {
        auto pp = p.toPoint2D();

        poly << QPointF(pp.x, pp.y);
      }

      if (poly.containsPoint(p2, Qt::WindingFill)) {
        auto c = Util::pointsCenter(points).toPoint2D();

        auto d = c.distanceTo(p1);

        if (! minFaceData.object || d < minFaceData.dist) {
          minFaceData.object = shape.shape;
          minFaceData.dist   = d;
        }
      }

      ++ii;
    }
  }

  if (minFaceData.object) {
    std::cerr << minFaceData.object->id().toStdString() << "\n";
    return true;
  }

  return false;
}

//---

void
Gizmo3DObj::
loadBuffer()
{
  for (const auto &shape : shapes_)
    shape.shape->loadBuffer();
}

CBBox3D
Gizmo3DObj::
calcBBox()
{
  rawBBox_ = CBBox3D();
  bbox_    = CBBox3D();

  for (const auto &shape : shapes_) {
    bbox_    += shape.shape->calcBBox();
    rawBBox_ += shape.shape->getRawBBox();
  }

  return bbox_;
}

void
Gizmo3DObj::
render()
{
  CPoint3D pos;
  CPoint3D angles;
  double   xs { 1 }, ys { 1 }, zs { 1 };

  if (parentObj()) {
    auto rawBBox = parentObj_->getRawBBox();

    pos = parentObj_->origin();

    xs = 1.1*rawBBox.getXSize();
    ys = 1.1*rawBBox.getYSize();
    zs = 1.1*rawBBox.getZSize();

    angles = parentObj_->angles();
  }

  Shape3DObj::initDraw(canvas_, s_program);

  auto drawShape = [&](const ShapeData &shape) {
    if (! shape.shape->isVisible())
      return;

    shape.shape->setPosition(pos + 1.1*shape.offset);

    shape.shape->setScales(CPoint3D(xs, ys, zs));

    shape.shape->setAngles(angles + shape.angles);

    shape.shape->render(s_program);
  };

  for (const auto &shape : shapes_)
    drawShape(shape);

  Shape3DObj::termDraw(canvas_);
}

}
