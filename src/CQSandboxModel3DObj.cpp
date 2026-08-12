#include <CQSandboxModel3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxBBox3DObj.h>
#include <CQSandboxApp.h>
#include <CQSandboxGeomObject.h>
#include <CQSandboxUtil.h>

#include <CQGLBuffer.h>
#include <CQGLUtil.h>
#include <CGLTexture.h>
#include <CImportBase.h>
#include <CGeometry3D.h>
#include <CGeomScene3D.h>

#include <CQTclUtil.h>
#include <CImageLib.h>
#include <CFile.h>

#include <QFileInfo>

namespace CQSandbox {

ShaderProgram* Model3DObj::s_program;

Object3D *
Model3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Model3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  if (args.size() >= 1) {
    auto filename = args[0];

    if (! obj->load(filename))
      return nullptr;
  }

  tcl->setResult(name);

  return obj;
}

Model3DObj::
Model3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
  vertShaderFile_ = canvas_->buildDir() + "/shaders/model.vs";
  fragShaderFile_ = canvas_->buildDir() + "/shaders/model.fs";
}

void
Model3DObj::
initShader()
{
  if (s_program)
    return;

  s_program = new ShaderProgram;

  s_program->addVertexFile  (vertShaderFile_);
  s_program->addFragmentFile(fragShaderFile_);

  s_program->link();
}

bool
Model3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if (name == "ref_object") {
    if (! object_)
      return false;

    auto *object1 = object_->createRef();

    object1->setInd(CGeometry3DInst->nextObjectId());

    auto *scene = canvas_->scene();

    scene->addObject(object1);

    auto children = object1->hierChildren();

    for (auto *child : children) {
      child->setInd(CGeometry3DInst->nextObjectId());

      scene->addObject(child);
    }

    scene->addObject(object1);

    QStringList args;
    auto *obj = dynamic_cast<Model3DObj *>(create(canvas_, args));
    if (! obj) return false;

    obj->object_ = object1;
  }
  else
    return Object3D::getValue(name, args, value);

  return true;
}

bool
Model3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto fileToTexture = [&](const QString filename, bool flipY=true) {
    CFile imageFile(filename.toStdString());

    if (! imageFile.exists())
      return static_cast<CGLTexture *>(nullptr);

    CImageFileSrc src(imageFile);

    auto image = CImageMgrInst->createImage(src);

    if (flipY)
      image = image->flippedH();

    return new CGLTexture(image);
  };

  auto resetShader = [&]() {
    if (s_program) {
      delete s_program;

      s_program = nullptr;
    }
  };

  //---

  if      (name == "diffuse_texture") {
    diffuseTexture_ = fileToTexture(value);

    needsUpdate_ = true;
  }
  else if (name == "specular_texture") {
    specularTexture_ = fileToTexture(value);

    needsUpdate_ = true;
  }
  else if (name == "normal_texture") {
    normalTexture_ = fileToTexture(value);

    needsUpdate_ = true;
  }
  else if (name == "emissive_texture") {
    emissiveTexture_ = fileToTexture(value);

    needsUpdate_ = true;
  }
  else if (name == "vert_shader") {
    vertShaderFile_ = value;

    resetShader();
  }
  else if (name == "frag_shader") {
    fragShaderFile_ = value;

    resetShader();
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

bool
Model3DObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  auto *tcl = canvas_->app()->tcl();

  if      (op == "translate") {
    if (args.size() < 1)
      return false;

    CPoint3D p;
    if (! Util::stringToPoint3D(tcl, args[0], p))
      return false;

    object_->setTranslate(p.x, p.y, p.z);

    transformed_ = true;

    setNeedsUpdate();
  }
  else if (op == "scale") {
    if (args.size() < 1)
      return false;

    CPoint3D p;
    if (! Util::stringToPoint3D(tcl, args[0], p))
      return false;

    object_->setScale(p.x, p.y, p.z);

    transformed_ = true;

    setNeedsUpdate();
  }
  else if (op == "rotate") {
    if (args.size() < 2)
      return false;

    CPoint3D p;
    if (! Util::stringToPoint3D(tcl, args[0], p))
      return false;

    double a;
    if (! Util::stringToReal(args[1], a))
      return false;

    object_->setRotate(Util::degToRad(a), CVector3D(p.x, p.y, p.z));

    transformed_ = true;

    setNeedsUpdate();
  }
  else
    return Object3D::exec(op, args, res);

  return true;
}

