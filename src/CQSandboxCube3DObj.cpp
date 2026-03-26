#include <CQSandboxCube3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
Cube3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Cube3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Cube3DObj::
Cube3DObj(Canvas3D *canvas) :
 Shape3DObj(canvas)
{
}

void
Cube3DObj::
init()
{
  Object3D::init();

  //---

  static Shape3DData::Points points = {
    CGLVector3D(-0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f,  0.5f, -0.5f),

    CGLVector3D( 0.5f,  0.5f, -0.5f),
    CGLVector3D(-0.5f,  0.5f, -0.5f),
    CGLVector3D(-0.5f, -0.5f, -0.5f),

    CGLVector3D(-0.5f, -0.5f,  0.5f),
    CGLVector3D( 0.5f, -0.5f,  0.5f),
    CGLVector3D( 0.5f,  0.5f,  0.5f),

    CGLVector3D( 0.5f,  0.5f,  0.5f),
    CGLVector3D(-0.5f,  0.5f,  0.5f),
    CGLVector3D(-0.5f, -0.5f,  0.5f),

    CGLVector3D(-0.5f,  0.5f,  0.5f),
    CGLVector3D(-0.5f,  0.5f, -0.5f),
    CGLVector3D(-0.5f, -0.5f, -0.5f),

    CGLVector3D(-0.5f, -0.5f, -0.5f),
    CGLVector3D(-0.5f, -0.5f,  0.5f),
    CGLVector3D(-0.5f,  0.5f,  0.5f),

    CGLVector3D( 0.5f,  0.5f,  0.5f),
    CGLVector3D( 0.5f,  0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f, -0.5f),

    CGLVector3D( 0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f,  0.5f),
    CGLVector3D( 0.5f,  0.5f,  0.5f),

    CGLVector3D(-0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f,  0.5f),

    CGLVector3D( 0.5f, -0.5f,  0.5f),
    CGLVector3D(-0.5f, -0.5f,  0.5f),
    CGLVector3D(-0.5f, -0.5f, -0.5f),

    CGLVector3D(-0.5f,  0.5f, -0.5f),
    CGLVector3D( 0.5f,  0.5f, -0.5f),
    CGLVector3D( 0.5f,  0.5f,  0.5f),

    CGLVector3D( 0.5f,  0.5f,  0.5f),
    CGLVector3D(-0.5f,  0.5f,  0.5f),
    CGLVector3D(-0.5f,  0.5f, -0.5f)
  };

  static Shape3DData::TexCoords texCoords = {
    CGLVector2D(0.0f, 0.0f),
    CGLVector2D(1.0f, 0.0f),
    CGLVector2D(1.0f, 1.0f),

    CGLVector2D(1.0f, 1.0f),
    CGLVector2D(0.0f, 1.0f),
    CGLVector2D(0.0f, 0.0f),

    CGLVector2D(0.0f, 0.0f),
    CGLVector2D(1.0f, 0.0f),
    CGLVector2D(1.0f, 1.0f),

    CGLVector2D(1.0f, 1.0f),
    CGLVector2D(0.0f, 1.0f),
    CGLVector2D(0.0f, 0.0f),

    CGLVector2D(1.0f, 0.0f),
    CGLVector2D(1.0f, 1.0f),
    CGLVector2D(0.0f, 1.0f),

    CGLVector2D(0.0f, 1.0f),
    CGLVector2D(0.0f, 0.0f),
    CGLVector2D(1.0f, 0.0f),

    CGLVector2D(1.0f, 0.0f),
    CGLVector2D(1.0f, 1.0f),
    CGLVector2D(0.0f, 1.0f),

    CGLVector2D(0.0f, 1.0f),
    CGLVector2D(0.0f, 0.0f),
    CGLVector2D(1.0f, 0.0f),

    CGLVector2D(0.0f, 1.0f),
    CGLVector2D(1.0f, 1.0f),
    CGLVector2D(1.0f, 0.0f),

    CGLVector2D(1.0f, 0.0f),
    CGLVector2D(0.0f, 0.0f),
    CGLVector2D(0.0f, 1.0f),

    CGLVector2D(0.0f, 1.0f),
    CGLVector2D(1.0f, 1.0f),
    CGLVector2D(1.0f, 0.0f),

    CGLVector2D(1.0f, 0.0f),
    CGLVector2D(0.0f, 0.0f),
    CGLVector2D(0.0f, 1.0f)
  };

  shapeData_.setPoints   (points);
  shapeData_.setTexCoords(texCoords);

  Shape3DObj::init();
}

}
