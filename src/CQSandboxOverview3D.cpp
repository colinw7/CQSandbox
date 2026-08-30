#include <CQSandboxOverview3D.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxGeomObject.h>
#include <CQSandboxGroup3DObj.h>
#include <CQSandboxModel3DObj.h>
#include <CQSandboxParticleList3DObj.h>
#include <CQSandboxPath3DObj.h>
#include <CQSandboxShape3DObj.h>
#include <CQSandboxSprite3DObj.h>
#include <CQSandboxSurface3DObj.h>
#include <CQSandboxText3DObj.h>
#include <CQSandboxCamera.h>
#include <CQSandboxLight3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxStatus.h>
#include <CQSandboxUtil.h>

#include <CQPixmapCache.h>
#include <CQRubberBand.h>
#include <CQGLBuffer.h>
#include <CQGLTexture.h>

#include <CGeomScene3D.h>
#include <CGeomObject3D.h>

#ifdef CQ_PERF_GRAPH
#include <CQPerfMonitor.h>
#else
struct CQPerfTrace {
  CQPerfTrace(const char *) { }
};
#endif

#include <QPainter>
#include <QMouseEvent>

namespace {

double polygonArea(const QPolygonF &poly) {
  double area = 0.0;

  int n = poly.size();

  int i1 = n - 1;
  int i2 = 0;

  for ( ; i2 < n; i1 = i2++) {
    const auto &p1 = poly[i1];
    const auto &p2 = poly[i2];

    area += std::abs(p1.x()*p2.y() - p1.y()*p2.x());
  }

  return area;
}

CPoint3D pointsCenter(const std::vector<CPoint3D> &points) {
  CPoint3D c(0, 0, 0);

  if (! points.empty()) {
    for (const auto &p : points)
      c += p;

    c /= points.size();
  }

  return c;
}

QPoint toQPoint(const CPoint2D &p) { return QPoint(p.x, p.y); }

}