bool
Model3DObj::
load(const QString &filename)
{
  // TODO: reuse object to add nore objects ?
  assert(! object_);

  //---

  filename_ = filename;

  QFileInfo fi(filename);

  auto suffix = fi.suffix().toLower();
  auto type   = CImportBase::suffixToType(suffix.toStdString());

  auto *im = CImportBase::createModel(type);

  if (! im)
    return canvas_->app()->errorMsg(QString("Invalid model type for '%1'").arg(filename_));

  for (const auto &dir : canvas_->modelDirs())
    im->addModelDir(dir.toStdString());

  CFile file(filename_.toStdString());

  if (! im->read(file)) {
    (void) canvas_->app()->errorMsg(QString("Failed to load file '%1'").arg(filename_));
    delete im;
    return false;
  }

  auto *scene = im->releaseScene();

  delete im;

  uint numTop = 0;

  for (auto *object : scene->getObjects()) {
    if (! object->parent()) {
      ++numTop;
      object_ = object;
    }
  }

  auto *scene1 = canvas_->scene();

  if (numTop > 1) {
    auto name = "object." + std::to_string(scene1->getObjects().size() + 1);

    auto *parentObj = CGeometry3DInst->createObject3D(scene1, name);

    scene1->addObject(parentObj);

    for (auto *object : scene->getObjects()) {
      scene1->addObject(object);

      if (! object->parent())
        parentObj->addChild(object);

      object->setInd(CGeometry3DInst->nextObjectId());
    }

    object_ = parentObj;
  }
  else {
    for (auto *object : scene->getObjects()) {
      scene1->addObject(object);

      object->setInd(CGeometry3DInst->nextObjectId());
    }
  }

  for (auto *material : scene->getMaterials()) {
    scene1->addMaterial(material);
  }

  for (auto *texture : scene->textures()) {
    scene1->addTexture(texture);
  }

  needsUpdate_ = true;

  return true;
}

void
Model3DObj::
tick()
{
  Object3D::tick();

  canvas_->update();
}

void
Model3DObj::
setModelMatrix(uint matrixFlags)
{
  if (transformed_) {
    modelMatrix_ = CMatrix3DH(object_->getHierTransform());
  }
  else {
    modelMatrix_ = CMatrix3DH::identity();

    auto c = sceneCenter_;

    if (matrixFlags & ModelMatrixFlags::TRANSLATE)
      modelMatrix_.translated(c.getX() + xPos(), c.getY() + yPos(), c.getZ() + zPos());

    if (matrixFlags & ModelMatrixFlags::SCALE)
      modelMatrix_.scaled(xscale(), yscale(), zscale());

    if (matrixFlags & ModelMatrixFlags::ROTATE) {
      modelMatrix_.rotated(xAngle(), CVector3D(1.0, 0.0, 0.0));
      modelMatrix_.rotated(yAngle(), CVector3D(0.0, 1.0, 0.0));
      modelMatrix_.rotated(zAngle(), CVector3D(0.0, 0.0, 1.0));
    }

    if (matrixFlags & ModelMatrixFlags::TRANSLATE)
      modelMatrix_.translated(-c.getX(), -c.getY(), -c.getZ());
  }
}

void
Model3DObj::
render()
{
  if (canvas_->isShowBBox() || isSelected()) {
    calcBBox();

    createBBoxObj();

    bboxObj_->render();
  }

  //---

  initShader();

  updateObjectData();

  //---

  // lighting
//auto *light = canvas_->currentLight();

//auto lightPos   = light->position();
//auto lightColor = light->color();

  //---

  setModelMatrix();

  auto t = 1.0*ticks_/100.0;

  if (t >= 1.0)
    dt_ = -dt_;

  drawObject(object_, t);
}

