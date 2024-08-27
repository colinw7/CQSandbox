#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>
#include <CQGLUtil.h>

#include <CImportBase.h>
#include <CGeomScene3D.h>
#include <CGeomTexture.h>
#include <CLorenzCalc.h>

#include <CQGLBuffer.h>
#include <CGLTexture.h>
#include <CGLCamera.h>

#include <CFile.h>

#include <QFileInfo>
#include <QMouseEvent>

#define Q(x) #x
#define QUOTE(x) Q(x)

namespace CQSandbox {

template<typename T>
int createObjectProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv) {
  auto *th = static_cast<Canvas3D *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);

  if (! T::create(th, args))
    return TCL_ERROR;

  return TCL_OK;
}

QString
point3DToString(const CPoint3D &p) {
  auto xstr = QString::number(p.x);
  auto ystr = QString::number(p.y);
  auto zstr = QString::number(p.z);

  return xstr + " " + ystr + " " + zstr;
}

QString
vector3DToString(const CGLVector3D &p) {
  auto xstr = QString::number(p.x());
  auto ystr = QString::number(p.y());
  auto zstr = QString::number(p.z());

  return xstr + " " + ystr + " " + zstr;
}

CPoint3D
stringToPoint3D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CPoint3D p;

  if (strs.size() >= 3) {
    bool ok;
    auto x = strs[0].toDouble(&ok);
    auto y = strs[1].toDouble(&ok);
    auto z = strs[2].toDouble(&ok);

    p.x = x;
    p.y = y;
    p.z = z;
  }

  return p;
}

//---

OpenGLWindow::
OpenGLWindow(QWidget *parent) :
 QOpenGLWidget(parent)
{
  //setSurfaceType(QWindow::OpenGLSurface);
}

OpenGLWindow::
~OpenGLWindow()
{
}


void
OpenGLWindow::
initializeGL()
{
  initializeOpenGLFunctions();

  initialize();
}

void
OpenGLWindow::
resizeGL(int, int)
{
}

void
OpenGLWindow::
paintGL()
{
  render();
}

void
OpenGLWindow::
initialize()
{
}

void
OpenGLWindow::
render()
{
}

void
OpenGLWindow::
setAnimating(bool animating)
{
  animating_ = animating;

  update();
}

void
OpenGLWindow::
setType(const Type &type)
{
  if (type != type_) {
    type_ = type;

    Q_EMIT typeChanged();
  }
}

bool
OpenGLWindow::
event(QEvent *event)
{
  return QOpenGLWidget::event(event);
}

void
OpenGLWindow::
genBufferId(GLuint *id)
{
  glGenBuffers(1, id);
}

void
OpenGLWindow::
bindArrayBuffer(GLuint id)
{
  glBindBuffer(GL_ARRAY_BUFFER, id);
}

void
OpenGLWindow::
bindArrayBufferData(size_t size, const void *data, GLuint type)
{
  glBufferData(GL_ARRAY_BUFFER, size, data, type);
}

void
OpenGLWindow::
enableVertexAttribArray(GLuint ind)
{
  glEnableVertexAttribArray(ind);
}

void
OpenGLWindow::
setBufferSubData(size_t size, const void *data)
{
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

void
OpenGLWindow::
setVertexAttribPointer(GLuint attrId, size_t size, GLuint type, GLuint normalized)
{
  glVertexAttribPointer(attrId, size, type, normalized, /*stride*/ 0, /* data offset */ (void*)0);
}

void
OpenGLWindow::
setVertexAttribDivisor(GLuint attrId, size_t n)
{
  glVertexAttribDivisor(attrId, n);
}

void
OpenGLWindow::
drawInstancesTriangles(size_t n)
{
  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, n);
}

//---

QOpenGLShaderProgram* Canvas3D::s_lightShaderProgram;

