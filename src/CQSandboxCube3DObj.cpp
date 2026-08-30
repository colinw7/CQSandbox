#include <CQSandboxCube3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>

#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
Cube3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Cube3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Cube3DObj::
Cube3DObj(Canvas3D *canvas) :
 Shape3DObj(canvas)
{
}

void
Cube3DObj::
init()
{
  Object3D::init();

  //---

  auto xc = 0.0;
  auto yc = 0.0;
  auto zc = 0.0;
  auto r  = 1.0;

  std::vector<CVector3D> v;
  v.resize(8);

  v[0] = CVector3D(xc + r/2, yc - r/2, zc + r/2);
  v[1] = CVector3D(xc + r/2, yc - r/2, zc - r/2);
  v[2] = CVector3D(xc + r/2, yc + r/2, zc - r/2);
  v[3] = CVector3D(xc + r/2, yc + r/2, zc + r/2);
  v[4] = CVector3D(xc - r/2, yc - r/2, zc + r/2);
  v[5] = CVector3D(xc - r/2, yc - r/2, zc - r/2);
  v[6] = CVector3D(xc - r/2, yc + r/2, zc - r/2);
  v[7] = CVector3D(xc - r/2, yc + r/2, zc + r/2);

  struct FaceData {
    std::vector<CVector3D> points;
    CVector3D              normal { 0, 1, 0 };

    void setPoints(const CVector3D &v1, const CVector3D &v2,
                   const CVector3D &v3, const CVector3D &v4) {
      points.push_back(v1);
      points.push_back(v2);
      points.push_back(v3);
      points.push_back(v4);
    }
  };

  std::vector<FaceData> faceData;
  faceData.resize(6);

  faceData[0].setPoints(v[0], v[1], v[2], v[3]); // Right
  faceData[1].setPoints(v[1], v[5], v[6], v[2]); // Back
  faceData[2].setPoints(v[5], v[4], v[7], v[6]); // Left
  faceData[3].setPoints(v[4], v[0], v[3], v[7]); // Front
  faceData[4].setPoints(v[3], v[2], v[6], v[7]); // Top
  faceData[5].setPoints(v[4], v[5], v[1], v[0]); // Bottom

  faceData[0].normal = CVector3D( 1.0,  0.0,  0.0);
  faceData[1].normal = CVector3D( 0.0,  0.0, -1.0);
  faceData[2].normal = CVector3D(-1.0,  0.0,  0.0);
  faceData[3].normal = CVector3D( 0.0,  0.0,  1.0);
  faceData[4].normal = CVector3D( 0.0,  1.0,  0.0);
  faceData[5].normal = CVector3D( 0.0, -1.0,  0.0);

  std::vector<CVector2D> faceTexCoords;
  faceTexCoords.resize(4);

  faceTexCoords[0] = CVector2D(0.0, 0.0);
  faceTexCoords[1] = CVector2D(1.0, 0.0);
  faceTexCoords[2] = CVector2D(1.0, 1.0);
  faceTexCoords[3] = CVector2D(0.0, 1.0);

  std::vector<CVector3D> points;
  std::vector<CVector2D> texCoords;
  std::vector<CVector3D> normals;

  auto addPoint = [&](const CVector3D &p, const CVector2D &t, const CVector3D &n) {
    points   .push_back(p);
    texCoords.push_back(t);
    normals  .push_back(n);
  };

  auto addFaceData = [&](const FaceData &data) {
    addPoint(data.points[0], faceTexCoords[0], data.normal);
    addPoint(data.points[1], faceTexCoords[1], data.normal);
    addPoint(data.points[2], faceTexCoords[2], data.normal);

    addPoint(data.points[2], faceTexCoords[2], data.normal);
    addPoint(data.points[3], faceTexCoords[3], data.normal);
    addPoint(data.points[0], faceTexCoords[0], data.normal);
  };

  for (const auto &d : faceData)
    addFaceData(d);

  shapeData_.setPoints   (points);
  shapeData_.setTexCoords(texCoords);
  shapeData_.setNormals  (normals);

  Shape3DObj::init();
}

}