void
Model3DObj::
drawObject(CGeomObject3D *object, double t)
{
  auto *geomObject = dynamic_cast<GeomObject *>(object);

  auto *buffer = geomObject->buffer();

  bool textured = canvas_->isTextured();

  //---

  // setup model shader
  buffer->bind();

  s_program->bind();

  s_program->setUniformValue("ticks", float(t));

  canvas_->setProgramLights(s_program);

  s_program->setUniformValue("viewPos", CQGLUtil::toVector(canvas_->viewPos()));

  s_program->setUniformValue("ambientColor"    , CQGLUtil::toVector(canvas_->ambientColor()));
  s_program->setUniformValue("ambientStrength" , float(canvas_->ambientStrength()));

  s_program->setUniformValue("diffuseStrength" , float(canvas_->diffuseStrength()));

  s_program->setUniformValue("specularColor"   , CQGLUtil::toVector(canvas_->specularColor()));
  s_program->setUniformValue("specularStrength", float(canvas_->specularStrength()));

  s_program->setUniformValue("emissionColor"   , CQGLUtil::toVector(canvas_->emissiveColor()));
  s_program->setUniformValue("emissiveStrength", float(canvas_->emissiveStrength()));

  s_program->setUniformValue("shininess", float(canvas_->shininess())); // per face ?

  // pass projection matrix to shader (note that in this case it could change every frame)
  s_program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));

  // camera/view transformation
  s_program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  // model rotation
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  // render model
  for (const auto &faceData : geomObject->faceDatas()) {
    // diffuse (texture 0)
    auto *diffuseTexture = faceData.diffuseTexture;

    if (! diffuseTexture)
      diffuseTexture = diffuseTexture_;

    bool useDiffuseTexture = !!diffuseTexture;

    s_program->setUniformValue("diffuseTexture.enabled", textured && useDiffuseTexture);

    if (useDiffuseTexture) {
      glActiveTexture(GL_TEXTURE0);
      diffuseTexture->bind();

      s_program->setUniformValue("diffuseTexture.texture", 0);
    }

    //---

    // specular (texture 1)
    auto *specularTexture = faceData.specularTexture;

    if (! specularTexture)
      specularTexture = specularTexture_;

    bool useSpecularTexture = !!specularTexture;

    s_program->setUniformValue("specularTexture.enabled", textured && useSpecularTexture);

    if (useSpecularTexture) {
      glActiveTexture(GL_TEXTURE1);
      specularTexture->bind();

      s_program->setUniformValue("specularTexture.texture", 1);
    }

    //---

    // normal (texture 2)
    auto *normalTexture = faceData.normalTexture;

    if (! normalTexture)
      normalTexture = normalTexture_;

    bool useNormalTexture = !!normalTexture;

    s_program->setUniformValue("normalTexture.enabled", textured && useNormalTexture);

    if (useNormalTexture) {
      glActiveTexture(GL_TEXTURE2);
      normalTexture->bind();

      s_program->setUniformValue("normalTexture.texture", 2);
    }

    //---

    // emissive (texture 3)
    auto *emissiveTexture = faceData.emissiveTexture;

    if (! emissiveTexture)
      emissiveTexture = emissiveTexture_;

    bool useEmissiveTexture = !!emissiveTexture;

    s_program->setUniformValue("emissiveTexture.enabled", textured && useEmissiveTexture);

    if (useEmissiveTexture) {
      glActiveTexture(GL_TEXTURE2);
      emissiveTexture->bind();

      s_program->setUniformValue("emissiveTexture.texture", 2);
    }

    //---

    if (canvas_->isSolid() || canvas_->isTextured()) {
      s_program->setUniformValue("isWireframe", 0);

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      glDrawArrays(GL_TRIANGLE_FAN, faceData.pos, faceData.len);
    }

    if (canvas_->isPolygonLine() || canvas_->isWireframe()) {
      s_program->setUniformValue("isWireframe", 1);

      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      glDrawArrays(GL_TRIANGLE_FAN, faceData.pos, faceData.len);
    //glDrawArrays(GL_TRIANGLES, faceData.pos, faceData.len);
    }
  }

  buffer->unbind();

  //---

  for (auto *child : object->children())
    drawObject(child, t);
}