Canvas3D::
Canvas3D(App *app) :
 app_(app)
{
  setFocusPolicy(Qt::StrongFocus);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void
Canvas3D::
init()
{
  addCommands();

  app_->runTclCmd("proc init { args } { }");
  app_->runTclCmd("proc update { args } { }");
}

void
Canvas3D::
addCommands()
{
   auto *tcl = app_->tcl();

  // global
  tcl->createObjCommand("sb3d::load_model",
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas3D::loadModelProc),
    static_cast<CQTcl::ObjCmdData>(this));

  tcl->createObjCommand("sb3d::particle_list",
    reinterpret_cast<CQTcl::ObjCmdProc>(&createObjectProc<ParticleListObj>),
    static_cast<CQTcl::ObjCmdData>(this));
}

void
Canvas3D::
createObjCommand(Object3D *obj)
{
  auto *tcl = app_->tcl();

  auto name = obj->getCommandName();

  tcl->createObjCommand(name,
    reinterpret_cast<CQTcl::ObjCmdProc>(&Canvas3D::objectCommandProc),
    static_cast<CQTcl::ObjCmdData>(obj));
}

QString
Canvas3D::
addNewObject(Object3D *obj)
{
  addObject(obj);

  obj->setInd(++lastInd_);

  createObjCommand(obj);

  return obj->calcId();
}

void
Canvas3D::
addObject(Object3D *obj)
{
  objects_.push_back(obj);
}

int
Canvas3D::
loadModelProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *th = static_cast<Canvas3D *>(clientData);
  assert(th);

  auto args = th->app()->getArgs(objc, objv);
  if (args.size() < 1) return TCL_ERROR;

  auto filename = args[0];

  QFileInfo fi(filename);

  if (! fi.exists()) {
    th->app()->errorMsg(QString("File '%1' does not exist").arg(filename));
    return TCL_ERROR;
  }

  auto *model = new Model3D(th);

  if (! model->load(filename)) {
    delete model;
    return TCL_ERROR;
  }

  auto name = th->addNewObject(model);

  auto *tcl = th->app()->tcl();

  tcl->setResult(name);

  return TCL_OK;
}

int
Canvas3D::
objectCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto *obj = static_cast<Object3D *>(clientData);
  assert(obj);

  auto *canvas = obj->canvas();
  auto *app    = canvas->app();

  auto *tcl = app->tcl();

  auto args = app->getArgs(objc, objv);

  if      (args[0] == "get") {
    if (args.size() > 1) {
      auto name = args[1];

      QStringList args1;

      for (int i = 2; i < args.length(); ++i)
        args1.push_back(args[i]);

      auto res = obj->getValue(name, args1);

      tcl->setResult(res);
    }
    else {
      app->errorMsg("Missing args for get");
    }
  }
  else if (args[0] == "set") {
    if (args.size() > 2) {
      auto name  = args[1];
      auto value = args[2];

      QStringList args1;

      for (int i = 3; i < args.length(); ++i)
        args1.push_back(args[i]);

      obj->setValue(args[1], args[2], args1);
    }
    else {
      app->errorMsg("Missing args for set");
    }
  }
  else {
    app->errorMsg(QString("Bad object command '%1'").arg(args[0]));
  }

  return TCL_OK;
}

