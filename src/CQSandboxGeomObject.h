#ifndef CQSandboxGeomObject_H
#define CQSandboxGeomObject_H

#include <CQSandboxFaceData.h>

#include <CGeomObject3D.h>
#include <CBBox3D.h>

class CQGLBuffer;

namespace CQSandbox {

class Canvas3D;
class ShaderProgram;

class GeomObject : public CGeomObject3D {
 public:
  using FaceDatas = std::vector<FaceData>;

 public:
  GeomObject(CGeomScene3D *pscene, const std::string &name);

  GeomObject(const GeomObject &object);

 ~GeomObject();

  GeomObject *dup() const override;

  //---

  const CBBox3D &bbox() const { return bbox_; }
  void setBBox(const CBBox3D &v) { bbox_ = v; }

  //---

  ShaderProgram* program() const { return program_; }

  const FaceDatas &faceDatas() const { return faceDatas_; }

  CQGLBuffer *initBuffer(Canvas3D *canvas);

  CQGLBuffer *getBuffer() const { return buffer_; }

  void addFaceData(const FaceData &faceData);

 private:
  CBBox3D bbox_;

  CQGLBuffer* buffer_ { nullptr };

  ShaderProgram* program_ { nullptr };

  FaceDatas faceDatas_;
};

}

#endif
