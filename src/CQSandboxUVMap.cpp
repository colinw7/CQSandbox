#include <CQSandboxUVMap.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxModel3DObj.h>
#include <CQSandboxShape3DObj.h>
#include <CQSandboxApp.h>
#include <CQSandboxStatus.h>

#include <CQGLBuffer.h>

#include <QPainter>
#include <QMouseEvent>

namespace CQSandbox {

UVMap::
UVMap(Canvas3D *canvas) :
 canvas_(canvas)
{
  setObjectName("uvMap");

  connect(canvas_, SIGNAL(objectsChanged()), this, SLOT(update()));

  connect(canvas_, SIGNAL(selectionChanged()), this, SLOT(update()));

  setFocusPolicy(Qt::StrongFocus);

  setMouseTracking(true);
}

void
UVMap::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.fillRect(rect(), bgColor_);

  w_ = width ();
  h_ = height();

  //---

  auto *object = object_;

  if (! object)
    object = canvas_->getCurrentObject();

  if (! object)
    return;

  //---

  umin_ = 0.0; umax_ = 1.0;
  vmin_ = 0.0; vmax_ = 1.0;

  auto updateFaceTexture = [&](const FaceData &faceData) {
    for (int i = 0; i < faceData.len; ++i) {
      CQGLBuffer::PointData data;
      faceData.buffer->getPointData(faceData.pos + i, data);

      auto tpoint = data.texturePoint;
      if (! tpoint) continue;

      umin_ = std::min(umin_, double(tpoint->x));
      umax_ = std::max(umax_, double(tpoint->x));

      vmin_ = std::min(vmin_, double(tpoint->y));
      vmax_ = std::max(vmax_, double(tpoint->y));
    }
  };

  const auto &faceDatas = object->getFaceDatas();

  if (! isWrapValues()) {
    for (const auto &faceData : faceDatas)
      updateFaceTexture(faceData);

    //---

    double sx = (umax_ - umin_);
    double sy = (vmax_ - vmin_);

    double s = std::max(sx, sy);

    auto umid = (umin_ + umax_)/2.0;
    auto vmid = (vmin_ + vmax_)/2.0;

    umin_ = umid - s/2.0;
    umax_ = umid + s/2.0;

    vmin_ = vmid - s/2.0;
    vmax_ = vmid + s/2.0;
  }

  //---

  texturePoints_.clear();

  painter.setPen(lineColor_);

  //---

  auto fixTValue = [&](double &t, double min, double max) {
    while (t > max)
      t -= (max - min);
    while (t < min)
      t += (max - min);
  };

  auto fixUValue = [&](double &t) { fixTValue(t, umin_, umax_); };
  auto fixVValue = [&](double &t) { fixTValue(t, vmin_, vmax_); };

  auto drawFaceTexture = [&](Object3D *object, const FaceData &faceData, int ind) {
    auto selected = object->isFaceSelected(faceData.buffer->ind(), ind);

    std::vector<QPointF> tpoints;

    int iv = 0;

    for (int i = 0; i < faceData.len; ++i) {
      CQGLBuffer::PointData data;
      faceData.buffer->getPointData(faceData.pos + i, data);

      if (! data.texturePoint)
        continue;

      TexturePoint texturePoint;

      texturePoint.tpoint = CPoint2D(data.texturePoint->x, data.texturePoint->y);

      fixUValue(texturePoint.tpoint.x);
      fixVValue(texturePoint.tpoint.y);

      auto x1 = mapU(texturePoint.tpoint.x);
      auto y1 = mapV(texturePoint.tpoint.y);

      texturePoint.p = QPointF(x1, y1);

      tpoints.push_back(texturePoint.p);

      texturePoints_.push_back(texturePoint);

      ++iv;
    }

    if (selected)
      painter.setPen(Qt::yellow);
    else
      painter.setPen(Qt::white);

    painter.drawPolygon(&tpoints[0], tpoints.size());
  };

#if 0
  for (const auto &faceData : faceDatas) {
    if (! isWrapValues())
      updateFaceTexture(faceData);
  }
#endif

  int i = 0;

  for (const auto &faceData : faceDatas)
    drawFaceTexture(object, faceData, i++);
}

void
UVMap::
mousePressEvent(QMouseEvent *e)
{
  auto pos = e->pos();

  auto p = CPoint2D(unmapU(pos.x()), unmapV(pos.y()));

  double minD;
  int    minI = -1;

  int i = 0;

  for (const auto &tp : texturePoints_) {
    auto d = tp.tpoint.distanceTo(p);

    if (minI < 0 || d < minD) {
      minI = i;
      minD = d;
    }

    ++i;
  }

  if (minI < 0)
    return;

  update();
}

void
UVMap::
mouseMoveEvent(QMouseEvent *e)
{
  auto pos = e->pos();

  auto x = unmapU(pos.x());
  auto y = unmapV(pos.y());

  canvas_->app()->status()->setMouseLabel(QString("%1 %2").arg(x).arg(y));
}

}
