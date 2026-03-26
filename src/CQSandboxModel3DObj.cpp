#include <CQSandboxModel3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxBBox3DObj.h>
#include <CQSandboxApp.h>

#include <CQGLBuffer.h>
#include <CQGLUtil.h>
#include <CGLTexture.h>
#include <CImportBase.h>
#include <CGeomScene3D.h>

#include <CQTclUtil.h>
#include <CImageLib.h>
#include <CFile.h>

#include <QFileInfo>

namespace CQSandbox {

ShaderProgram* Model3DObj::s_program;

bool
Model3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Model3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  if (args.size() >= 1) {
    auto filename = args[0];

    obj->load(filename);
  }

  tcl->setResult(name);

  return true;
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

QVariant
Model3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
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
load(const QString &filename)
{
  filename_ = filename;

  QFileInfo fi(filename);

  auto suffix = fi.suffix().toLower();
  auto type   = CImportBase::suffixToType(suffix.toStdString());

  import_ = CImportBase::createModel(type);

  if (! import_) {
    canvas_->app()->errorMsg(QString("Invalid model type for '%1'").arg(filename_));
    return false;
  }

  CFile file(filename_.toStdString());

  if (! import_->read(file)) {
    canvas_->app()->errorMsg(QString("Failed to load file '%1'").arg(filename_));
    return false;
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
  modelMatrix_ = CGLMatrix3D::identity();

  if (matrixFlags & ModelMatrixFlags::TRANSLATE)
    modelMatrix_.translated(float(sceneCenter_.getX()),
                            float(sceneCenter_.getY()),
                            float(sceneCenter_.getZ()));

  if (matrixFlags & ModelMatrixFlags::SCALE)
    modelMatrix_.scaled(xscale(), yscale(), zscale());

  if (matrixFlags & ModelMatrixFlags::ROTATE) {
    modelMatrix_.rotated(xAngle(), CGLVector3D(1.0, 0.0, 0.0));
    modelMatrix_.rotated(yAngle(), CGLVector3D(0.0, 1.0, 0.0));
    modelMatrix_.rotated(zAngle(), CGLVector3D(0.0, 0.0, 1.0));
  }

  if (matrixFlags & ModelMatrixFlags::TRANSLATE)
    modelMatrix_.translated(float(-sceneCenter_.getX()),
                            float(-sceneCenter_.getY()),
                            float(-sceneCenter_.getZ()));
}

void
Model3DObj::
render()
{
  if (canvas_->isBBox() || isSelected()) {
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

  // setup model shader
  for (auto &po : objectDatas_) {
    auto *objectData = po.second;

    objectData->buffer->bind();

    s_program->bind();

    s_program->setUniformValue("ticks", float(t));

    canvas_->setProgramLights(s_program);

    s_program->setUniformValue("viewPos", CQGLUtil::toVector(canvas_->viewPos()));

    s_program->setUniformValue("ambientStrength" , float(canvas_->ambient()));
    s_program->setUniformValue("diffuseStrength" , float(canvas_->diffuse()));
    s_program->setUniformValue("specularStrength", float(canvas_->specular()));
    s_program->setUniformValue("shininess"       , float(canvas_->shininess()));

    // pass projection matrix to shader (note that in this case it could change every frame)
    s_program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));

    // camera/view transformation
    s_program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

    // model rotation
    s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

    // render model
    for (const auto &faceData : objectData->faceDatas) {
      // diffuse (texture 0)
      auto *diffuseTexture = faceData.diffuseTexture;

      if (! diffuseTexture)
        diffuseTexture = diffuseTexture_;

      bool useDiffuseTexture = !!diffuseTexture;

      s_program->setUniformValue("diffuseTexture.enabled", useDiffuseTexture);

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

      s_program->setUniformValue("specularTexture.enabled", useSpecularTexture);

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

      s_program->setUniformValue("normalTexture.enabled", useNormalTexture);

      if (useNormalTexture) {
        glActiveTexture(GL_TEXTURE2);
        normalTexture->bind();

        s_program->setUniformValue("normalTexture.texture", 2);
      }

      //---

      s_program->setUniformValue("isWireframe", canvas_->isWireframe() ? 1 : 0);

      if (! canvas_->isWireframe()) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glDrawArrays(GL_TRIANGLE_FAN, faceData.pos, faceData.len);
      }

      if (canvas_->isPolygonLine() || canvas_->isWireframe()) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glDrawArrays(GL_TRIANGLE_FAN, faceData.pos, faceData.len);
        //glDrawArrays(GL_TRIANGLES, faceData.pos, faceData.len);
      }
    }

    objectData->buffer->unbind();
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

  if (import_) {
    auto &scene = import_->getScene();

    scene.getBBox(bbox_);

    sceneSize    = bbox_.getSize();
    sceneCenter_ = bbox_.getCenter();
    //std::cerr << "Scene Center : " << sceneCenter_.getX() << " " <<
    //             sceneCenter_.getY() << " " << sceneCenter_.getZ() << "\n";

    for (auto *object : scene.getObjects()) {
      ObjectData *objectData { nullptr };

      auto pd = objectDatas_.find(object);

      if (pd == objectDatas_.end())
        pd = objectDatas_.insert(pd, ObjectDatas::value_type(object, new ObjectData));

      objectData = (*pd).second;

      if (! objectData->buffer)
        objectData->buffer = s_program->createBuffer();

      //---

      auto *buffer = objectData->buffer;

      buffer->clearAll();

      objectData->faceDatas.clear();

      //---

      auto *diffuseTexture  = object->getDiffuseTexture();
      auto *specularTexture = object->getSpecularTexture();
      auto *normalTexture   = object->getNormalTexture();

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

          if (! diffuseTexture ) diffuseTexture  = diffuseTexture_;
          if (! specularTexture) specularTexture = specularTexture_;
          if (! normalTexture  ) normalTexture   = normalTexture_;

          if (diffuseTexture || specularTexture || normalTexture) {
            const auto &tpoint = vertex.getTextureMap();

            buffer->addTexturePoint(float(tpoint.x), float(tpoint.y));
          }
          else
            buffer->addTexturePoint(0.0f, 0.0f);
        }

        pos += faceData.len;

        objectData->faceDatas.push_back(faceData);
      }

      objectData->buffer->load();
    }
  }

  auto max3 = [](double x, double y, double z) {
    return std::max(std::max(x, y), z);
  };

  auto sceneScale = float(1.0/max3(sceneSize.getX(), sceneSize.getY(), sceneSize.getZ()));
  //std::cerr << "Scene Scale : " << sceneScale << "\n";

  xscale_ = sceneScale;
  yscale_ = sceneScale;
  zscale_ = sceneScale;
}

void
Model3DObj::
calcTangents()
{
  if (! import_)
    return;

  auto &scene = import_->getScene();

  for (auto *object : scene.getObjects()) {
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
  }
}

}
