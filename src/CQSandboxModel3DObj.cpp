#include <CQSandboxModel3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxBBox3DObj.h>
#include <CQSandboxApp.h>
#include <CQSandboxGeomObject.h>
#include <CQSandboxTexture.h>
#include <CQSandboxUtil.h>

#include <CQGLTexture.h>
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
  auto *app = canvas->app();
  auto *tcl = app->tcl();

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
 Object3D(canvas, Type::MODEL)
{
  auto *app = canvas_->app();

  vertShaderFile_ = app->buildDir() + "/shaders/model.vs";
  fragShaderFile_ = app->buildDir() + "/shaders/model.fs";
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
  if      (name == "ref_object") {
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
  else if (name == "transformed_model_bbox") {
    CBBox3D bbox;
    object_->getTransformedModelBBox(bbox);

    value = Util::bbox3DToString(bbox);
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
      return static_cast<CQGLTexture *>(nullptr);

    CImageFileSrc src(imageFile);

    auto image = CImageMgrInst->createImage(src);

    if (flipY)
      image = image->flippedH();

    auto *texture = dynamic_cast<Texture *>(CGeometry3DInst->createTexture(image));

    return texture->glTexture(canvas_);
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
  else if (name == "anim.name") {
    auto *geomObject = dynamic_cast<GeomObject *>(object_);

    auto *geomObject1 = geomObject;

    if (geomObject->refObject()) {
      geomObject1 = dynamic_cast<GeomObject *>(geomObject->refObject());
      assert(geomObject1);
    }

    geomObject1->setAnimName(value.toStdString());
  }
  else if (name == "anim.repeat") {
    auto *geomObject = dynamic_cast<GeomObject *>(object_);

    auto *geomObject1 = geomObject;

    if (geomObject->refObject()) {
      geomObject1 = dynamic_cast<GeomObject *>(geomObject->refObject());
      assert(geomObject1);
    }

    geomObject1->setAnimRepeat(Util::stringToBool(value));
  }
  else if (name == "anim.step") {
    auto *geomObject = dynamic_cast<GeomObject *>(object_);

    auto *geomObject1 = geomObject;

    if (geomObject->refObject()) {
      geomObject1 = dynamic_cast<GeomObject *>(geomObject->refObject());
      assert(geomObject1);
    }

    geomObject1->setAnimTimeStep(Util::stringToReal(value));
  }
  else if (name == "child.visible") {
    if (args.size() < 1)
      return false;

    auto *child = object_->getChildOfName(value.toStdString());
    if (! child) return false;

    child->setVisible(Util::stringToBool(args[0]));

  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

bool
Model3DObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

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

  auto *app = canvas_->app();

  filename_ = filename;

  QFileInfo fi(filename);

  auto suffix = fi.suffix().toLower();
  auto type   = CImportBase::suffixToType(suffix.toStdString());

  auto *im = CImportBase::createModel(type);

  if (! im)
    return app->errorMsg(QString("Invalid model type for '%1'").arg(filename_));

  for (const auto &dir : canvas_->modelDirs())
    im->addModelDir(dir.toStdString());

  CFile file(filename_.toStdString());

  if (! im->read(file)) {
    (void) app->errorMsg(QString("Failed to load file '%1'").arg(filename_));
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
setModelMatrix(uint /*matrixFlags*/)
{
#if 0
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
#endif
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

  //setModelMatrix();

  auto t = 1.0*ticks_/100.0;

  if      (dt_ > 0 && t >= 1.0)
    dt_ = -dt_;
  else if (dt_ < 0 && t <= 0.0)
    dt_ = -dt_;

  drawObject(object_, elapsed_);
}

void
Model3DObj::
initDraw(double t)
{
  //s_program->bind();
  canvas_->bindProgram(s_program);

  s_program->setUniformValue("ticks", float(t));

  // camera projection
  s_program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));

  // camera/view transformation
  s_program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  // view pos
  s_program->setUniformValue("viewPos", CQGLUtil::toVector(canvas_->viewPos()));

  //---

  // light data
  s_program->setUniformValue("ambientColor"    , CQGLUtil::toVector(canvas_->ambientColor()));
  s_program->setUniformValue("ambientStrength" , float(canvas_->ambientStrength()));

  s_program->setUniformValue("diffuseStrength" , float(canvas_->diffuseStrength()));

  s_program->setUniformValue("specularColor"   , CQGLUtil::toVector(canvas_->specularColor()));
  s_program->setUniformValue("specularStrength", float(canvas_->specularStrength()));

  s_program->setUniformValue("emissionColor"   , CQGLUtil::toVector(canvas_->emissiveColor()));
  s_program->setUniformValue("emissiveStrength", float(canvas_->emissiveStrength()));

  s_program->setUniformValue("shininess", float(canvas_->shininess())); // per face ?

  canvas_->setProgramLights(s_program);
}

void
Model3DObj::
drawObject(CGeomObject3D *object, double t)
{
  initDraw(t);

  //---

  auto *geomObject = dynamic_cast<GeomObject *>(object);

  auto *geomObject1 = geomObject;

  if (geomObject->refObject()) {
    geomObject1 = dynamic_cast<GeomObject *>(geomObject->refObject());
    assert(geomObject1);

    if (! geomObject1->buffer())
      updateObject(geomObject1);
  }

  //---

  auto *animObject = geomObject1->getAnimObject();

  bool isAnim = false;

  if (canvas_->isAnimEnabled())
    isAnim = (animObject && animObject->animName() != "");

  // mesh matrix
  CMatrix3DH meshMatrix;
  bool       hasMeshMatrix { false };

  if (isAnim)
    hasMeshMatrix = canvas_->addObjectMeshData(geomObject1, meshMatrix);

  if (! hasMeshMatrix)
    meshMatrix = CMatrix3DH(object->getMeshGlobalTransform());

  s_program->setUniformValue("meshMatrix", CQGLUtil::toQMatrix(meshMatrix));

  //---

  // model matrix
  auto modelMatrix = CMatrix3DH(object->getHierTransform());

  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix));

  //---

  // anim
  s_program->setUniformValue("useBonePoints", isAnim);

  if (isAnim) {
    canvas_->updateNodeMatrices(geomObject1);

    s_program->setUniformValueArray("globalBoneTransform",
      canvas_->nodeQMatrices(), canvas_->numNodeQMatrices());
  }

  //---

  // setup data buffer
  auto *buffer = geomObject1->buffer();

  //buffer->bind();
  canvas_->bindBuffer(buffer);

  //---

  bool textured = canvas_->isTextured();

  //---

  // render model
  for (const auto &faceData : geomObject1->faceDatas()) {
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

    // normal (texture 1)
    auto *normalTexture = faceData.normalTexture;

    if (! normalTexture)
      normalTexture = normalTexture_;

    bool useNormalTexture = !!normalTexture;

    s_program->setUniformValue("normalTexture.enabled", textured && useNormalTexture);

    if (useNormalTexture) {
      glActiveTexture(GL_TEXTURE1);
      normalTexture->bind();

      s_program->setUniformValue("normalTexture.texture", 1);
    }

    //---

    // specular (texture 2)
    auto *specularTexture = faceData.specularTexture;

    if (! specularTexture)
      specularTexture = specularTexture_;

    bool useSpecularTexture = !!specularTexture;

    s_program->setUniformValue("specularTexture.enabled", textured && useSpecularTexture);

    if (useSpecularTexture) {
      glActiveTexture(GL_TEXTURE2);
      specularTexture->bind();

      s_program->setUniformValue("specularTexture.texture", 2);
    }

    //---

    // emissive (texture 3)
    auto *emissiveTexture = faceData.emissiveTexture;

    if (! emissiveTexture)
      emissiveTexture = emissiveTexture_;

    bool useEmissiveTexture = !!emissiveTexture;

    s_program->setUniformValue("emissiveTexture.enabled", textured && useEmissiveTexture);

    if (useEmissiveTexture) {
      glActiveTexture(GL_TEXTURE3);
      emissiveTexture->bind();

      s_program->setUniformValue("emissiveTexture.texture", 3);
    }

    //---

#if 0
    s_program->setUniformValue("emissionColor", CQGLUtil::toVector(faceData.emission));
    s_program->setUniformValue("shininess"    , float(faceData.shininess));
    s_program->setUniformValue("transparency" , float(1.0 - transparency));
#endif

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

  //buffer->unbind();

  //---

  for (auto *child : geomObject->children()) {
    if (! child->getVisible())
      continue;

    drawObject(child, t);
  }
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
    //object_->getModelBBox(bbox_);
    object_->getTransformedModelBBox(bbox_);

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

  if (geomObject->refObject())
    return;

  //---

  int    boneNodeIds[4];
  double boneWeights[4];

  //---

  auto modelMatrix = CMatrix3DH(object->getHierTransform());
  auto meshMatrix  = CMatrix3DH(object->getMeshGlobalTransform());

  //---

  auto *animObject = object->getAnimObject();

  auto animName = (animObject ? animObject->animName() : "");

  bool isAnim = false;

  if (canvas_->isAnimEnabled())
    isAnim = (animObject && animName != "");

  //double animTime { 0.0 };

  if (isAnim) {
    //animTime = animObject->animTime();

    auto meshNodeId = object->getMeshNode();

    CGeomNodeData *node = nullptr;

    if (meshNodeId >= 0)
      node = const_cast<CGeomNodeData *>(&animObject->getNode(meshNodeId));

    auto isJointed = (node && object->isJointed());

    if (node && ! isJointed) {
      auto &objectMeshData = canvas_->getObjectMeshData(object);

      objectMeshData.nt = object->animTimeFrames();

      (void) animObject->getAnimationTranslationRange(animName,
               objectMeshData.tmin, objectMeshData.tmax);

      if (objectMeshData.nt > 1)
        objectMeshData.dt = (objectMeshData.tmax - objectMeshData.tmin)/(objectMeshData.nt - 1);
      else
        objectMeshData.dt = (objectMeshData.tmax - objectMeshData.tmin);

      for (int i = 0; i < objectMeshData.nt; ++i) {
        auto animTime1 = objectMeshData.tmin + i*objectMeshData.dt;

        auto meshMatrix1 =
          CMatrix3DH(object->getNodeAnimHierTransform(*node, animName, animTime1));

        objectMeshData.frameMatrix[i] = meshMatrix1;
      }
    }
  }

  //---

  auto *buffer = geomObject->initBuffer(canvas_);

  //---

  auto *objectMaterial = object->getMaterialP();

  auto *diffuseTexture  = object->getDiffuseTexture();
  auto *specularTexture = object->getSpecularTexture();
  auto *normalTexture   = object->getNormalTexture();
  auto *emissiveTexture = object->getEmissiveTexture();

  //---

  auto makeTexture = [&](const CImagePtr &image) {
    auto *texture = new CQGLTexture(image);

    texture->setFunctions(const_cast<Canvas3D *>(canvas_));

    return texture;
  };

  auto initGLTexture = [&](Texture *texture) {
    const auto &image = texture->image()->image();

    auto flippedImage = image->dup();

    flippedImage->flipH();

    auto *t1 = makeTexture(image);
    auto *t2 = makeTexture(flippedImage);

    t1->setName(texture->name());
    t2->setName(texture->name() + ".flip");

    texture->setGlTextures(canvas_, t1, t2);
  };

  auto getGLTexture = [&](CGeomTexture *texture, bool /*add*/) {
    auto *texture1 = dynamic_cast<Texture *>(texture);
    assert(texture1);

    if (! texture1->glTexture(canvas_)) {
      //if (! add) return nullptr;

      initGLTexture(texture1);
    }

    return texture1->glTexture(canvas_);
  };

  //---

  CBBox3D bbox1;

  int pos = 0;

  const auto &faces = geomObject->getFaces();

  for (const auto *face : faces) {
    FaceData faceData;

    faceData.face = const_cast<CGeomFace3D *>(face);

    //---

    auto *faceMaterial = faceData.face->getMaterialP();

    if (! faceMaterial && objectMaterial)
      faceMaterial = objectMaterial;

    //---

    auto color = face->color().value_or(CRGBA(1, 1, 1));

    if (faceMaterial && faceMaterial->diffuse())
      color = faceMaterial->diffuse().value();

    faceData.color = Util::RGBAToQColor(color);

    //---

    // set face textures
    auto *diffuseTexture1  = face->getDiffuseTexture();
    auto *normalTexture1   = face->getNormalTexture();
    auto *specularTexture1 = face->getSpecularTexture();
    auto *emissiveTexture1 = face->getEmissiveTexture();

    if (! diffuseTexture1 ) diffuseTexture1  = diffuseTexture;
    if (! normalTexture1  ) normalTexture1   = normalTexture;
    if (! specularTexture1) specularTexture1 = specularTexture;
    if (! emissiveTexture1) emissiveTexture1 = emissiveTexture;

    if (faceMaterial) {
      if (faceMaterial->diffuseTexture ()) diffuseTexture1  = faceMaterial->diffuseTexture ();
      if (faceMaterial->normalTexture  ()) normalTexture1   = faceMaterial->normalTexture  ();
      if (faceMaterial->specularTexture()) specularTexture1 = faceMaterial->specularTexture();
      if (faceMaterial->emissiveTexture()) emissiveTexture1 = faceMaterial->emissiveTexture();
    }

    if (diffuseTexture1)
      faceData.diffuseTexture = getGLTexture(diffuseTexture1, /*add*/true);

    if (normalTexture1)
      faceData.normalTexture = getGLTexture(normalTexture1, /*add*/true);

    if (specularTexture1)
      faceData.specularTexture = getGLTexture(specularTexture1, /*add*/true);

    if (emissiveTexture1)
      faceData.emissiveTexture = getGLTexture(emissiveTexture1, /*add*/true);

    //---

    const auto &vertices = face->getVertices();

    //---

    // get face normal
    CVector3D normal;

    if (face->getNormalSet())
      normal = face->getNormal();
    else {
      for (const auto &v : vertices) {
        auto &vertex = object->getVertex(v);

        vertex.setViewed(vertex.getModel());
      }

      face->calcModelNormal(normal);
    }

    //---

    faceData.pos = pos;
    faceData.len = int(vertices.size());

    int iv = 0;

    for (const auto &v : vertices) {
      const auto &vertex = geomObject->getVertex(v);
      const auto &model  = vertex.getModel();

      auto model1 = meshMatrix *model;
      auto model2 = modelMatrix*model1;

      //---

      // update color, normal for custom vertex value

      auto normal1 = normal;
      auto color1  = color;

      if      (face->hasVertexNormals())
        normal1 = face->getVertexNormal(iv);
      else if (vertex.hasNormal())
        normal1 = vertex.getNormal();

      if (vertex.hasColor())
        color1 = vertex.getColor();

      //---

      if (faceData.normalTexture) {
        CPoint2D tpoint;

        if (vertex.hasTextureMap())
          tpoint = vertex.getTextureMap();
        else
          tpoint = face->getTexturePoint(vertex, iv);

        int tw = faceData.normalTexture->getWidth ();
        int th = faceData.normalTexture->getHeight();

        auto tx = CMathUtil::clamp(tpoint.x, 0.0, 1.0);
        auto ty = CMathUtil::clamp(tpoint.y, 0.0, 1.0);

        // get normal value from texture
        auto rgba = faceData.normalTexture->getImage().pixel(tx*(tw - 1), ty*(th - 1));
        auto tnormal = CVector3D(qRed(rgba)/255.0, qGreen(rgba)/255.0, qBlue(rgba)/255.0);

        // this normal is in tangent space
        normal1 = (tnormal*2.0 - CVector3D(1.0, 1.0, 1.0)).normalized();
      }

      //---

      buffer->addInd(vertex.getInd());

      buffer->addPoint(float(model.x), float(model.y), float(model.z));

      buffer->addNormal(float(normal1.getX()), float(normal1.getY()), float(normal1.getZ()));

      buffer->addColor(color1);

      //---

      if (isAnim) {
        if (vertex.hasJointData()) {
          const auto &jointData = vertex.getJointData();

          for (int i = 0; i < 4; ++i) {
            boneNodeIds[i] = jointData.nodeDatas[i].node;
            boneWeights[i] = jointData.nodeDatas[i].weight;
          }

          buffer->addBoneIds    (boneNodeIds[0], boneNodeIds[1], boneNodeIds[2], boneNodeIds[3]);
          buffer->addBoneWeights(boneWeights[0], boneWeights[1], boneWeights[2], boneWeights[3]);
        }
      }

      //---

      if (faceData.diffuseTexture) {
        const auto &tpoint = face->getTexturePoint(vertex, iv);

        buffer->addTexturePoint(float(tpoint.x), float(tpoint.y));
      }
      else
        buffer->addTexturePoint(0.0f, 0.0f);

      //---

      ++iv;

      bbox1 += model2;
    }

    pos += faceData.len;

    geomObject->addFaceData(faceData);
  }

  //---

  buffer->load();

  //---

  for (auto *child : geomObject->children()) {
    if (! child->getVisible())
      continue;

    updateObject(child);
  }
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
calcTangents1(CGeomObject3D *geomObject)
{
  std::vector<CVector3D> tangents;

  auto nv = geomObject->getNumVertices();

  tangents.resize(nv);

  const auto &faces = geomObject->getFaces();

  for (const auto *face : faces) {
    const auto &vertices = face->getVertices();
    if (vertices.size() < 3) continue;

    const auto &v0 = geomObject->getVertex(vertices[0]);
    const auto &v1 = geomObject->getVertex(vertices[1]);
    const auto &v2 = geomObject->getVertex(vertices[2]);

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

  for (auto *child : geomObject->children()) {
    if (! child->getVisible())
      continue;

    calcTangents1(child);
  }
}

}