void
Canvas3D::
initialize()
{
  // camera
  camera_ = new CGLCamera(CGLVector3D(0.0f, 0.0f, 1.0f));

  camera_->setLastPos(float(this->width())/2.0f, float(this->height())/2.0f);

  //---

  if (! s_lightShaderProgram) {
    QString buildDir = QUOTE(BUILD_DIR);

    s_lightShaderProgram = new QOpenGLShaderProgram;

    if (! s_lightShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                        buildDir + "/shaders/light.vs"))
      std::cerr << s_lightShaderProgram->log().toStdString() << "\n";

    if (! s_lightShaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                                        buildDir + "/shaders/light.fs"))
      std::cerr << s_lightShaderProgram->log().toStdString() << "\n";

    s_lightShaderProgram->link();
  }

  //---

  // set up vertex data (and buffer(s)) and configure vertex attributes
  lightBuffer_ = new CQGLBuffer(s_lightShaderProgram);

  lightBuffer_->addPoint(-0.5f, -0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f, -0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f,  0.5f, -0.5f);

  lightBuffer_->addPoint( 0.5f,  0.5f, -0.5f);
  lightBuffer_->addPoint(-0.5f,  0.5f, -0.5f);
  lightBuffer_->addPoint(-0.5f, -0.5f, -0.5f);

  lightBuffer_->addPoint(-0.5f, -0.5f,  0.5f);
  lightBuffer_->addPoint( 0.5f, -0.5f,  0.5f);
  lightBuffer_->addPoint( 0.5f,  0.5f,  0.5f);

  lightBuffer_->addPoint( 0.5f,  0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f,  0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f, -0.5f,  0.5f);

  lightBuffer_->addPoint(-0.5f,  0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f,  0.5f, -0.5f);
  lightBuffer_->addPoint(-0.5f, -0.5f, -0.5f);

  lightBuffer_->addPoint(-0.5f, -0.5f, -0.5f);
  lightBuffer_->addPoint(-0.5f, -0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f,  0.5f,  0.5f);

  lightBuffer_->addPoint( 0.5f,  0.5f,  0.5f);
  lightBuffer_->addPoint( 0.5f,  0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f, -0.5f, -0.5f);

  lightBuffer_->addPoint( 0.5f, -0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f, -0.5f,  0.5f);
  lightBuffer_->addPoint( 0.5f,  0.5f,  0.5f);

  lightBuffer_->addPoint(-0.5f, -0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f, -0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f, -0.5f,  0.5f);

  lightBuffer_->addPoint( 0.5f, -0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f, -0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f, -0.5f, -0.5f);

  lightBuffer_->addPoint(-0.5f,  0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f,  0.5f, -0.5f);
  lightBuffer_->addPoint( 0.5f,  0.5f,  0.5f);

  lightBuffer_->addPoint( 0.5f,  0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f,  0.5f,  0.5f);
  lightBuffer_->addPoint(-0.5f,  0.5f, -0.5f);

  lightBuffer_->load();

  //---

  app_->runTclCmd("init");
}

