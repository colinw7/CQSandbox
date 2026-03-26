#include <CQSandboxSkybox3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLCubemap.h>
#include <CQGLTexture.h>
#include <CQGLBuffer.h>
#include <CQGLUtil.h>

#include <CGeomScene3D.h>
#include <CImportBase.h>
#include <CQTclUtil.h>
#include <CFile.h>

#include <QFileInfo>

namespace CQSandbox {

ShaderProgram *Skybox3DObj::s_program = nullptr;

bool
Skybox3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Skybox3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Skybox3DObj::
Skybox3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
  (void) load();
}

void
Skybox3DObj::
init()
{
  Object3D::init();
}

void
Skybox3DObj::
initShader()
{
  if (! s_program) {
#if 0
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "layout (location = 0) in vec3 pos;\n"
      "out vec3 fragPos;\n"
      "uniform mat4 projection;\n"
      "uniform mat4 view;\n"
      "uniform mat4 model;\n"
      "void main() {\n"
      "  fragPos = pos;\n"
      "  //gl_Position = projection * view * model * vec4(pos, 1.0);\n"
      "  gl_Position = (projection * view * model * vec4(pos, 1.0)).xyww;\n"
      "  //gl_Position = view * model * vec4(pos, 1.0);\n"
      "}\n";
    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "uniform samplerCube textureId;\n"
      "//uniform sampler2D textureId;\n"
      "uniform bool isWireframe;\n"
      "in vec3 fragPos;\n"
      "out vec4 outCol;\n"
      "void main() {\n"
      "  if (isWireframe) {\n"
      "    outCol = vec4(fragPos, 1.0);\n"
      "  } else {\n"
      "    outCol = texture(textureId, fragPos);\n"
      "    //outCol = texture(textureId, 0.5*(fragPos.xy + 1.0));\n"
      "    //outCol = vec4(fragPos, 1.0);\n"
      "  }\n"
      "}\n";
#endif

    s_program = new ShaderProgram(this);

#if 0
    s_program->addVertexCode  (vertexShaderSource);
    s_program->addFragmentCode(fragmentShaderSource);
#else
    s_program->addVertexFile  (canvas_->buildDir() + "/shaders/skybox.vs");
    s_program->addFragmentFile(canvas_->buildDir() + "/shaders/skybox.fs");
#endif

    s_program->link();
  }
}

bool
Skybox3DObj::
load()
{
  auto filename = canvas_->buildDir() + "/models/ply/cube.ply";

  QFileInfo fi(filename);

  auto suffix = fi.suffix().toLower();
  auto type   = CImportBase::suffixToType(suffix.toStdString());

  import_ = CImportBase::createModel(type);

  if (! import_) {
    canvas_->app()->errorMsg(QString("Invalid model type for '%1'").arg(filename));
    return false;
  }

  CFile file(filename.toStdString());

  if (! import_->read(file)) {
    canvas_->app()->errorMsg(QString("Failed to load file '%1'").arg(filename));
    return false;
  }

  needsUpdate_ = true;

  return true;
}

QVariant
Skybox3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Skybox3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  if      (name == "images") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.length() != 6)
      return false;

    images_.resize(6);

    for (int i = 0; i < 6; ++i) {
      images_[i] = QImage(strs[i]);

      if (images_[i].isNull())
        std::cerr << "Invalid image '" << strs[i].toStdString() << "'\n";
    }

    delete cubemap_;

    cubemap_ = new CQGLCubemap;
    cubemap_->setImages(images_);

    canvas_->glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    delete texture_;

    texture_ = new CQGLTexture;
    texture_->setImage(images_[0]);

    needsUpdate_ = true;
  }
  else if (name == "cubemap") {
    useCubemap_ = Util::stringToBool(value);
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Skybox3DObj::
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

      const auto &faces = object->getFaces();

      int pos = 0;

      for (const auto *face : faces) {
        FaceData faceData;

        //---

        const auto &vertices = face->getVertices();

        faceData.pos = pos;
        faceData.len = int(vertices.size());

        for (const auto &v : vertices) {
          auto &vertex = object->getVertex(v);

          const auto &model = vertex.getModel();

          if (! flipYZ_)
            buffer->addPoint(float(model.x), float(model.y), float(model.z));
          else
            buffer->addPoint(float(model.x), float(model.z), float(model.y));
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
Skybox3DObj::
render()
{
  initShader();

  updateObjectData();

  //---

//glDepthMask(GL_FALSE);
  glDepthFunc(GL_LEQUAL);

  // setup model shader
  for (auto &po : objectDatas_) {
    auto *objectData = po.second;

    objectData->buffer->bind();

    s_program->bind();

    if (useCubemap_) {
      if (cubemap_) {
        cubemap_->enable(/*enable*/true);

        glActiveTexture(GL_TEXTURE0);

        cubemap_->bind();

        //cubemap_->setParameters();
      }
    }
    else {
      if (texture_) {
        texture_->enable(/*enable*/true);

        glActiveTexture(GL_TEXTURE0);

        texture_->bind();
      }
    }

    s_program->setUniformValue("textureId", 0);

    // pass projection matrix to shader (note that in this case it could change every frame)
    s_program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));

    // camera/view transformation
    s_program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

    // model rotation
    s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

    s_program->setUniformValue("isWireframe", canvas_->isWireframe() ? 1 : 0);

#if 0
    // render model
    for (const auto &faceData : objectData->faceDatas) {
      if (! canvas_->isWireframe()) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glDrawArrays(GL_TRIANGLE_FAN, faceData.pos, faceData.len);
      }
      else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glDrawArrays(GL_TRIANGLE_FAN, faceData.pos, faceData.len);
      }
    }
#else
    if (! canvas_->isWireframe()) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
#endif

    if (useCubemap_) {
      if (cubemap_) {
        cubemap_->enable(/*enable*/false);

        cubemap_->unbind();

        //cubemap_->setParameters();
      }
    }
    else {
      if (texture_) {
        texture_->enable(/*enable*/false);

        texture_->unbind();
      }
    }

    objectData->buffer->unbind();
  }

//glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);
}

}