namespace CQSandbox {

Overview3D::
Overview3D(App *app) :
 app_(app)
{
  setObjectName("overview");

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  setFocusPolicy(Qt::StrongFocus);

  setMouseTracking(true);

  //---

  lightPixmap_ = CQPixmapCacheInst->getSizedPixmap("LIGHT", QSize(32, 32));

  rubberBand_ = new CQRubberBand(this);

  //---

  auto initView = [&](ViewData &view, int ind, ViewType type, const QString &name) {
    view.ind = ind; view.range = new CDisplayRange2D; view.type = type; view.name = name;

    views_.push_back(&view);
  };

  initView(xview_, 0, ViewType::XY    , "XY");
  initView(yview_, 1, ViewType::ZY    , "ZY");
  initView(zview_, 2, ViewType::XZ    , "XZ");
  initView(pview_, 3, ViewType::THREED, "3D");

  views2d_.push_back(&xview_);
  views2d_.push_back(&yview_);
  views2d_.push_back(&zview_);
}

Overview3D::
~Overview3D()
{
}

void
Overview3D::
init()
{
  auto *canvas = app_->canvas3D();

  auto *camera   = canvas->camera();
  auto *fpCamera = canvas->camera();

//connect(canvas_, SIGNAL(animStateChanged()), this, SLOT(invalidate()));
//connect(canvas_, SIGNAL(animTimeChanged()), this, SLOT(invalidate()));

  connect(camera  , SIGNAL(stateChangedSignal()), this, SLOT(cameraChangeSlot()));
  connect(fpCamera, SIGNAL(stateChangedSignal()), this, SLOT(cameraChangeSlot()));

  for (auto *light : canvas->lights())
    connect(light, SIGNAL(changedSignal()), this, SLOT(lightChangeSlot()));
}

void
Overview3D::
setEditType(const EditType &v)
{
  editType_ = v;

  updateState();

  Q_EMIT editTypeChanged();
}

void
Overview3D::
setSelectType(const SelectType &v)
{
  selectType_ = v;

  updateState();

  Q_EMIT selectTypeChanged();
}

void
Overview3D::
invalidate()
{
  setValid(false);

  update();
}

void
Overview3D::
cameraChangeSlot()
{
  update();
}

void
Overview3D::
lightChangeSlot()
{
  update();
}

void
Overview3D::
resizeEvent(QResizeEvent *)
{
  updateRange();
}

void
Overview3D::
updateState()
{
}

void
Overview3D::
updateRange()
{
  auto zoomFactor = xview_.range->zoomFactor();

  w_ = width ();
  h_ = height();

  auto w2 = w_/2.0;
  auto h2 = h_/2.0;

  int x1 = 0, x2 = x1 + w2, x3 = x2 + w2;
  int y1 = 0, y2 = y1 + h2, y3 = y2 + h2;

  xview_.range->setPixelRange(x1, y1, x2, y2); // XY
  yview_.range->setPixelRange(x2, y1, x3, y2); // ZY
  zview_.range->setPixelRange(x1, y2, x2, y3); // XZ
  pview_.range->setPixelRange(x2, y2, x3, y3); // 3D

  for (auto *view : views2d_)
    view->range->setEqualScale(isEqualScale());

  for (auto *v : views_)
    v->range->zoomOut(zoomFactor);

  invalidate();
}

void
Overview3D::
paintEvent(QPaintEvent *)
{
  CQPerfTrace trace("Overview3D::paintEvent");

  //---

  QPainter painter(this);

  drawData_.painter = &painter;

  //---

  auto *canvas = app_->canvas3D();
  auto *camera = canvas->currentCamera();

  drawData_.projectionMatrix = camera->perspectiveMatrix();
  drawData_.viewMatrix       = camera->viewMatrix();
  drawData_.pvMatrix         = drawData_.projectionMatrix*drawData_.viewMatrix;

  drawData_.near = camera->near();
  drawData_.far  = camera->far();

  drawData_.cameraOrigin   = camera->origin();
  drawData_.cameraPosition = camera->position();
  drawData_.cameraFront    = camera->front();

  //---

  if (! isValid()) {
    updateObjects();

    setValid(true);
  }

  updateBBox();

  //---

  // draw background
  painter.fillRect(rect(), bgColor());

  //---

  // draw view borders (sets view rect)

  auto drawPixelBorder = [&](ViewData &viewData) {
    bool current = (viewData.ind == ind_);

    QPen pen;
    pen.setColor(current ? Qt::red : Qt::black);
    pen.setWidthF(current ? 3 : 0);
    painter.setPen(pen);

    QBrush brush(current ? QColor(220, 220, 240) : bgColor());
    painter.setBrush(brush);

    double pxmin, pymin, pxmax, pymax;
    viewData.range->getPixelRange(&pxmin, &pymin, &pxmax, &pymax);

    viewData.rect = QRectF(pxmin, pymin, pxmax - pxmin - 1, pymax - pymin - 1);
    painter.drawRect(viewData.rect);

    painter.setBrush(Qt::NoBrush);
  };

  for (auto *view : views_)
    drawPixelBorder(*view);

  //---

  drawObjects();

  //---

  drawCameras();

  drawLights();

  //---

  painter.save();

  drawTexts();

  painter.restore();

  //---

  // draw labels
  painter.setPen(Qt::black);

  QFontMetrics fm(font());

  auto drawTitle = [&](const ViewData &view) {
    painter.setClipRect(view.rect);

    double pxmin, pymin, pxmax, pymax;
    view.range->getPixelRange(&pxmin, &pymin, &pxmax, &pymax);

    auto r = QRectF(pxmin + 4, pymin + 4, fm.horizontalAdvance(view.name) + 4, fm.height() + 4);
    painter.drawText(r, view.name);
  };

  for (auto *view : views2d_)
    drawTitle(*view);
}

void
Overview3D::
updateObjects()
{
  CQPerfTrace trace("Overview3D::updateObjects");

  //---

  drawData_.bbox = CBBox3D();

  auto *canvas = app_->canvas3D();

  for (auto *object : canvas->objects()) {
    if (! object->isVisible())
      continue;

    updateObject(object);
  }
}

void
Overview3D::
updateObject(Object3D *object)
{
  auto *group3DObj      = dynamic_cast<Group3DObj        *>(object);
  auto *model3DObj      = dynamic_cast<Model3DObj        *>(object);
  auto *particleListObj = dynamic_cast<ParticleList3DObj *>(object);
  auto *pathObj         = dynamic_cast<Path3DObj         *>(object);
  auto *shapeObj        = dynamic_cast<Shape3DObj        *>(object);
  auto *spriteObj       = dynamic_cast<Sprite3DObj       *>(object);
  auto *surface3DObj    = dynamic_cast<Surface3DObj      *>(object);
  auto *textObj         = dynamic_cast<Text3DObj         *>(object);

  if      (group3DObj)
    updateGroup(group3DObj);
  else if (model3DObj)
    updateModel(model3DObj);
  else if (particleListObj)
    updateParticleList(particleListObj);
  else if (pathObj)
    updatePath(pathObj);
  else if (shapeObj)
    updateShape(shapeObj);
  else if (spriteObj)
    updateSprite(spriteObj);
  else if (surface3DObj)
    updateSurface(surface3DObj);
  else if (textObj)
    updateText(textObj);
}

void
Overview3D::
updateGroup(Group3DObj *obj)
{
  for (auto *object : obj->objects()) {
    if (! object->isVisible())
      continue;

    updateObject(object);
  }
}

void
Overview3D::
updateModel(Model3DObj *obj)
{
  updateGeomObject(obj->object());
}

void
Overview3D::
updateParticleList(ParticleList3DObj *obj)
{
  const auto &mm = obj->modelMatrix();

  const auto &points = obj->points();

  auto np = points.size();

  for (uint i = 0; i < np; ++i) {
    drawData_.bbox += mm*points[i].vector();
  }
}

void
Overview3D::
updatePath(Path3DObj *obj)
{
  const auto &mm = obj->modelMatrix();

  const auto &points = obj->points();

  auto np = points.size();

  for (uint i = 0; i < np; i += 2) {
    auto p1 = mm*points[i + 0].point();
    auto p2 = mm*points[i + 1].point();

    drawData_.bbox += p1;
    drawData_.bbox += p2;
  }
}

void
Overview3D::
updateShape(Shape3DObj *obj)
{
  const auto &mm = obj->modelMatrix();

  const auto &shapeData = obj->shapeData();

  const auto &inds   = shapeData.indices();
  const auto &points = shapeData.points();

  auto ni = inds.size();

  if (ni > 0) {
    for (uint i = 0; i < ni; i += 3) {
      drawData_.bbox += mm*points[inds[i + 0]].point();
      drawData_.bbox += mm*points[inds[i + 1]].point();
      drawData_.bbox += mm*points[inds[i + 2]].point();
    }
  }
  else {
    if      (shapeData.isUseTriangleStrip()) {
      for (const auto &p : points)
        drawData_.bbox += mm*p;
    }
    else if (shapeData.isUseTriangleFan()) {
      for (const auto &p : points)
        drawData_.bbox += mm*p;
    }
    else {
      auto np = points.size();

      for (uint i = 0; i < np; i += 3) {
        drawData_.bbox += mm*points[i + 0].point();
        drawData_.bbox += mm*points[i + 1].point();
        drawData_.bbox += mm*points[i + 2].point();
      }
    }
  }
}

void
Overview3D::
updateSprite(Sprite3DObj *obj)
{
  auto *texture = obj->currentTexture();
  if (! texture) return;

  auto pos = obj->position();

  drawData_.bbox += pos;
}

void
Overview3D::
updateSurface(Surface3DObj *obj)
{
  const auto &mm = obj->modelMatrix();

  const auto &inds   = obj->indices();
  const auto &points = obj->points();

  auto ni = inds.size();

  if (ni > 0) {
    for (uint i = 0; i < ni; i += 3) {
      drawData_.bbox += mm*points[inds[i + 0]].point();
      drawData_.bbox += mm*points[inds[i + 1]].point();
      drawData_.bbox += mm*points[inds[i + 2]].point();
    }
  }
  else {
    auto np = points.size();

    for (uint i = 0; i < np; i += 3) {
      drawData_.bbox += mm*points[i + 0].point();
      drawData_.bbox += mm*points[i + 1].point();
      drawData_.bbox += mm*points[i + 2].point();
    }
  }
}

void
Overview3D::
updateText(Text3DObj *obj)
{
  auto pos = obj->position();

  drawData_.bbox += pos;
}

void
Overview3D::
updateGeomObject(CGeomObject3D *object)
{
  CQPerfTrace trace("Overview3D::updateGeomObject");

  if (! object->getVisible())
    return;

  //---

  auto *canvas = app_->canvas3D();

  auto *geomObject = dynamic_cast<GeomObject *>(object);
  assert(geomObject);

  auto *geomObject1 = geomObject;

  if (object->refObject()) {
    geomObject1 = dynamic_cast<GeomObject *>(object->refObject());
    assert(geomObject1);

    updateGeomObject(geomObject1);
  }

  //---

  drawData_.modelMatrix = CMatrix3DH(object->getHierTransform());
  drawData_.meshMatrix  = CMatrix3DH(object->getMeshGlobalTransform());

#if 0
  if (refObject && refObject != object)
    drawData_.modelMatrix = refObject->getHierTransform()*drawData_.modelMatrix;
#endif

  //---

  auto *animObject = geomObject1->getAnimObject();

  auto animName = (animObject ? animObject->animName() : "");

  bool isAnim = false;

  if (canvas->isAnimEnabled())
    isAnim = (animObject && animName != "");

  if (isAnim) {
    auto meshNodeId = geomObject1->getMeshNode();

    CGeomNodeData *node = nullptr;

    if (meshNodeId >= 0)
      node = const_cast<CGeomNodeData *>(&animObject->getNode(meshNodeId));

    auto isJointed = (node && geomObject1->isJointed());

    if (node && ! isJointed) {
      auto animTime = animObject->animTime();

      drawData_.meshMatrix =
        CMatrix3DH(geomObject1->getNodeAnimHierTransform(*node, animName, animTime));
    }
  }

  //---

  auto *nodeMatrices = (isAnim ? &canvas->getObjectNodeMatrices(animObject) : nullptr);

  //---

  auto &faces = drawData_.objFaces[object];

  faces.clear();

  auto *buffer = geomObject1->buffer();

  for (const auto &faceData : geomObject1->faceDatas()) {
    Face face;

    face.face  = faceData.face;
    face.color = faceData.color;

    for (int i = 0; i < faceData.len; ++i) {
      CQGLBuffer::PointData data;
      buffer->getPointData(faceData.pos + i, data);

      auto p = data.point->point();

      if (isAnim) {
        auto *vertex = geomObject1->getVertexP(*data.ind);

        if (vertex->hasJointData())
          p = canvas->adjustAnimPoint(*vertex, p, *nodeMatrices);
      }

      p = drawData_.modelMatrix*drawData_.meshMatrix*p;

      face.points.push_back(p);

      drawData_.bbox += p;
    }

    faces.push_back(face);
  }

  //---

  for (auto *child : object->children()) {
    updateGeomObject(child);
  }
}

void
Overview3D::
updateBBox()
{
  if (bboxSet_)
    return;

  bboxSet_ = true;

  bbox_ = drawData_.bbox;

  std::cerr << "Overview3D BBox: " << bbox_ << "\n";

  auto c = bbox_.getCenter();

  xs_ = bbox_.getXSize();
  ys_ = bbox_.getYSize();
  zs_ = bbox_.getZSize();

  xview_.range->setWindowRange(c.x - xs_, c.y - ys_, c.x + xs_, c.y + ys_); // XY
  yview_.range->setWindowRange(c.z + zs_, c.y - ys_, c.z - zs_, c.y + ys_); // ZY
  zview_.range->setWindowRange(c.x - xs_, c.z + zs_, c.x + xs_, c.z - zs_); // XZ
  pview_.range->setWindowRange(-1, -1, 1, 1); // 3D
}

void
Overview3D::
drawObjects()
{
  auto *canvas = app_->canvas3D();

  auto strokeColor   = this->strokeColor  (); strokeColor  .setAlpha(255*strokeAlpha());
  auto fillColor     = this->fillColor    (); fillColor    .setAlpha(255*fillAlpha  ());
  auto selectedColor = this->selectedColor(); selectedColor.setAlpha(255*strokeAlpha());

  drawData_.painter->setPen(strokeColor);
  drawData_.painter->setBrush(fillColor);

  //---

  drawData_.viewSortedPolygon2DArray.clear();
  drawData_.viewSortedLine2DArray   .clear();
  drawData_.viewSortedPoint2DArray  .clear();

  drawData_.painter->save();

  for (auto *object : canvas->objects()) {
    if (! object->isVisible())
      continue;

    drawObject(object);
  }

  //---

  for (auto &pv : drawData_.viewSortedPolygon2DArray) {
    auto *view = views_[pv.first];

    drawData_.painter->setClipRect(view->rect);

    for (auto &pp : pv.second) {
      for (auto &pd : pp.second) {
        if (pd.solid) {
          drawData_.painter->setPen(Qt::NoPen);
          drawData_.painter->setBrush(fillColor);

          drawData_.painter->drawPolygon(&pd.points[0], int(pd.points.size()));
        }

        if (pd.wireframe || pd.selected) {
          drawData_.painter->setPen(pd.selected ? selectedColor : strokeColor);
          drawData_.painter->setBrush(Qt::NoBrush);

          drawData_.painter->drawPolygon(&pd.points[0], int(pd.points.size()));
        }
      }
    }
  }

  for (auto &pv : drawData_.viewSortedPoint2DArray) {
    auto *view = views_[pv.first];

    drawData_.painter->setClipRect(view->rect);

    for (auto &pp : pv.second) {
      for (auto &pd : pp.second) {
        auto px = pd.point.x();
        auto py = pd.point.y();

        if (pd.size > 0) {
          drawData_.painter->setPen(strokeColor);
          drawData_.painter->setBrush(pd.color);

          drawData_.painter->drawEllipse(QRectF(px - pd.size/2, py - pd.size/2, pd.size, pd.size));
        }
        else {
          drawData_.painter->setPen(pd.color);

          drawData_.painter->drawPoint(px, py);
        }

        if (pd.label != "") {
          drawData_.painter->setPen(pd.selected ? selectedColor : strokeColor);

          drawData_.painter->drawText(px, py, pd.label);
        }
      }
    }
  }

  //---

  drawData_.painter->restore();
}

void
Overview3D::
drawObject(Object3D *object)
{
  auto *group3DObj      = dynamic_cast<Group3DObj        *>(object);
  auto *model3DObj      = dynamic_cast<Model3DObj        *>(object);
  auto *particleListObj = dynamic_cast<ParticleList3DObj *>(object);
  auto *pathObj         = dynamic_cast<Path3DObj         *>(object);
  auto *shapeObj        = dynamic_cast<Shape3DObj        *>(object);
  auto *spriteObj       = dynamic_cast<Sprite3DObj       *>(object);
  auto *surface3DObj    = dynamic_cast<Surface3DObj      *>(object);
  auto *textObj         = dynamic_cast<Text3DObj         *>(object);

  if      (group3DObj)
    drawGroup(group3DObj);
  else if (model3DObj)
    drawModel(model3DObj);
  else if (particleListObj)
    drawParticleList(particleListObj);
  else if (pathObj)
    drawPath(pathObj);
  else if (shapeObj)
    drawShape(shapeObj);
  else if (spriteObj)
    drawSprite(spriteObj);
  else if (surface3DObj)
    drawSurface(surface3DObj);
  else if (textObj)
    drawText(textObj);
}

void
Overview3D::
drawGroup(Group3DObj *obj)
{
  for (auto *object : obj->objects()) {
    if (! object->isVisible())
      continue;

    drawObject(object);
  }
}

void
Overview3D::
drawModel(Model3DObj *obj)
{
  CQPerfTrace trace("Overview3D::drawModel");

  auto *object = obj->object();

  drawGeomObject(object);
}

void
Overview3D::
drawGeomObject(CGeomObject3D *object)
{
  bool objSelected = object->getSelected();

  auto &faces = drawData_.objFaces[object];

  for (auto &face : faces) {
    bool faceSelected = face.face->getSelected() || objSelected;

    drawModelPolygon(face.points, faceSelected);
  }

  //---

  if (isBasisVisible() && objSelected) {
    CVector3D u, v, w;
    getBasis(object, u, v, w);

    CBBox3D bbox;
    object->getTransformedModelBBox(bbox);

    auto c = bbox.getCenter();

    drawLine(c, c + u);
    drawLine(c, c + v);
    drawLine(c, c + w);
  }

  //---

  for (auto *child : object->children()) {
    drawGeomObject(child);
  }
}

void
Overview3D::
getBasis(CGeomObject3D *object, CVector3D &u, CVector3D &v, CVector3D &w) const
{
  CVector3D u1, v1, w1;
  object->getBasis(u1, v1, w1);

  const auto &modelMatrix = object->getHierTransform();

  u = (modelMatrix*u1).normalized();
  v = (modelMatrix*v1).normalized();
  w = (modelMatrix*w1).normalized();
}

void
Overview3D::
drawCameras()
{
  if (! isCameraVisible() && editType() != EditType::CAMERA)
    return;

  auto *canvas = app_->canvas3D();
  auto *camera = canvas->currentCamera();

  drawCamera(camera);

#if 0
  if (isEyeLineVisible())
    drawEyeLine();
#endif
}

void
Overview3D::
drawCamera(CGLCameraIFace *camera)
{
#if 0
  Camera::Shape shape;
  camera->getCameraShape(shape);
#endif

  auto pos    = camera->position();
  auto origin = camera->origin();

  //---

  drawData_.painter->setBrush(Qt::NoBrush);

  //---

  // draw camera position, origin and direction vectors
  auto drawCameraVector = [&](const CVector3D &v, const QColor &c, const QString &text) {
    drawData_.painter->setPen(c);
    drawVector(pos, v, text);
  };

  auto front = camera->front();
  auto up    = camera->up   ();
  auto right = camera->right();

  drawData_.pointSize = 3;

  drawPoint(pos   .point(), "P", SymbolType::CROSS);
  drawPoint(origin.point(), "O", SymbolType::CROSS);

  drawCameraVector(front, QColor(255, 0, 0, 255), "F");
  drawCameraVector(up   , QColor(0, 255, 0, 255), "U");
  drawCameraVector(right, QColor(0, 0, 255, 255), "R");

  //---

#if 0
  // draw view frustrum
  drawData_.painter->setPen(Qt::black);

  drawLine(shape.p11, shape.p12, "");
  drawLine(shape.p12, shape.p22, "");
  drawLine(shape.p22, shape.p21, "");
  drawLine(shape.p21, shape.p11, "");

  drawLine(pos.point(), shape.p11, "");
  drawLine(pos.point(), shape.p12, "");
  drawLine(pos.point(), shape.p22, "");
  drawLine(pos.point(), shape.p21, "");
#endif

  //---

  // draw camera orbit
  drawData_.painter->setPen(QColor(100, 100, 100));
  drawData_.painter->setBrush(Qt::NoBrush);

  drawSphere(origin.point(), pos.point());
}

#if 0
void
Overview3D::
getCameraShape(Camera *camera, CameraShape &shape) const
{
  auto pos    = camera->position();
  auto origin = camera->origin();

  auto dist = CVector3D(pos, origin).length();

  auto front = camera->front();
  auto up    = camera->up   ();
  auto right = camera->right();

  auto fov = CMathGen::DegToRad(camera->fov());

  auto m1 = CMatrix3D::rotation(-fov/2.0, up   );
  auto m2 = CMatrix3D::rotation( fov/2.0, up   );
  auto m3 = CMatrix3D::rotation(-fov/2.0, right);
  auto m4 = CMatrix3D::rotation( fov/2.0, right);

  auto front1 = m1*front;
  auto front2 = m2*front;

  auto front11 = m3*front1;
  auto front12 = m4*front1;
  auto front21 = m3*front2;
  auto front22 = m4*front2;

  shape.p11 = (pos + dist*front11).point();
  shape.p12 = (pos + dist*front12).point();
  shape.p21 = (pos + dist*front21).point();
  shape.p22 = (pos + dist*front22).point();
}
#endif

void
Overview3D::
drawLights()
{
  if (! isLightsVisible() && editType_ != EditType::LIGHT)
    return;

  drawData_.painter->setPen(QColor(0, 0, 0, 255));
  drawData_.painter->setBrush(Qt::NoBrush);

  auto *canvas = app_->canvas3D();

  // draw lights
  for (auto *light : canvas->lights()) {
    if (! light->getEnabled())
      continue;

    auto p = light->getPosition();

    drawPixmap(p, lightPixmap_);

    if      (light->getType() == Light3D::Type::DIRECTIONAL) {
      auto d = light->getDirection();

      drawVector(CVector3D(p), d, "D");
    }
    else if (light->getType() == Light3D::Type::POINT) {
      auto r = light->getPointRadius();

      drawCircle(p, r, "P");
    }
    else if (light->getType() == Light3D::Type::SPOT) {
      auto d = light->getSpotDirection();
      auto a = light->getSpotCutOffAngle();

      auto a1 = CMathGen::DegToRad(a)/2.0;

      drawCone(CVector3D(p), d, a1);
    }
#if 0
    else if (light->type() == Light3D::Type::FLASHLIGHT) {
      // TODO
    }
#endif
  }
}

void
Overview3D::
drawTexts()
{
#if 0
  auto *canvas = app_->canvas3D();

  for (auto *text : canvas->texts()) {
    drawText(text);
  }
#endif
}

void
Overview3D::
drawParticleList(ParticleList3DObj *obj)
{
  bool objSelected = obj->isSelected();

  drawData_.modelMatrix = obj->modelMatrix();

  const auto &points = obj->points();
  const auto &colors = obj->colors();

  drawData_.pointSize = pointSize();

  auto np = points.size();

  for (uint i = 0; i < np; ++i) {
    const auto &c = colors[i];

    drawData_.painter->setPen(Util::colorToQColor(c));

    drawModelPoint(points[i].point(), "", objSelected);
  }
}

void
Overview3D::
drawPath(Path3DObj *obj)
{
  drawData_.modelMatrix = obj->modelMatrix();

  const auto &points = obj->points();

  auto np = points.size();

  for (uint i = 0; i < np; i += 2) {
    auto p1 = points[i + 0].point();
    auto p2 = points[i + 1].point();

    drawModelLine(p1, p2);
  }
}

void
Overview3D::
drawShape(Shape3DObj *obj)
{
  bool objSelected = obj->isSelected();

  drawData_.painter->setPen(objSelected ? selectedColor() : strokeColor());

  drawData_.modelMatrix = obj->modelMatrix();

  const auto &shapeData = obj->shapeData();

  const auto &inds   = shapeData.indices();
  const auto &points = shapeData.points();

  auto ni = inds.size();

  if (ni > 0) {
    for (uint i = 0; i < ni; i += 3) {
      std::vector<CPoint3D> poly;

      poly.push_back(points[inds[i + 0]].point());
      poly.push_back(points[inds[i + 1]].point());
      poly.push_back(points[inds[i + 2]].point());

      drawModelPolygon(poly, objSelected);
    }
  }
  else {
    if      (shapeData.isUseTriangleStrip()) {
      for (const auto &p : points)
        drawModelPoint(p.point(), "", objSelected);
    }
    else if (shapeData.isUseTriangleFan()) {
      for (const auto &p : points)
        drawModelPoint(p.point(), "", objSelected);
    }
    else {
      auto np = points.size();

      for (uint i = 0; i < np; i += 3) {
        std::vector<CPoint3D> poly;

        poly.push_back(points[i + 0].point());
        poly.push_back(points[i + 1].point());
        poly.push_back(points[i + 2].point());

        drawModelPolygon(poly, objSelected);
      }
    }
  }
}

void
Overview3D::
drawSprite(Sprite3DObj *obj)
{
  auto *texture = obj->currentTexture();
  if (! texture) return;

  auto pos = obj->position();

  const auto &image = texture->getImage();

  auto w = image.width ();
  auto h = image.height();

  auto image1 = image.scaled(w*obj->xscale(), h*obj->yscale());

  drawImage(pos, image1);
}

void
Overview3D::
drawSurface(Surface3DObj *obj)
{
  drawData_.modelMatrix = obj->modelMatrix();

  const auto &inds   = obj->indices();
  const auto &points = obj->points();

  auto ni = inds.size();

  if (ni > 0) {
    for (uint i = 0; i < ni; i += 3) {
      std::vector<CPoint3D> poly;

      poly.push_back(points[inds[i + 0]].point());
      poly.push_back(points[inds[i + 1]].point());
      poly.push_back(points[inds[i + 2]].point());

      drawModelPolygon(poly, /*selected*/false);
    }
  }
  else {
    auto np = points.size();

    for (uint i = 0; i < np; i += 3) {
      std::vector<CPoint3D> poly;

      poly.push_back(points[i + 0].point());
      poly.push_back(points[i + 1].point());
      poly.push_back(points[i + 2].point());

      drawModelPolygon(poly, /*selected*/false);
    }
  }
}

void
Overview3D::
drawText(Text3DObj *obj)
{
  bool objSelected = obj->isSelected();

  auto pos = obj->position();

  drawData_.painter->setPen(objSelected ? selectedColor() : strokeColor());

  drawModelPoint(CPoint3D(pos.getX(), pos.getY(), pos.getZ()), obj->text(), objSelected);
}

void
Overview3D::
drawModelPolygon(const std::vector<CPoint3D> &points, bool selected) const
{
  auto drawPolygon2D = [&](const ViewData &view, double pos, const std::vector<CPoint2D> &points) {
    PolygonData polygonData;

    polygonData.solid     = isSolid();
    polygonData.wireframe = isWireframe();
    polygonData.selected  = selected;

    polygonData.pen   = drawData_.painter->pen();
    polygonData.brush = drawData_.painter->brush();

    polygonData.pen.setWidthF(polygonData.selected ? 3 : 0);

    for (const auto &p : points) {
      double px, py;
      view.range->windowToPixel(p.x, p.y, &px, &py);

      polygonData.points.push_back(QPointF(px, py));
    }

    drawData_.viewSortedPolygon2DArray[view.ind][-pos].push_back(polygonData);
  };

  std::vector<CPoint2D> xpoints, ypoints, zpoints, ppoints;

  bool behind = isZClip();

  for (const auto &p : points) {
    auto p1 = drawData_.modelMatrix*p;

    xpoints.push_back(CPoint2D(p1.getX(), p1.getY())); // XY
    ypoints.push_back(CPoint2D(p1.getZ(), p1.getY())); // ZY
    zpoints.push_back(CPoint2D(p1.getX(), p1.getZ())); // XZ

    if (isZClip()){
      auto dir1 = p1 - drawData_.cameraPosition;
      auto a = drawData_.cameraFront.dotProduct(dir1);
      if (a >= 0) behind = false;
    }

    auto p2 = drawData_.pvMatrix*p1;

    ppoints.push_back(CPoint2D(p2.getX(), p2.getY()));
  }

  auto c = pointsCenter(points);

  drawPolygon2D(xview_, c.x, xpoints);
  drawPolygon2D(yview_, c.y, ypoints);
  drawPolygon2D(zview_, c.z, zpoints);

  if (! behind)
    drawPolygon2D(pview_, c.z, ppoints);
}

void
Overview3D::
drawCone(const CVector3D &p, const CVector3D &d, double a) const
{
  auto m1 = CMatrix2D::rotation( a);
  auto m2 = CMatrix2D::rotation(-a);

  auto drawCone2D = [&](const ViewData &view, const CVector3D &p,
                        const CVector3D &d, double sx, double sy) {
    auto p1 = view.viewPoint(p.point());
    auto dp = view.viewPoint(d.point());

    drawData_.painter->setClipRect(view.rect);

    auto s = std::sqrt(sx*sx + sy*sy)/3.0;

    auto d1 = m1*dp;
    auto d2 = m2*dp;

    double px1, py1;
    view.range->windowToPixel(p1.x, p1.y, &px1, &py1);
    double px2, py2;
    view.range->windowToPixel(p1.x + s*d1.x, p1.y + s*d1.y, &px2, &py2);
    double px3, py3;
    view.range->windowToPixel(p1.x + s*d2.x, p1.y + s*d2.y, &px3, &py3);

    drawData_.painter->drawLine(px1, py1, px2, py2);
    drawData_.painter->drawLine(px1, py1, px3, py3);
  };

  drawCone2D(xview_, p, d, xs_, ys_); // XY
  drawCone2D(yview_, p, d, zs_, ys_); // ZY
  drawCone2D(zview_, p, d, xs_, zs_); // XZ
}

void
Overview3D::
drawModelLine(const CPoint3D &p1, const CPoint3D &p2,
              const QString & /*label*/, bool /*selected*/) const
{
  auto drawLine2D = [&](const ViewData &view, double /*pos*/,
                        const CPoint2D &p1, const CPoint2D &p2) {
    CPoint2D pp1, pp2;
    view.range->windowToPixel(p1, pp1);
    view.range->windowToPixel(p2, pp2);

    drawData_.painter->setClipRect(view.rect);
    drawData_.painter->drawLine(pp1.x, pp1.y, pp2.x, pp2.y);
  };

  auto pm1 = drawData_.modelMatrix*p1;
  auto pm2 = drawData_.modelMatrix*p2;

  auto c = (p1 + p2)/2.0;

  drawLine2D(xview_, c.z, CPoint2D(pm1.getX(), pm1.getY()), CPoint2D(pm2.getX(), pm2.getY())); // XY
  drawLine2D(yview_, c.x, CPoint2D(pm1.getZ(), pm1.getY()), CPoint2D(pm2.getZ(), pm2.getY())); // ZY
  drawLine2D(zview_, c.y, CPoint2D(pm1.getX(), pm1.getZ()), CPoint2D(pm2.getX(), pm2.getZ())); // XZ

  pm1 = drawData_.pvMatrix*p1;
  pm2 = drawData_.pvMatrix*p2;

  drawLine2D(pview_, c.z, CPoint2D(pm1.getX(), pm1.getY()), CPoint2D(pm2.getX(), pm2.getY()));
}

void
Overview3D::
drawLine(const CPoint3D &p1, const CPoint3D &p2, const QString & /*label*/, bool /*selected*/) const
{
  auto drawLine2D = [&](const ViewData &view, const CPoint2D &p1, const CPoint2D &p2) {
    CPoint2D pp1, pp2;
    view.range->windowToPixel(p1, pp1);
    view.range->windowToPixel(p2, pp2);

    drawData_.painter->setClipRect(view.rect);
    drawData_.painter->drawLine(pp1.x, pp1.y, pp2.x, pp2.y);
  };

  drawLine2D(xview_, CPoint2D(p1.getX(), p1.getY()), CPoint2D(p2.getX(), p2.getY())); // XY
  drawLine2D(yview_, CPoint2D(p1.getZ(), p1.getY()), CPoint2D(p2.getZ(), p2.getY())); // ZY
  drawLine2D(zview_, CPoint2D(p1.getX(), p1.getZ()), CPoint2D(p2.getX(), p2.getZ())); // XZ

  auto pm1 = drawData_.pvMatrix*p1;
  auto pm2 = drawData_.pvMatrix*p2;

  drawLine2D(pview_, CPoint2D(pm1.getX(), pm1.getY()), CPoint2D(pm2.getX(), pm2.getY()));
}

void
Overview3D::
drawVector(const CVector3D &p, const CVector3D &d, const QString &label) const
{
  auto drawVector2D = [&](const ViewData &view, const CPoint2D &p,
                          const CPoint2D &d, double sx, double sy, const QString &label) {
    drawData_.painter->setClipRect(view.rect);

    auto s = std::sqrt(sx*sx + sy*sy)/3.0;

    double px1, py1;
    view.range->windowToPixel(p.x, p.y, &px1, &py1);
    double px2, py2;
    view.range->windowToPixel(p.x + s*d.x, p.y + s*d.y, &px2, &py2);

    drawData_.painter->drawLine(px1, py1, px2, py2);

    if (label != "")
      drawData_.painter->drawText(px2, py2, label);
  };

  auto x1 = p.getX(), y1 = p.getY(), z1 = p.getZ();
  auto x2 = d.getX(), y2 = d.getY(), z2 = d.getZ();

  auto xs = drawData_.bbox.getXSize();
  auto ys = drawData_.bbox.getYSize();
  auto zs = drawData_.bbox.getZSize();

  drawVector2D(xview_, CPoint2D(x1, y1), CPoint2D(x2, y2), xs, ys, label); // XY
  drawVector2D(yview_, CPoint2D(z1, y1), CPoint2D(z2, y2), zs, ys, label); // ZY
  drawVector2D(zview_, CPoint2D(x1, z1), CPoint2D(x2, z2), xs, zs, label); // XZ
}

void
Overview3D::
drawCircle(const CPoint3D &origin, double r, const QString &label) const
{
  auto drawCircle2D = [&](const ViewData &view, const CPoint2D &o, double r, const QString &label) {
    drawData_.painter->setClipRect(view.rect);

    double px1, py1;
    view.range->windowToPixel(o.x - r, o.y - r, &px1, &py1);
    double px2, py2;
    view.range->windowToPixel(o.x + r, o.y + r, &px2, &py2);

    drawData_.painter->drawEllipse(QRectF(px1, py1, px2 - px1, py2 - py1));

    if (label != "")
      drawData_.painter->drawText(px2, py2, label);
  };

  auto xo = CPoint2D(origin.x, origin.y);
  auto yo = CPoint2D(origin.z, origin.y);
  auto zo = CPoint2D(origin.x, origin.z);

  drawCircle2D(xview_, xo, r, label); // XY
  drawCircle2D(yview_, yo, r, label); // ZY
  drawCircle2D(zview_, zo, r, label); // XZ
}

void
Overview3D::
drawSphere(const CPoint3D &origin, const CPoint3D &pos) const
{
  auto drawCircle2D = [&](const ViewData &view, const CPoint2D &o, double r) {
    drawData_.painter->setClipRect(view.rect);

    double px1, py1;
    view.range->windowToPixel(o.x - r, o.y - r, &px1, &py1);
    double px2, py2;
    view.range->windowToPixel(o.x + r, o.y + r, &px2, &py2);

    drawData_.painter->drawEllipse(QRectF(px1, py1, px2 - px1, py2 - py1));
  };

  auto x1 = CPoint2D(origin.x, origin.y); auto x2 = CPoint2D(pos.x, pos.y);
  auto y1 = CPoint2D(origin.z, origin.y); auto y2 = CPoint2D(pos.z, pos.y);
  auto z1 = CPoint2D(origin.x, origin.z); auto z2 = CPoint2D(pos.x, pos.z);

  auto rx = x1.distanceTo(x2);
  auto ry = y1.distanceTo(y2);
  auto rz = z1.distanceTo(z2);

  drawCircle2D(xview_, x1, rx); // XY
  drawCircle2D(yview_, y1, ry); // ZY
  drawCircle2D(zview_, z1, rz); // XZ
}

void
Overview3D::
drawImage(const CPoint3D &p, const QImage &image) const
{
  auto w = image.width ();
  auto h = image.height();

  auto px = windowToPixelX(CPoint2D(p.x, p.y));
  auto py = windowToPixelY(CPoint2D(p.z, p.y));
  auto pz = windowToPixelZ(CPoint2D(p.x, p.z));

  drawData_.painter->setClipRect(xview_.rect);
  drawData_.painter->drawImage(px.x - w/2, px.y - h/2, image);

  drawData_.painter->setClipRect(yview_.rect);
  drawData_.painter->drawImage(py.x - w/2, py.y - h/2, image);

  drawData_.painter->setClipRect(zview_.rect);
  drawData_.painter->drawImage(pz.x - w/2, pz.y - h/2, image);
}

void
Overview3D::
drawModelPoint(const CPoint3D &p, const QString &label, bool selected) const
{
  auto drawPoint2D = [&](const ViewData &view, double pos, const CPoint2D &p) {
    PointData pointData;

    pointData.selected = selected;

    pointData.color = drawData_.painter->pen().color();
    pointData.size  = drawData_.pointSize;

    pointData.pen   = drawData_.painter->pen();
    pointData.brush = drawData_.painter->brush();

    pointData.pen.setWidthF(pointData.selected ? 3 : 0);

    double px, py;
    view.range->windowToPixel(p.x, p.y, &px, &py);

    pointData.point = QPointF(px, py);
    pointData.label = label;

    drawData_.viewSortedPoint2DArray[view.ind][-pos].push_back(pointData);
  };

  auto p1 = drawData_.modelMatrix*p;

  drawPoint2D(xview_, p.z, CPoint2D(p1.getX(), p1.getY())); // XY
  drawPoint2D(yview_, p.x, CPoint2D(p1.getZ(), p1.getY())); // ZY
  drawPoint2D(zview_, p.y, CPoint2D(p1.getX(), p1.getZ())); // XZ

  auto p2 = drawData_.pvMatrix*p1;

  drawPoint2D(pview_, p.z, CPoint2D(p2.getX(), p2.getY()));
}

void
Overview3D::
drawPoint(const CPoint3D &p, const QString &label, const SymbolType &symbolType) const
{
  auto drawPoint2D = [&](const ViewData &view, const CPoint2D &p) {
    double px, py;
    view.range->windowToPixel(p.x, p.y, &px, &py);

    auto s = drawData_.pointSize;

    drawData_.painter->setClipRect(view.rect);

    if (s > 0) {
      drawData_.painter->setBrush(drawData_.painter->pen().color());
      drawData_.painter->setPen(strokeColor());

      if      (symbolType == SymbolType::PLUS) {
        drawData_.painter->drawLine(px - s, py, px + s, py);
        drawData_.painter->drawLine(px, py - s, px, py + s);
      }
      else if (symbolType == SymbolType::CROSS) {
        drawData_.painter->drawLine(px - s, py - s, px + s, py + s);
        drawData_.painter->drawLine(px - s, py + s, px + s, py - s);
      }
      else if (symbolType == SymbolType::ELLIPSE)
        drawData_.painter->drawEllipse(QRectF(px - s/2, py - s/2, s, s));
      else
        drawData_.painter->drawPoint(px, py);
    }
    else
      drawData_.painter->drawPoint(px, py);

    if (label != "")
      drawData_.painter->drawText(px, py, label);
  };

  auto p1 = drawData_.modelMatrix*p;

  drawPoint2D(xview_, CPoint2D(p1.getX(), p1.getY())); // XY
  drawPoint2D(yview_, CPoint2D(p1.getZ(), p1.getY())); // ZY
  drawPoint2D(zview_, CPoint2D(p1.getX(), p1.getZ())); // XZ

  auto p2 = drawData_.pvMatrix*p1;

  drawPoint2D(pview_, CPoint2D(p2.getX(), p2.getY()));
}

void
Overview3D::
drawPixmap(const CPoint3D &p, const QPixmap &pixmap) const
{
  auto s = pixmap.width();

  auto drawPixmap2D = [&](const ViewData &view, const CPoint2D &p, const QPixmap &pixmap) {
    drawData_.painter->setClipRect(view.rect);

    double px, py;
    view.range->windowToPixel(p.x, p.y, &px, &py);

    drawData_.painter->drawPixmap(px - s/2, py - s/2, pixmap);
  };

  drawPixmap2D(xview_, CPoint2D(p.getX(), p.getY()), pixmap); // XY
  drawPixmap2D(yview_, CPoint2D(p.getZ(), p.getY()), pixmap); // ZY
  drawPixmap2D(zview_, CPoint2D(p.getX(), p.getZ()), pixmap); // XZ
}

CPoint2D
Overview3D::
windowToPixelX(const CPoint2D &w) const
{
  CPoint2D p;
  xview_.range->windowToPixel(w, p);
  return p;
}

CPoint2D
Overview3D::
windowToPixelY(const CPoint2D &w) const
{
  CPoint2D p;
  yview_.range->windowToPixel(w, p);
  return p;
}

CPoint2D
Overview3D::
windowToPixelZ(const CPoint2D &w) const
{
  CPoint2D p;
  zview_.range->windowToPixel(w, p);
  return p;
}

CPoint2D
Overview3D::
windowToPixelP(const CPoint2D &w) const
{
  CPoint2D p;
  pview_.range->windowToPixel(w, p);
  return p;
}

void
Overview3D::
mousePressEvent(QMouseEvent *e)
{
  mouseData_.pressed  = true;
  mouseData_.button   = e->button();

  mouseData_.isShift   = (e->modifiers() & Qt::ShiftModifier);
  mouseData_.isControl = (e->modifiers() & Qt::ControlModifier);

  mouseData_.pressPos = CPoint2D(e->x(), e->y());
  mouseData_.movePos1 = mouseData_.pressPos;
  mouseData_.movePos2 = mouseData_.movePos1;

  //---

  if      (mouseData_.button == Qt::LeftButton) {
    if      (editType() == EditType::SELECT) {
      rubberBand_->setBounds(toQPoint(mouseData_.pressPos), toQPoint(mouseData_.movePos1));
      rubberBand_->show();
    }
    else if (editType() == EditType::CAMERA) {
      if      (mouseData_.isShift)
        setCameraPosition(mouseData_.pressPos);
      else if (mouseData_.isControl)
        setCameraOrigin(mouseData_.pressPos);
    }
    else if (editType() == EditType::LIGHT) {
      if      (mouseData_.isShift)
        setLightPosition(mouseData_.pressPos);
      else if (mouseData_.isControl)
        setLightDirection(mouseData_.pressPos);
    }
  }
  else if (mouseData_.button == Qt::MiddleButton) {
  }
  else if (mouseData_.button == Qt::RightButton) {
  }
}

void
Overview3D::
mouseMoveEvent(QMouseEvent *e)
{
  mouseData_.movePos2 = CPoint2D(e->x(), e->y());

  //---

  if (mouseData_.pressed) {
    if      (mouseData_.button == Qt::LeftButton) {
      if      (editType() == EditType::SELECT) {
        rubberBand_->setBounds(toQPoint(mouseData_.pressPos), toQPoint(mouseData_.movePos2));
      }
      else if (editType() == EditType::CAMERA) {
        if      (mouseData_.isShift)
          setCameraPosition(mouseData_.movePos2);
        else if (mouseData_.isControl)
          setCameraOrigin(mouseData_.movePos2);
      }
      else if (editType() == EditType::LIGHT) {
        if      (mouseData_.isShift)
          setLightPosition(mouseData_.movePos2);
        else if (mouseData_.isControl)
          setLightDirection(mouseData_.movePos2);
      }
    }
    else if (mouseData_.button == Qt::MiddleButton) {
      CPoint2D p2;
      if      (xview_.pressRange(mouseData_.movePos2, p2)) {
        CPoint2D p1;
        xview_.pressRange(mouseData_.movePos1, p1);

        auto dx = p1.x - p2.x;
        auto dy = p1.y - p2.y;

        xview_.range->scroll(dx, dy);
        invalidate();
      }
      else if (yview_.pressRange(mouseData_.movePos2, p2)) {
        CPoint2D p1;
        yview_.pressRange(mouseData_.movePos1, p1);

        auto dx = p1.x - p2.x;
        auto dy = p1.y - p2.y;

        yview_.range->scroll(dx, dy);
        invalidate();
      }
      else if (zview_.pressRange(mouseData_.movePos2, p2)) {
        CPoint2D p1;
        zview_.pressRange(mouseData_.movePos1, p1);

        auto dx = p1.x - p2.x;
        auto dy = p1.y - p2.y;

        zview_.range->scroll(dx, dy);
        invalidate();
      }
      else if (pview_.pressRange(mouseData_.movePos2, p2)) {
        auto *canvas = app_->canvas3D();
        auto *camera = canvas->currentCamera();

        auto dx = CMathUtil::sign(mouseData_.movePos2.x - mouseData_.movePos1.x);
        auto dy = CMathUtil::sign(mouseData_.movePos2.y - mouseData_.movePos1.y);

        if      (mouseData_.button == Qt::MiddleButton) {
          auto da = M_PI/180.0;

          camera->rotateY(-dx*da);
          camera->rotateX(-dy*da);
        }
        else if (mouseData_.button == Qt::RightButton) {
          camera->moveRight(-dx/100.0);
          camera->moveUp   ( dy/100.0);
        }

        invalidate();
      }
    }
    else if (mouseData_.button == Qt::RightButton) {
    }
  }

  //---

  auto showPoint = [&](const CPoint3D &p1) {
    auto posStr = QString("X=%1, Y=%2, Z=%3").arg(p1.x).arg(p1.y).arg(p1.z);
    app_->status()->setMouseLabel(posStr);
  };

  int ind = ind_;

  ind_ = -1;

  CPoint2D p;
  if (xview_.pressRange(mouseData_.movePos2, p)) {
    ind_ = xview_.ind; showPoint(CPoint3D(p.x, p.y, 0.0)); } // XY
  if (yview_.pressRange(mouseData_.movePos2, p)) {
    ind_ = yview_.ind; showPoint(CPoint3D(0.0, p.y, p.x)); } // ZY
  if (zview_.pressRange(mouseData_.movePos2, p)) {
    ind_ = zview_.ind; showPoint(CPoint3D(p.x, 0.0, p.y)); } // XZ
  if (pview_.pressRange(mouseData_.movePos2, p))
    ind_ = pview_.ind;                                       // XZ

  if (! mouseData_.pressed) {
    if (ind_ != ind)
      invalidate();
  }

  mouseData_.movePos1 = mouseData_.movePos2;
}

void
Overview3D::
mouseReleaseEvent(QMouseEvent *e)
{
  mouseData_.movePos2 = CPoint2D(e->x(), e->y());

  if (mouseData_.button == Qt::LeftButton) {
    if (editType() == EditType::SELECT) {
      bool clear = ! mouseData_.isControl;

      int dx = std::abs(mouseData_.pressPos.x - mouseData_.movePos2.x);
      int dy = std::abs(mouseData_.pressPos.y - mouseData_.movePos2.y);

      if (dx < 4 && dy < 4)
        selectObjectAt(mouseData_.pressPos, clear);
      else
        selectObjectIn(mouseData_.pressPos, rubberBand_->bounds(), clear);

      rubberBand_->hide();
    }
  }

  mouseData_.pressed = false;
}

void
Overview3D::
wheelEvent(QWheelEvent *e)
{
  auto dw = e->angleDelta().y()/250.0;

  if      (dw < 0) {
    for (auto *v : views2d_)
      v->range->zoomOut(1.05);
  }
  else if (dw > 0) {
    for (auto *v : views2d_)
      v->range->zoomIn(1.05);
  }

  update();
}

void
Overview3D::
keyPressEvent(QKeyEvent *e)
{
  auto *canvas = app_->canvas3D();
  auto *camera = canvas->currentCamera();

  auto k = e->key();

  mouseData_.isShift   = (e->modifiers() & Qt::ShiftModifier);
  mouseData_.isControl = (e->modifiers() & Qt::ControlModifier);

//auto bbox = canvas->bbox();
  auto bbox = bbox_;

  auto d = bbox.getMaxSize()/100.0;
  auto da = M_PI/180.0;

  //---

  if      (editType() == EditType::CAMERA) {
    if      (k == Qt::Key_Left) {
      if (ind_ == 3)
        camera->moveRight(-d);
    }
    else if (k == Qt::Key_Right) {
      if (ind_ == 3)
        camera->moveRight(d);
    }
    else if (k == Qt::Key_Up) {
      if (ind_ == 3)
        camera->moveUp(d);
    }
    else if (k == Qt::Key_Down) {
      if (ind_ == 3)
        camera->moveUp(-d);
    }
    else if (k == Qt::Key_Plus) {
      if (ind_ == 3)
        camera->moveFront(d);
    }
    else if (k == Qt::Key_Minus) {
      if (ind_ == 3)
        camera->moveFront(-d);
    }
    else if (k == Qt::Key_W) {
      if (ind_ == 3)
        camera->rotateX(da);
    }
    else if (k == Qt::Key_S) {
      if (ind_ == 3)
        camera->rotateX(-da);
    }
    else if (k == Qt::Key_A) {
      if      (ind_ == 0)
        camera->moveAroundZ(-3*d);
      else if (ind_ == 1)
        camera->moveAroundX(-3*d);
      else if (ind_ == 2)
        camera->moveAroundY(-3*d);
      else if (ind_ == 3)
        camera->rotateY(da);
    }
    else if (k == Qt::Key_D) {
      if      (ind_ == 0)
        camera->moveAroundZ(3*d);
      else if (ind_ == 1)
        camera->moveAroundX(3*d);
      else if (ind_ == 2)
        camera->moveAroundY(3*d);
      else if (ind_ == 3)
        camera->rotateY(-da);
    }
    else if (k == Qt::Key_Q) {
      if (ind_ == 3)
        camera->rotateZ(-da);
    }
    else if (k == Qt::Key_E) {
      if (ind_ == 3)
        camera->rotateZ(da);
    }
    else if (k == Qt::Key_P) {
      if (ind_ >= 0 && ind_ <= 2)
        camera->setPitch(camera->pitch() + (mouseData_.isShift ? -0.1 : 0.1));
    }
    else if (k == Qt::Key_R) {
      if (ind_ >= 0 && ind_ <= 2)
        camera->setRoll(camera->roll() + (mouseData_.isShift ? -0.1 : 0.1));
    }
    else if (k == Qt::Key_Y) {
      if (ind_ >= 0 && ind_ <= 2)
        camera->setYaw(camera->yaw() + (mouseData_.isShift ? -0.1 : 0.1));
    }
  }
  else if (editType() == EditType::LIGHT) {
  }
  else if (editType() == EditType::SELECT) {
#if 0
    if (! mouseData_.isShift && ! mouseData_.isControl) {
      if      (e->key() == Qt::Key_G) { // Grab
        setEditType(EditType::MOVE);
        return;
      }
      else if (e->key() == Qt::Key_S) { // Scale
        setEditType(EditType::SCALE);
        return;
      }
      else if (e->key() == Qt::Key_R) { // Rotate
        setEditType(EditType::ROTATE);
        return;
      }
    }
#endif
  }

  if      (k == Qt::Key_Plus) {
    for (auto *v : views_)
      v->range->zoomIn(1.1);
  }
  else if (k == Qt::Key_Minus) {
    for (auto *v : views_)
      v->range->zoomOut(1.1);
  }
  else if (k == Qt::Key_Home) {
    for (auto *v : views_)
      v->range->reset();

    bboxSet_ = false;
  }
#if 0
  else if (k == Qt::Key_1) {
    drawData_.pointLabels[0].show = ! drawData_.pointLabels[0].show;
  }
  else if (k == Qt::Key_2) {
    drawData_.pointLabels[1].show = ! drawData_.pointLabels[1].show;
  }
  else if (k == Qt::Key_3) {
    drawData_.pointLabels[2].show = ! drawData_.pointLabels[2].show;
  }
  else if (k == Qt::Key_4) {
    drawData_.pointLabels[3].show = ! drawData_.pointLabels[3].show;
  }
  else if (k == Qt::Key_5) {
    drawData_.pointLabels[4].show = ! drawData_.pointLabels[4].show;
  }
#endif
  else if (k == Qt::Key_Space) {
    bboxSet_ = false;
    updateBBox();
  }

  canvas->update();

  invalidate();

  update();
}

void
Overview3D::
selectObjectAt(const CPoint2D &p, bool clear)
{
  auto *canvas = app_->canvas3D();
  auto *camera = canvas->currentCamera();

  auto pos = camera->position();

  CPoint2D p1;

  if      (xview_.pressRange(p, p1))
    selectObjectAt1(xview_, CPoint3D(p1.x, p1.y, pos.z()), clear); // XY
  else if (yview_.pressRange(p, p1))
    selectObjectAt1(yview_, CPoint3D(pos.x(), p1.y, p1.x), clear); // ZY
  else if (zview_.pressRange(p, p1))
    selectObjectAt1(zview_, CPoint3D(p1.x, pos.y(), p1.y), clear); // XZ
}

void
Overview3D::
selectObjectIn(const CPoint2D &p, const QRect &r, bool clear)
{
  auto *canvas = app_->canvas3D();

  ViewType viewType { ViewType::NONE };

  CPoint2D p1;

  if      (xview_.pressRange(p, p1))
    viewType = xview_.type;
  else if (yview_.pressRange(p, p1))
    viewType = yview_.type;
  else if (zview_.pressRange(p, p1))
    viewType = zview_.type;
  else
    return;

  auto pr1 = pixelToView(viewType, r.topLeft    ());
  auto pr2 = pixelToView(viewType, r.bottomRight());

  auto r1 = QRectF(QPointF(pr1.x, pr1.y), QPointF(pr2.x, pr2.y)).normalized();

  auto selectType = this->selectType();

  if      (selectType == SelectType::OBJECT) {
    std::set<Object3D *> selectObjectSet;

    for (auto *object : canvas->objects()) {
      if (! object->isVisible())
        continue;

      auto *model3DObj      = dynamic_cast<Model3DObj        *>(object);
      auto *particleListObj = dynamic_cast<ParticleList3DObj *>(object);
      auto *pathObj         = dynamic_cast<Path3DObj         *>(object);
      auto *shapeObj        = dynamic_cast<Shape3DObj        *>(object);
      auto *spriteObj       = dynamic_cast<Sprite3DObj       *>(object);
      auto *surface3DObj    = dynamic_cast<Surface3DObj      *>(object);
      auto *textObj         = dynamic_cast<Text3DObj         *>(object);

      if      (model3DObj) {
        if (selectModelIn(model3DObj, viewType, r1))
          selectObjectSet.insert(object);
      }
      else if (particleListObj) {
        //selectParticleListIn(particleListObj);
      }
      else if (pathObj) {
        //selectPathIn(pathObj);
      }
      else if (shapeObj) {
        if (selectShapeIn(shapeObj, viewType, r1))
          selectObjectSet.insert(object);
      }
      else if (spriteObj) {
        //selectSpriteIn(spriteObj);
      }
      else if (surface3DObj) {
        //selectSurfaceIn(surface3DObj);
      }
      else if (textObj) {
        //selectTextIn(textObj);
      }
    }

    std::vector<Object3D *> selectObjects;

    for (auto *object : selectObjectSet)
      selectObjects.push_back(object);

    canvas->selectObjects(selectObjects, clear, /*update*/true);
  }
  else if (selectType == SelectType::FACE) {
#if 0
    std::vector<CGeomFace3D *> selectFaces;

    for (auto &pf : drawData_.objFaces) {
      auto *object = pf.first;

      const auto &geomData = pf.second;

      //---

      for (auto *faceData : geomData.faceDatas) {
        QPolygonF poly;

        for (const auto &pf : faceData->points) {
          auto pf1 = viewQPoint(viewType, pf);

          poly << pf1;
        }

        if (poly.intersects(QPolygonF(r1)))
          selectFaces.push_back(faceData->face);
      }
    }

    canvas->selectFaces(selectFaces, clear, /*update*/true);
#endif
  }
  else if (selectType == SelectType::EDGE) {
#if 0
#endif
  }
  else if (selectType == SelectType::POINT) {
#if 0
    CQCamera3DCanvas::ObjectSelectInds selectInds;

    for (auto &pf : drawData_.objFaces) {
      auto *object = pf.first;

      const auto &geomData = pf.second;

      //---

      for (auto *faceData : geomData.faceDatas) {
        int i = 0;

        for (const auto &pf : faceData->points) {
          auto pf1 = viewQPoint(viewType, pf);

          if (r1.contains(pf1)) {
            auto vInd = faceData->face->getVertex(i);

            selectInds[object].insert(vInd);
          }

          ++i;
        }
      }
    }

    canvas->selectVertices(selectInds);
#endif
  }
}

bool
Overview3D::
selectModelIn(Model3DObj *obj, ViewType viewType, const QRectF &r)
{
  auto *object = obj->object();

  auto &faces = drawData_.objFaces[object];

  //---

  bool intersect = false;

  for (const auto &face : faces) {
    QPolygonF poly;

    for (const auto &pf : face.points) {
      auto pf1 = viewQPoint(viewType, pf);

      poly << pf1;
    }

    if (poly.intersects(QPolygonF(r))) {
      intersect = true;
      break;
    }
  }

  return intersect;
}

bool
Overview3D::
selectShapeIn(Shape3DObj *obj, ViewType viewType, const QRectF &r)
{
  const auto &mm = obj->modelMatrix();

  const auto &shapeData = obj->shapeData();

  const auto &inds   = shapeData.indices();
  const auto &points = shapeData.points();

  auto ni = inds.size();

  if (ni > 0) {
    for (uint i = 0; i < ni; i += 3) {
      QPolygonF poly;

      poly.push_back(viewQPoint(viewType, mm*points[inds[i + 0]].point()));
      poly.push_back(viewQPoint(viewType, mm*points[inds[i + 1]].point()));
      poly.push_back(viewQPoint(viewType, mm*points[inds[i + 2]].point()));

      if (poly.intersects(QPolygonF(r)))
        return true;
    }
  }
  else {
    if      (shapeData.isUseTriangleStrip()) {
      return false;
    }
    else if (shapeData.isUseTriangleFan()) {
      return false;
    }
    else {
      auto np = points.size();

      for (uint i = 0; i < np; i += 3) {
        QPolygonF poly;

        poly.push_back(viewQPoint(viewType, mm*points[i + 0].point()));
        poly.push_back(viewQPoint(viewType, mm*points[i + 1].point()));
        poly.push_back(viewQPoint(viewType, mm*points[i + 2].point()));

        if (poly.intersects(QPolygonF(r)))
          return true;
      }
    }
  }

  return false;
}

void
Overview3D::
selectObjectAt1(const ViewData &view, const CPoint3D &p, bool clear)
{
  SelectAtData selectAtData;

  // get view point
  auto p1 = viewQPoint(view.type, p);

  auto *canvas = app_->canvas3D();

  for (auto *object : canvas->objects()) {
    if (! object->isVisible())
      continue;

    auto *model3DObj      = dynamic_cast<Model3DObj        *>(object);
    auto *particleListObj = dynamic_cast<ParticleList3DObj *>(object);
    auto *pathObj         = dynamic_cast<Path3DObj         *>(object);
    auto *shapeObj        = dynamic_cast<Shape3DObj        *>(object);
    auto *spriteObj       = dynamic_cast<Sprite3DObj       *>(object);
    auto *surface3DObj    = dynamic_cast<Surface3DObj      *>(object);
    auto *textObj         = dynamic_cast<Text3DObj         *>(object);

    if      (model3DObj) {
      selectModelAt(model3DObj, view, p1, selectAtData);
    }
    else if (particleListObj) {
    }
    else if (pathObj) {
    }
    else if (shapeObj) {
      selectShapeAt(shapeObj, view, p1, selectAtData);
    }
    else if (spriteObj) {
    }
    else if (surface3DObj) {
    }
    else if (textObj) {
    }
  }

  //---

  if (selectAtData.areaObjFaces.empty()) {
    canvas->deselectAll();
    return;
  }

  //---

  // select associated shape type
  auto selectType = this->selectType();

  if      (selectType == SelectType::OBJECT) {
    const auto &pf = selectAtData.areaObjFaces.begin()->second.begin();

    auto *object = (*pf).first;

    canvas->selectObject(object, clear, /*update*/true);
  }
  else if (selectType == SelectType::FACE) {
    const auto &pf = selectAtData.areaObjFaces.begin()->second.begin();

    auto *face = (*(*pf).second.begin()).face;

    canvas->selectFace(face, clear, /*update*/true);
  }
  else if (selectType == SelectType::EDGE) {
#if 0
    auto pm = viewPoint(view.type, p);

    CGeomEdge3D* minEdge = nullptr;
    double       minDist = 0.0;

    for (auto &pf : drawData_.objFaces) {
      auto *object = pf.first;

      const auto &geomData = pf.second;

      const auto &edges = object->getEdges();

      for (auto *edge : edges) {
        auto getEdgeVertex = [&](uint ind) {
          auto pv = geomData.vertexDatas.find(ind);
          assert(pv != geomData.vertexDatas.end());
          return (*pv).second->p;
        };

        auto pe1 = getEdgeVertex(edge->getStart());
        auto pe2 = getEdgeVertex(edge->getEnd  ());

        auto pf1 = viewPoint(view.type, pe1);
        auto pf2 = viewPoint(view.type, pe2);

        double dist = 0.0;
        (void) CMathGeom2D::PointLineDistance(pm, CLine2D(pf1, pf2), &dist);

        if (! minEdge || dist < minDist) {
          minEdge = edge;
          minDist = dist;
        }
      }
    }

    if (minEdge)
      canvas->selectEdge(minEdge, clear, /*update*/true);
#endif
  }
  else if (selectType == SelectType::POINT) {
#if 0
    auto pm = viewPoint(view.type, p);

    int    minInd  = -1;
    double minDist = 0.0;

    CGeomFace3D *face = nullptr;

    for (auto &pf : drawData_.objFaces) {
      auto *object = pf.first;

      const auto &geomData = pf.second;

      //---

      for (auto *faceData : geomData.faceDatas) {
        int i = 0;

        for (const auto &pf : faceData->points) {
          auto pf1 = viewPoint(view.type, pf);

          auto d = pf1.distanceTo(pm);

          if (! face || d < minDist) {
            face    = faceData->face;
            minInd  = i;
            minDist = d;
          }

          ++i;
        }
      }
    }

    auto  vInd = face->getVertex(minInd);
    auto &v    = object->getVertex(vInd);

    canvas->selectVertex(&v, clear, /*update*/true);
#endif
  }
}

void
Overview3D::
selectModelAt(Model3DObj *obj, const ViewData &view, const QPointF &p1, SelectAtData &selectAtData)
{
  auto *object = obj->object();

  auto &faces = drawData_.objFaces[object];

  //---

  // get inside faces
  for (const auto &face : faces) {
    QPolygonF poly;

    for (const auto &pf : face.points) {
      auto pf1 = viewQPoint(view.type, pf);

      poly.push_back(pf1);
    }

    if (poly.containsPoint(p1, Qt::WindingFill)) {
      auto area = polygonArea(poly);

      selectAtData.areaObjFaces[area][obj].push_back(face);
    }
  }
}

void
Overview3D::
selectShapeAt(Shape3DObj *obj, const ViewData &view, const QPointF &p1, SelectAtData &selectAtData)
{
  auto selectType = this->selectType();

  if (selectType != SelectType::OBJECT)
    return;

  const auto &mm = obj->modelMatrix();

  const auto &shapeData = obj->shapeData();

  const auto &inds   = shapeData.indices();
  const auto &points = shapeData.points();

  auto ni = inds.size();

  if (ni > 0) {
    for (uint i = 0; i < ni; i += 3) {
      QPolygonF poly;

      poly.push_back(viewQPoint(view.type, mm*points[inds[i + 0]].point()));
      poly.push_back(viewQPoint(view.type, mm*points[inds[i + 1]].point()));
      poly.push_back(viewQPoint(view.type, mm*points[inds[i + 2]].point()));

      if (poly.containsPoint(p1, Qt::WindingFill)) {
        Face face;

        face.ind = i;

        auto area = polygonArea(poly);

        selectAtData.areaObjFaces[area][obj].push_back(face);
      }
    }
  }
  else {
    if      (shapeData.isUseTriangleStrip()) {
    }
    else if (shapeData.isUseTriangleFan()) {
    }
    else {
      auto np = points.size();

      for (uint i = 0; i < np; i += 3) {
        QPolygonF poly;

        poly.push_back(viewQPoint(view.type, mm*points[i + 0].point()));
        poly.push_back(viewQPoint(view.type, mm*points[i + 1].point()));
        poly.push_back(viewQPoint(view.type, mm*points[i + 2].point()));

        if (poly.containsPoint(p1, Qt::WindingFill)) {
          Face face;

          face.ind = i;

          auto area = polygonArea(poly);

          selectAtData.areaObjFaces[area][obj].push_back(face);
        }
      }
    }
  }
}

QPointF
Overview3D::
viewQPoint(ViewType viewType, const CPoint3D &p) const
{
  auto p1 = viewPoint(viewType, p);

  return QPointF(p1.x, p1.y);
}

CPoint2D
Overview3D::
viewPoint(ViewType viewType, const CPoint3D &p) const
{
  CPoint2D p1;

  if      (viewType == ViewType::XY)
    p1 = CPoint2D(p.x, p.y);
  else if (viewType == ViewType::ZY)
    p1 = CPoint2D(p.z, p.y);
  else if (viewType == ViewType::XZ)
    p1 = CPoint2D(p.x, p.z);
  else
    assert(false);

  return p1;
}

CPoint2D
Overview3D::
pixelToView(ViewType viewType, const QPointF &p) const
{
  CPoint2D p0(p.x(), p.y());

  CPoint2D p1;

  if      (viewType == ViewType::XY) {
    (void) xview_.pressRange(p0, p1);
  }
  else if (viewType == ViewType::ZY) {
    (void) yview_.pressRange(p0, p1);
  }
  else if (viewType == ViewType::XZ) {
    (void) zview_.pressRange(p0, p1);
  }
  else
    assert(false);

  return p1;
}

void
Overview3D::
setCameraPosition(const CPoint2D &pressPos)
{
  auto *canvas = app_->canvas3D();
  auto *camera = canvas->currentCamera();

  auto position = camera->position();

  CPoint2D p;
  if (xview_.pressRange(pressPos, p))
    camera->setPosition(CVector3D(p.x, p.y, position.z())); // XY
  if (yview_.pressRange(pressPos, p))
    camera->setPosition(CVector3D(position.x(), p.y, p.x)); // ZY
  if (zview_.pressRange(pressPos, p))
    camera->setPosition(CVector3D(p.x, position.y(), p.y)); // XZ
}

void
Overview3D::
setCameraOrigin(const CPoint2D &pressPos)
{
  auto *canvas = app_->canvas3D();
  auto *camera = canvas->currentCamera();

  auto origin = camera->origin();

  CPoint2D p;
  if (xview_.pressRange(pressPos, p))
    camera->setOrigin(CVector3D(p.x, p.y, origin.z())); // XY
  if (yview_.pressRange(pressPos, p))
    camera->setOrigin(CVector3D(origin.x(), p.y, p.x)); // ZY
  if (zview_.pressRange(pressPos, p))
    camera->setOrigin(CVector3D(p.x, origin.y(), p.y)); // XZ
}

void
Overview3D::
setLightPosition(const CPoint2D &pressPos)
{
  auto *canvas = app_->canvas3D();
  auto *light  = canvas->currentLight();

  auto pos = light->getPosition();

  CPoint2D p;

  if (xview_.pressRange(pressPos, p)) light->setPosition(CPoint3D(p.x, p.y, pos.z)); // XY
  if (yview_.pressRange(pressPos, p)) light->setPosition(CPoint3D(pos.x, p.y, p.x)); // ZY
  if (zview_.pressRange(pressPos, p)) light->setPosition(CPoint3D(p.x, pos.y, p.y)); // XZ

  canvas->update();

  //canvas->stateChanged();

  invalidate();
}

void
Overview3D::
setLightDirection(const CPoint2D &pressPos)
{
  auto *canvas = app_->canvas3D();
  auto *light  = canvas->currentLight();

  auto pos = light->getPosition();

  CVector3D dir { 1, 0, 0 };

  if      (light->getType() == Light3D::Type::DIRECTIONAL)
    dir = light->getDirection();
  else if (light->getType() == Light3D::Type::SPOT)
    dir = light->getSpotDirection();
#if 0
  else if (light->type() == Light3D::Type::FLASHLIGHT)
    dir = light->getDirection();
#endif
  else
    return;

  auto pos1 = pos + dir;

  CPoint2D p;

  if (xview_.pressRange(pressPos, p)) pos1 = CPoint3D(p.x, p.y, pos1.z); // XY
  if (yview_.pressRange(pressPos, p)) pos1 = CPoint3D(pos1.x, p.y, p.x); // ZY
  if (zview_.pressRange(pressPos, p)) pos1 = CPoint3D(p.x, pos1.y, p.y); // XZ

  auto dir1 = CVector3D(pos1 - pos).normalized();

  if      (light->getType() == Light3D::Type::DIRECTIONAL) {
    light->setDirection(dir1);
  }
  else if (light->getType() == Light3D::Type::SPOT) {
    light->setSpotDirection(dir1);
  }
#if 0
  else if (light->type() == Light3D::Type::FLASHLIGHT) {
    light->setSpotDirection(dir1);
  }
#endif

  canvas->update();

  //canvas->stateChanged();

  invalidate();
}

}