void
Model3DObj::
updateObjectData()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  // set up vertex data (and buffer(s)) and configure vertex attributes
  CVector3D sceneSize(1, 1, 1);

  if (object_) {
    object_->getModelBBox(bbox_);

    sceneSize    = bbox_.getSize();
    sceneCenter_ = bbox_.getCenter();

    //std::cerr << "Scene Center : " << sceneCenter_.getX() << " " <<
    //             sceneCenter_.getY() << " " << sceneCenter_.getZ() << "\n";

    updateObject(object_);
  }

  //---

  if (autoScale_) {
    auto max3 = [](double x, double y, double z) {
      return std::max(std::max(x, y), z);
    };

    auto sceneScale = float(1.0/max3(sceneSize.getX(), sceneSize.getY(), sceneSize.getZ()));
    //std::cerr << "Scene Scale : " << sceneScale << "\n";

    xscale_ = sceneScale;
    yscale_ = sceneScale;
    zscale_ = sceneScale;
  }
}

void
Model3DObj::
updateObject(CGeomObject3D *object)
{
  auto *geomObject = dynamic_cast<GeomObject *>(object);

  auto *buffer = geomObject->initBuffer(canvas_);

  //---

  auto *diffuseTexture  = object->getDiffuseTexture();
  auto *specularTexture = object->getSpecularTexture();
  auto *normalTexture   = object->getNormalTexture();
  auto *emissiveTexture = object->getEmissiveTexture();

  //---

  const auto &faces = object->getFaces();

  int pos = 0;

  for (const auto *face : faces) {
    FaceData faceData;

    //---

    const auto &color = face->getColor();

    //---

    auto *diffuseTexture1 = face->getDiffuseTexture();

    if (! diffuseTexture1)
      diffuseTexture1 = diffuseTexture;

    auto *specularTexture1 = face->getSpecularTexture();

    if (! specularTexture1)
      specularTexture1 = specularTexture;

    auto *normalTexture1 = face->getNormalTexture();

    if (! normalTexture1)
      normalTexture1 = normalTexture;

    auto *emissiveTexture1 = face->getEmissiveTexture();

    if (! emissiveTexture1)
      emissiveTexture1 = emissiveTexture;

    //---

    if (diffuseTexture1) {
      auto pt = glTextures_.find(diffuseTexture1->id());

      if (pt == glTextures_.end()) {
        const auto &image = diffuseTexture1->image()->image();

        auto *glTexture = new CGLTexture(image);

        pt = glTextures_.insert(pt, GLTextures::value_type(diffuseTexture1->id(), glTexture));
      }

      faceData.diffuseTexture = (*pt).second;
    }

    if (specularTexture1) {
      auto pt = glTextures_.find(specularTexture1->id());

      if (pt == glTextures_.end()) {
        const auto &image = specularTexture1->image()->image();

        auto *glTexture = new CGLTexture(image);

        pt = glTextures_.insert(pt, GLTextures::value_type(specularTexture1->id(), glTexture));
      }

      faceData.specularTexture = (*pt).second;
    }

    if (normalTexture1) {
      auto pt = glTextures_.find(normalTexture1->id());

      if (pt == glTextures_.end()) {
        const auto &image = normalTexture1->image()->image();

        auto *glTexture = new CGLTexture(image);

        pt = glTextures_.insert(pt, GLTextures::value_type(normalTexture1->id(), glTexture));
      }

      faceData.normalTexture = (*pt).second;
    }

    if (emissiveTexture1) {
      auto pt = glTextures_.find(emissiveTexture1->id());

      if (pt == glTextures_.end()) {
        const auto &image = emissiveTexture1->image()->image();

        auto *glTexture = new CGLTexture(image);

        pt = glTextures_.insert(pt, GLTextures::value_type(emissiveTexture1->id(), glTexture));
      }

      faceData.emissiveTexture = (*pt).second;
    }

    //---

  //const auto &ambient   = face->getMaterial().getAmbient  ();
  //const auto &diffuse   = face->getMaterial().getDiffuse  ();
  //const auto &specular  = face->getMaterial().getSpecular ();
  //double      shininess = face->getMaterial().getShininess();

    CVector3D normal;

    if (face->getNormalSet())
      normal = face->getNormal();
    else
      face->calcModelNormal(normal);

    const auto &vertices = face->getVertices();

    faceData.pos = pos;
    faceData.len = int(vertices.size());

    for (const auto &v : vertices) {
      auto &vertex = object->getVertex(v);

      const auto &model = vertex.getModel();

      auto vnormal = vertex.getNormal(normal);

      if (! flipYZ_) {
        buffer->addPoint(float(model.x), float(model.y), float(model.z));
        buffer->addNormal(float(vnormal.getX()), float(vnormal.getY()), float(vnormal.getZ()));
      }
      else {
        buffer->addPoint(float(model.x), float(model.z), float(model.y));
        buffer->addNormal(float(vnormal.getX()), float(vnormal.getZ()), float(vnormal.getY()));
      }

      auto vcolor = vertex.getColor(color);

      buffer->addColor(vcolor.getRedF(), vcolor.getGreenF(), vcolor.getBlueF());

      auto *diffuseTexture  = faceData.diffuseTexture;
      auto *specularTexture = faceData.specularTexture;
      auto *normalTexture   = faceData.normalTexture;
      auto *emissiveTexture = faceData.emissiveTexture;

      if (! diffuseTexture ) diffuseTexture  = diffuseTexture_;
      if (! specularTexture) specularTexture = specularTexture_;
      if (! normalTexture  ) normalTexture   = normalTexture_;
      if (! emissiveTexture) emissiveTexture = emissiveTexture_;

      if (diffuseTexture || specularTexture || normalTexture || emissiveTexture) {
        const auto &tpoint = vertex.getTextureMap();

        buffer->addTexturePoint(float(tpoint.x), float(tpoint.y));
      }
      else
        buffer->addTexturePoint(0.0f, 0.0f);
    }

    pos += faceData.len;

    geomObject->addFaceData(faceData);
  }

  buffer->load();

  //---

  for (auto *child : object->children())
    updateObject(child);
}