void
Canvas3D::
render()
{
  const qreal retinaScale = devicePixelRatio();

  glViewport(0, 0, width()*retinaScale, height()*retinaScale);

  glClearColor(bgColor_.redF(), bgColor_.greenF(), bgColor_.blueF(), 1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glDisable(GL_CULL_FACE);

  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  //---

  aspect_ = float(width())/float(height());

  projectionMatrix_ = CGLMatrix3D::perspective(camera_->zoom(), aspect_, 0.1f, 100.0f);
  viewMatrix_       = camera_->getViewMatrix();

  //---

  // lighting
  viewPos_ = camera_->position();

  //---

  for (auto *obj : objects_)
    obj->render();

  //---

  // setup light shader
  lightBuffer_->bind();

  s_lightShaderProgram->bind();

  s_lightShaderProgram->setUniformValue("projection",
    CQGLUtil::toQMatrix(projectionMatrix()));
  s_lightShaderProgram->setUniformValue("view", CQGLUtil::toQMatrix(viewMatrix()));

  auto lightMatrix = CGLMatrix3D::translation(lightPos());
  lightMatrix.scaled(0.01f, 0.01f, 0.01f);
  s_lightShaderProgram->setUniformValue("model", CQGLUtil::toQMatrix(lightMatrix));

  // draw light
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  //lightBuffer_->drawTriangles();

  lightBuffer_->unbind();
}

void
Canvas3D::
mousePressEvent(QMouseEvent *e)
{
  float xpos = float(e->x());
  float ypos = float(e->y());

  auto type = this->type();

  if (type == Type::CAMERA)
    camera_->setLastPos(xpos, ypos);

  update();

  pressed_ = true;
}

void
Canvas3D::
mouseReleaseEvent(QMouseEvent *)
{
  update();

  pressed_ = false;
}

void
Canvas3D::
mouseMoveEvent(QMouseEvent *e)
{
  if (pressed_) {
    float xpos = float(e->x());
    float ypos = float(e->y());

    //---

    auto type = this->type();

    if (type == Type::CAMERA) {
      float xoffset, yoffset;

      camera_->deltaPos(xpos, ypos, xoffset, yoffset);

      camera_->setLastPos(xpos, ypos);

      camera_->processMouseMovement(xoffset, yoffset);

      //Q_EMIT cameraChanged();
    }

    update();
  }
}

void
Canvas3D::
wheelEvent(QWheelEvent *e)
{
  float y = float(e->angleDelta().y()/250.0);

  auto type = this->type();

  if (type == Type::CAMERA)
    camera_->processMouseScroll(y);

  update();
}

void
Canvas3D::
keyPressEvent(QKeyEvent *e)
{
  bool isShift = (e->modifiers() & Qt::ShiftModifier);

  auto dt = 0.01f; /* camera_->deltaTime(); */
  auto da = M_PI/180.0;

  if (isShift) {
    dt = -dt;
    da = -da;
  }

  auto type = this->type();

  if      (e->key() == Qt::Key_W) {
    if      (type == Type::CAMERA) {
      camera_->processKeyboard(CGLCamera::Movement::FORWARD, dt);

      //Q_EMIT cameraChanged();
    }
    else if (type == Type::LIGHT)
      lightPos_ += CGLVector3D(0.0f, 0.1f, 0.0f);
  }
  else if (e->key() == Qt::Key_S) {
    if      (type == Type::CAMERA) {
      camera_->processKeyboard(CGLCamera::Movement::BACKWARD, dt);

      //Q_EMIT cameraChanged();
    }
    else if (type == Type::LIGHT)
      lightPos_ -= CGLVector3D(0.0f, 0.1f, 0.0f);
  }
  else if (e->key() == Qt::Key_A) {
    if      (type == Type::CAMERA) {
      camera_->processKeyboard(CGLCamera::Movement::LEFT, dt);

      //Q_EMIT cameraChanged();
    }
    else if (type == Type::LIGHT)
      lightPos_ -= CGLVector3D(0.1f, 0.0f, 0.0f);
  }
  else if (e->key() == Qt::Key_D) {
    if      (type == Type::CAMERA) {
      camera_->processKeyboard(CGLCamera::Movement::RIGHT, dt);

      //Q_EMIT cameraChanged();
    }
    else if (type == Type::LIGHT)
      lightPos_ += CGLVector3D(0.1f, 0.0f, 0.0f);
  }
  else if (e->key() == Qt::Key_C) {
    setType(Type::CAMERA);
  }
  else if (e->key() == Qt::Key_L) {
    setType(Type::LIGHT);
  }
  else if (e->key() == Qt::Key_M) {
    setType(Type::MODEL);
  }
  else if (e->key() == Qt::Key_X) {
//  if (type == Type::MODEL) {
//    for (auto *obj : objects_)
//      obj->angle.incX(da);
//  }
  }
  else if (e->key() == Qt::Key_Y) {
//  if (type == Type::MODEL) {
//    for (auto *obj : objects_)
//      obj->angle.incY(da);
//  }
  }
  else if (e->key() == Qt::Key_Z) {
//  if (type == Type::MODEL) {
//    for (auto *obj : objects_)
//      obj->angle.incZ(da);
//  }
  }
  else if (e->key() == Qt::Key_Up) {
    if (type == Type::LIGHT)
      lightPos_ += CGLVector3D(0.0f, 0.0f, 0.1f);
  }
  else if (e->key() == Qt::Key_Down) {
    if (type == Type::LIGHT)
      lightPos_ -= CGLVector3D(0.0f, 0.0f, 0.1f);
  }

  update();
}

//---

Object3D::
Object3D(Canvas3D *canvas) :
 canvas_(canvas)
{
}

QString
Object3D::
getCommandName() const
{
  return QString("object3d.%1").arg(ind_);
}

QString
Object3D::
calcId() const
{
  auto id = this->id();

  if (id == "")
    id = getCommandName();

  return id;
}

QVariant
Object3D::
getValue(const QString &name, const QStringList &)
{
  auto *app = canvas()->app();

  if (name == "id")
    return id();
  else {
    app->errorMsg(QString("Invalid get name '%1'").arg(name));
    return QVariant();
  }
}

void
Object3D::
setValue(const QString &name, const QString &value, const QStringList &)
{
  auto *app = canvas()->app();

  if (name == "id")
    setId(value);
  else
    app->errorMsg(QString("Invalid set name '%1'").arg(name));
}

void
Object3D::
render()
{
}

//---

QOpenGLShaderProgram* Model3D::s_modelShaderProgram;

Model3D::
Model3D(Canvas3D *canvas) :
 Object3D(canvas)
{
  if (! s_modelShaderProgram) {
    QString buildDir = QUOTE(BUILD_DIR);

    s_modelShaderProgram = new QOpenGLShaderProgram;

    if (! s_modelShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                        buildDir + "/shaders/model.vs"))
      std::cerr << s_modelShaderProgram->log().toStdString() << "\n";

    if (! s_modelShaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                                        buildDir + "/shaders/model.fs"))
      std::cerr << s_modelShaderProgram->log().toStdString() << "\n";

    s_modelShaderProgram->link();
  }
}

