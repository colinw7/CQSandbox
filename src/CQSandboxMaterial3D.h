#ifndef CQSandboxMaterial3D_H
#define CQSandboxMaterial3D_H

#include <QString>
#include <QColor>

class CQGLTexture;

namespace CQSandbox {

class Material3D {
 public:
  Material3D();

  //---

  const QString &name() const { return name_; }
  void setName(const QString &s) { name_ = s; }

  const uint &id() const { return id_; }
  void setId(const uint &v) { id_ = v; }

  //---

  const QColor &diffuseColor() const { return diffuseColor_; }
  void setDiffuseColor(const QColor &v) { diffuseColor_ = v; }

  //---

  CQGLTexture *diffuseTexture() const { return diffuseTexture_; }
  void setDiffuseTexture(CQGLTexture *t);

  CQGLTexture *normalTexture() const { return normalTexture_; }
  void setNormalTexture(CQGLTexture *t);

  CQGLTexture *specularTexture() const { return specularTexture_; }
  void setSpecularTexture(CQGLTexture *t);

  CQGLTexture *emissionTexture() const { return emissionTexture_; }
  void setEmissionTexture(CQGLTexture *t);

  //---

  double emission() const { return emission_; }
  void setEmission(double r) { emission_ = r; }

  double specular() const { return specular_; }
  void setSpecular(double r) { specular_ = r; }

  double shininess() const { return shininess_; }
  void setShininess(double r) { shininess_ = r; }

  //---

  double transparency() const { return transparency_; }
  void setTransparency(double r) { transparency_ = r; }

  double reflectivity() const { return reflectivity_; }
  void setReflectivity(double r) { reflectivity_ = r; }

  double refractivity() const { return refractivity_; }
  void setRefractivity(double r) { refractivity_ = r; }

 private:
  QString name_;
  uint    id_ { 0 };

  QColor diffuseColor_ { Qt::white };

  CQGLTexture* diffuseTexture_  { nullptr };
  CQGLTexture* normalTexture_   { nullptr };
  CQGLTexture* specularTexture_ { nullptr };
  CQGLTexture* emissionTexture_ { nullptr };

  double emission_  { 0.0 };
  double specular_  { 0.0 };
  double shininess_ { 1.0 };

  double transparency_ { 0 };
  double reflectivity_ { 0 };
  double refractivity_ { 0 };
};

}

#endif
