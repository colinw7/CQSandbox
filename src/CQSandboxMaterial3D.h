#ifndef CQSandboxMaterial3D_H
#define CQSandboxMaterial3D_H

#include <CQSandboxTexture.h>

#include <CGeomMaterial.h>

#include <QString>
#include <QColor>

namespace CQSandbox {

class Canvas3D;
class Texture;

class Material3D : public CGeomMaterial {
 public:
  Material3D(Canvas3D *canvas);

  //---

  CGeomTexture *diffuseTexture() const override { return diffuseTexture_; }
  void setDiffuseTexture(CGeomTexture *t) override;

  CGeomTexture *normalTexture() const override { return normalTexture_; }
  void setNormalTexture(CGeomTexture *t) override;

  CGeomTexture *specularTexture() const override { return specularTexture_; }
  void setSpecularTexture(CGeomTexture *t) override;

  CGeomTexture *emissiveTexture() const override { return emissiveTexture_; }
  void setEmissiveTexture(CGeomTexture *t) override;

  //---

  double emissiveStrength() const { return emissiveStrength_; }
  void setEmissiveStrength(double r) { emissiveStrength_ = r; }

  double specularStrength() const { return specularStrength_; }
  void setSpecularStrength(double r) { specularStrength_ = r; }

  //---

  double reflectivity() const { return reflectivity_; }
  void setReflectivity(double r) { reflectivity_ = r; }

  double refractivity() const { return refractivity_; }
  void setRefractivity(double r) { refractivity_ = r; }

 private:
  Canvas3D* canvas_ { nullptr };

  Texture* diffuseTexture_  { nullptr };
  Texture* normalTexture_   { nullptr };
  Texture* specularTexture_ { nullptr };
  Texture* emissiveTexture_ { nullptr };

  double emissiveStrength_ { 0.0 };
  double specularStrength_ { 0.0 };

  double reflectivity_ { 0.0 };
  double refractivity_ { 0.0 };
};

}

#endif