bool
Model3D::
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

  updateObjectData();

  return true;
}

void
Model3D::
render()
{
  // lighting
  CGLVector3D lightColor(1.0f, 1.0f, 1.0f);

  //---

  // setup model shader
  for (auto &po : objectDatas_) {
    auto *objectData = po.second;

    objectData->buffer->bind();

    s_modelShaderProgram->bind();

    s_modelShaderProgram->setUniformValue("lightColor", CQGLUtil::toVector(lightColor));
    s_modelShaderProgram->setUniformValue("lightPos"  , CQGLUtil::toVector(canvas_->lightPos()));
    s_modelShaderProgram->setUniformValue("viewPos"   , CQGLUtil::toVector(canvas_->viewPos()));

    s_modelShaderProgram->setUniformValue("ambientStrength" , float(canvas_->ambient()));
    s_modelShaderProgram->setUniformValue("diffuseStrength" , float(canvas_->diffuse()));
    s_modelShaderProgram->setUniformValue("specularStrength", float(canvas_->specular()));
    s_modelShaderProgram->setUniformValue("shininess"       , float(canvas_->shininess()));

    // pass projection matrix to shader (note that in this case it could change every frame)
    s_modelShaderProgram->setUniformValue("projection",
      CQGLUtil::toQMatrix(canvas_->projectionMatrix()));

    // camera/view transformation
    s_modelShaderProgram->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

    // model rotation
    auto modelMatrix = CGLMatrix3D::identity();
    modelMatrix.scaled(sceneScale_, sceneScale_, sceneScale_);
    modelMatrix.rotated(canvas_->modelXAngle(), CGLVector3D(1.0, 0.0, 0.0));
    modelMatrix.rotated(canvas_->modelYAngle(), CGLVector3D(0.0, 1.0, 0.0));
    modelMatrix.rotated(canvas_->modelZAngle(), CGLVector3D(0.0, 0.0, 1.0));
    modelMatrix.translated(float(-sceneCenter_.getX()),
                           float(-sceneCenter_.getY()),
                           float(-sceneCenter_.getZ()));
    s_modelShaderProgram->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix));

    // render model
    for (const auto &faceData : objectData->faceDatas) {
      bool useDiffuseTexture = faceData.diffuseTexture;

      s_modelShaderProgram->setUniformValue("useDiffuseTexture", useDiffuseTexture);

      if (useDiffuseTexture) {
        glActiveTexture(GL_TEXTURE0);
        faceData.diffuseTexture->bind();

        s_modelShaderProgram->setUniformValue("diffuseTexture", 0);
      }

      //---

      bool useSpecularTexture = faceData.specularTexture;

      s_modelShaderProgram->setUniformValue("useSpecularTexture", useSpecularTexture);

      if (useSpecularTexture) {
        glActiveTexture(GL_TEXTURE1);
        faceData.specularTexture->bind();

        s_modelShaderProgram->setUniformValue("specularTexture", 1);
      }

      //---

      bool useNormalTexture = faceData.normalTexture;

      s_modelShaderProgram->setUniformValue("useNormalTexture", useNormalTexture);

      if (useNormalTexture) {
        glActiveTexture(GL_TEXTURE2);
        faceData.normalTexture->bind();

        s_modelShaderProgram->setUniformValue("normalTexture", 2);
      }

      //---

      s_modelShaderProgram->setUniformValue("isWireframe", 0);

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      glDrawArrays(GL_TRIANGLE_FAN, faceData.pos, faceData.len);

      if (canvas_->isPolygonLine()) {
        s_modelShaderProgram->setUniformValue("isWireframe", 1);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glDrawArrays(GL_TRIANGLE_FAN, faceData.pos, faceData.len);
      }
    }

    objectData->buffer->unbind();
  }
}

