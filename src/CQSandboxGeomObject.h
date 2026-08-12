#ifndef CQSandboxGeomObject_H
#define CQSandboxGeomObject_H

#include <CQSandboxFaceData.h>

#include <CGeomObject3D.h>
#include <CBBox3D.h>

class CQGLBuffer;

namespace CQSandbox {

class Canvas3D;

class GeomObject : public CGeomObject3D {
 public:
  using FaceDatas = std::vector<FaceData>;

 public:
  GeomObject(CGeomScene3D *pscene, const std::string &name);

  GeomObject(const GeomObject &object);

 ~GeomObject();

  GeomObject *dup() const override;

  //---

  CQGLBuffer *buffer() const { return buffer_; }

  const FaceDatas &faceDatas() const { return faceDatas_; }

  CQGLBuffer *initBuffer(Canvas3D *canvas);

  void addFaceData(const FaceData &faceData);

  //---

 private:
  CQGLBuffer* buffer_ { nullptr };

  FaceDatas faceDatas_;
};

}

#endif
