#ifndef CQSandboxSkyboxObj_H
#define CQSandboxSkyboxObj_H

#include <CQSandboxObject3D.h>

#include <QImage>

class CQGLCubemap;

class CGeomObject3D;
class CImportBase;
class CQGLBuffer;
class CQGLTexture;
class CQGLCubemap;
class CGLTexture;

namespace CQSandbox {

class ShaderProgram;

class Skybox3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Skybox3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Skybox"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  bool load();

  void initShader();

  void updateObjectData();

  void render() override;

 private:
  struct FaceData {
    int         pos             { 0 };
    int         len             { 0 };
    CGLTexture *diffuseTexture  { nullptr };
    CGLTexture *specularTexture { nullptr };
    CGLTexture *normalTexture   { nullptr };
  };

  using FaceDatas = std::vector<FaceData>;

  //FaceDatas faceDatas;

  struct ObjectData {
    CQGLBuffer* buffer { nullptr };
    FaceDatas   faceDatas;
  };

  using ObjectDatas = std::map<CGeomObject3D *, ObjectData *>;

  static ShaderProgram* s_program;

  CImportBase* import_ { nullptr };

  std::vector<QImage> images_;
  CQGLTexture*        texture_ { nullptr };
  CQGLCubemap*        cubemap_ { nullptr };
  bool                useCubemap_ { true };

  CPoint3D    sceneCenter_ { 0 , 0, 0 };
  ObjectDatas objectDatas_;

  bool flipYZ_ { false };
};

}

#endif