void
Model3D::
updateObjectData()
{
  // set up vertex data (and buffer(s)) and configure vertex attributes
  CVector3D sceneSize(1, 1, 1);

  if (import_) {
    auto &scene = import_->getScene();

    CBBox3D bbox;

    scene.getBBox(bbox);

    sceneSize    = bbox.getSize();
    sceneCenter_ = bbox.getCenter();
    //std::cerr << "Scene Center : " << sceneCenter_.getX() << " " <<
    //             sceneCenter_.getY() << " " << sceneCenter_.getZ() << "\n";

    for (auto *object : scene.getObjects()) {
      ObjectData *objectData { nullptr };

      auto pd = objectDatas_.find(object);

      if (pd == objectDatas_.end())
        pd = objectDatas_.insert(pd, ObjectDatas::value_type(object, new ObjectData));

      objectData = (*pd).second;

      if (! objectData->buffer)
        objectData->buffer = new CQGLBuffer(s_modelShaderProgram);

      //---

      auto *buffer = objectData->buffer;

      buffer->clearPoints();
      buffer->clearNormals();
      buffer->clearColors();
      buffer->clearTexturePoints();

      objectData->faceDatas.clear();

      //---

      const auto &faces = object->getFaces();

      int pos = 0;

      for (const auto *face : faces) {
        FaceData faceData;

        //---

        const auto &color = face->getColor();

        auto *diffuseTexture  = face->getDiffuseTexture();
        auto *specularTexture = face->getSpecularTexture();
        auto *normalTexture   = face->getNormalTexture();

        if (diffuseTexture) {
          auto pt = glTextures_.find(diffuseTexture->id());

          if (pt == glTextures_.end()) {
            const auto &image = diffuseTexture->image()->image();

            auto *glTexture = new CGLTexture(image);

            pt = glTextures_.insert(pt, GLTextures::value_type(diffuseTexture->id(), glTexture));
          }

          faceData.diffuseTexture = (*pt).second;
        }

        if (specularTexture) {
          auto pt = glTextures_.find(specularTexture->id());

          if (pt == glTextures_.end()) {
            const auto &image = specularTexture->image()->image();

            auto *glTexture = new CGLTexture(image);

            pt = glTextures_.insert(pt, GLTextures::value_type(specularTexture->id(), glTexture));
          }

          faceData.specularTexture = (*pt).second;
        }

        if (normalTexture) {
          auto pt = glTextures_.find(normalTexture->id());

          if (pt == glTextures_.end()) {
            const auto &image = normalTexture->image()->image();

            auto *glTexture = new CGLTexture(image);

            pt = glTextures_.insert(pt, GLTextures::value_type(normalTexture->id(), glTexture));
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
          face->calcNormal(normal);

        const auto &vertices = face->getVertices();

        faceData.pos = pos;
        faceData.len = int(vertices.size());

        for (const auto &v : vertices) {
          auto &vertex = object->getVertex(v);

          const auto &model = vertex.getModel();

          if (! flipYZ_) {
            buffer->addPoint(float(model.x), float(model.y), float(model.z));
            buffer->addNormal(float(normal.getX()), float(normal.getY()), float(normal.getZ()));
          }
          else {
            buffer->addPoint(float(model.x), float(model.z), float(model.y));
            buffer->addNormal(float(normal.getX()), float(normal.getZ()), float(normal.getY()));
          }

          buffer->addColor(color.getRedF(), color.getGreenF(), color.getBlueF());

          if (faceData.diffuseTexture) {
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

  sceneScale_ = float(1.0/max3(sceneSize.getX(), sceneSize.getY(), sceneSize.getZ()));
  //std::cerr << "Scene Scale : " << sceneScale_ << "\n";
}

//---

struct ProgramData {
  QOpenGLShaderProgram *program           { nullptr };
  GLint                 positionAttr      { 0 };
  GLint                 centerAttr        { 0 };
  GLint                 colorAttr         { 0 };
  GLint                 projectionUniform { 0 };
  GLint                 viewUniform       { 0 };
  GLint                 modelUniform      { 0 };
};

size_t       ParticleListObj::s_maxPoints = 1024;
ProgramData *ParticleListObj::s_program   = nullptr;

bool
ParticleListObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new ParticleListObj(canvas);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

ParticleListObj::
ParticleListObj(Canvas3D *canvas) :
 Object3D(canvas)
{
  updateGL();
}

QVariant
ParticleListObj::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "size")
    return Util::intToString(int(points_.size()));
  else if (name == "position") {
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(points_.size()))
        return QVariant();

      return vector3DToString(points_[i]);
    }
    else
      return QVariant();
  }
  else
    return Object3D::getValue(name, args);
}

void
ParticleListObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

  if      (name == "size") {
    auto n = Util::stringToInt(value);

    setNumPoints(n);
  }
  else if (name == "position") {
    // get index from args
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(points_.size()))
        return;

      points_[i] = stringToPoint3D(tcl, value);
    }
    else
      app->errorMsg("Missing index for position");
  }
  else if (name == "generator") {
    int n = 1000;

    if (args.size() > 0)
      n = Util::stringToInt(args[0]);

    if (value == "lorenz") {
      setNumPoints(n);

      CLorenzCalc calc(0, n);

      int j = 0;

      for (int i = 0; i < calc.getIterationEnd(); i++) {
        double x, y, z;

        calc.nextValue(x, y, z);

        if (i >= calc.getIterationStart()) {
          points_[j] = CGLVector3D(x, y, z);

          ++j;
        }
      }
    }
  }
  else
    Object3D::setValue(name, value, args);
}