void
Model3DObj::
calcTangents()
{
  if (! object_)
    return;

  calcTangents1(object_);
}

void
Model3DObj::
calcTangents1(CGeomObject3D *object)
{
  std::vector<CVector3D> tangents;

  auto nv = object->getNumVertices();

  tangents.resize(nv);

  const auto &faces = object->getFaces();

  for (const auto *face : faces) {
    const auto &vertices = face->getVertices();
    if (vertices.size() < 3) continue;

    const auto &v0 = object->getVertex(vertices[0]);
    const auto &v1 = object->getVertex(vertices[1]);
    const auto &v2 = object->getVertex(vertices[2]);

    auto uv0 = v0.getTextureMap();
    auto uv1 = v1.getTextureMap();
    auto uv2 = v2.getTextureMap();

    auto edge1 = v1.getViewed() - v0.getViewed();
    auto edge2 = v2.getViewed() - v0.getViewed();

    auto dUV1 = uv1 - uv0;
    auto dUV2 = uv2 - uv0;

    double f = 1.0/(dUV1.x*dUV2.y - dUV2.x*dUV1.y);

    auto tx = f*(dUV2.y*edge1.x - dUV1.y*edge2.x);
    auto ty = f*(dUV2.y*edge1.y - dUV1.y*edge2.y);
    auto tz = f*(dUV2.y*edge1.z - dUV1.y*edge2.z);

    auto tan = CVector3D(tx, ty, tz);
    tan.normalize();

    tangents[vertices[0]] += tan;
    tangents[vertices[1]] += tan;
    tangents[vertices[2]] += tan;
  }

  for (uint i = 0; i < tangents.size(); ++i)
    tangents[i].normalize();

  //---

  for (auto *child : object->children())
    calcTangents1(child);
}

}