void
ParticleListObj::
setNumPoints(int n)
{
  auto n1 = int(points_.size());

  if      (n > n1) {
    for (int i = 0; i < n - n1; ++i) {
      points_.emplace_back();
      colors_.emplace_back(1.0, 1.0, 1.0);
    }
  }
  else if (n < n1) {
    for (int i = 0; i < n1 - n; ++i) {
      points_.pop_back();
      colors_.pop_back();
    }
  }
}

void
ParticleListObj::
updateGL()
{
  if (! s_program) {
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "attribute highp vec4 position;\n"
      "attribute highp vec4 center;\n"
      "attribute lowp  vec4 color;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "varying lowp vec4 col;\n"
      "void main() {\n"
      "  col = color;\n"
      "  gl_Position = projection * view * model * (position + center);\n"
      "}\n";

    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "varying lowp vec4 col;\n"
      "void main() {\n"
      "  gl_FragColor = col;\n"
      "}\n";

    s_program = new ProgramData;

    s_program->program = new QOpenGLShaderProgram(this);

    s_program->program->addShaderFromSourceCode(QOpenGLShader::Vertex  , vertexShaderSource);
    s_program->program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    s_program->program->link();

    // get program variables
    s_program->positionAttr = s_program->program->attributeLocation("position");
    Q_ASSERT(s_program->positionAttr != -1);

    s_program->centerAttr = s_program->program->attributeLocation("center");
    Q_ASSERT(s_program->centerAttr != -1);

    s_program->colorAttr = s_program->program->attributeLocation("color");
    Q_ASSERT(s_program->colorAttr != -1);

    s_program->projectionUniform = s_program->program->uniformLocation("projection");
    Q_ASSERT(s_program->projectionUniform != -1);

    s_program->viewUniform = s_program->program->uniformLocation("view");
    Q_ASSERT(s_program->viewUniform != -1);

    s_program->modelUniform = s_program->program->uniformLocation("model");
    Q_ASSERT(s_program->modelUniform != -1);
  }

  // The VBO containing the 4 vertices of the particles.
  // Thanks to instancing, they will be shared by all particles.
  static const GLfloat g_vertex_buffer_data[] = {
   -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
   -0.5f,  0.5f, 0.0f,
    0.5f,  0.5f, 0.0f,
  };

  canvas_->genBufferId(&billboard_vertex_buffer_);
  canvas_->bindArrayBuffer(billboard_vertex_buffer_);
  canvas_->bindArrayBufferData(sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

  // The VBO containing the positions and sizes of the particles
  canvas_->genBufferId(&particles_position_buffer_);
  canvas_->bindArrayBuffer(particles_position_buffer_);
  // Initialize with empty (null) buffer : it will be updated later, each frame.
  canvas_->bindArrayBufferData(s_maxPoints*sizeof(CGLVector3D), nullptr, GL_STREAM_DRAW);

  // The VBO containing the colors of the particles
  canvas_->genBufferId(&particles_color_buffer_);
  canvas_->bindArrayBuffer(particles_color_buffer_);
  // Initialize with empty (null) buffer : it will be updated later, each frame.
  canvas_->bindArrayBufferData(s_maxPoints*sizeof(Color), nullptr, GL_STREAM_DRAW);
}

void
ParticleListObj::
render()
{
  s_program->program->bind();

  s_program->program->setUniformValue("projection",
    CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program->program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));
  auto modelMatrix = CGLMatrix3D::identity();
  modelMatrix.scaled(0.1, 0.1, 0.1);
  s_program->program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix));

  auto n = points_.size();

  // Update the buffers that OpenGL uses for rendering.
  // There are much more sophisticated means to stream data from the CPU to the GPU,
  // but this is outside the scope of this tutorial.
  // http://www.opengl.org/wiki/Buffer_Object_Streaming
  canvas_->bindArrayBuffer(particles_position_buffer_);
  // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  canvas_->bindArrayBufferData(s_maxPoints*sizeof(CGLVector3D), nullptr, GL_STREAM_DRAW);
  canvas_->setBufferSubData(n*sizeof(CGLVector3D), &points_[0]);

  canvas_->bindArrayBuffer(particles_color_buffer_);
  // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  canvas_->bindArrayBufferData(s_maxPoints*sizeof(Color), nullptr, GL_STREAM_DRAW);
  canvas_->setBufferSubData(n*sizeof(Color), &colors_[0]);

  // 1rst attribute buffer : vertices
  canvas_->enableVertexAttribArray(s_program->positionAttr);
  canvas_->bindArrayBuffer(billboard_vertex_buffer_);
  canvas_->setVertexAttribPointer(
   s_program->positionAttr,
   3, // size
   GL_FLOAT, // type
   GL_FALSE // normalized?
  );

  // 2nd attribute buffer : positions of particles' centers
  canvas_->enableVertexAttribArray(s_program->centerAttr);
  canvas_->bindArrayBuffer(particles_position_buffer_);
  canvas_->setVertexAttribPointer(
   s_program->centerAttr,
   4, // size : x + y + z + size => 4
   GL_FLOAT, // type
   GL_FALSE // normalized?
  );

  // 3rd attribute buffer : particles' colors
  canvas_->enableVertexAttribArray(s_program->colorAttr);
  canvas_->bindArrayBuffer(particles_color_buffer_);
  canvas_->setVertexAttribPointer(
   s_program->colorAttr,
   4, // size : r + g + b + a => 4
   GL_UNSIGNED_BYTE, // type
   GL_TRUE // normalized? *** YES, this means that the unsigned char[4] will
           // be accessible with a vec4 (floats) in the shader ***
  );

  canvas_->drawInstancesTriangles(n);

  // These functions are specific to glDrawArrays*Instanced*.
  // The first parameter is the attribute buffer we're talking about.
  // The second parameter is the "rate at which generic vertex attributes advance when
  // rendering multiple instances"
  // http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml

  // particles vertices : always reuse the same 4 vertices -> 0
  canvas_->setVertexAttribDivisor(s_program->positionAttr, 0);
  // center per quad -> 1
  canvas_->setVertexAttribDivisor(s_program->centerAttr, 1);
  // color per quad -> 1
  canvas_->setVertexAttribDivisor(s_program->colorAttr, 1);

  // Draw the particules !
  // This draws many times a small triangle_strip (which looks like a quad).
  // This is equivalent to :
  // for(i in n) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4),
  // but faster.
  canvas_->drawInstancesTriangles(n);

  s_program->program->release();
}

}
