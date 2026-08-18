#include <CQSandboxCube3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>

#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
Cube3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Cube3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
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
    CVector3D(-0.5, -0.5, -0.5),
    CVector3D( 0.5, -0.5, -0.5),
    CVector3D( 0.5,  0.5, -0.5),

    CVector3D( 0.5,  0.5, -0.5),
    CVector3D(-0.5,  0.5, -0.5),
    CVector3D(-0.5, -0.5, -0.5),

    CVector3D(-0.5, -0.5,  0.5),
    CVector3D( 0.5, -0.5,  0.5),
    CVector3D( 0.5,  0.5,  0.5),

    CVector3D( 0.5,  0.5,  0.5),
    CVector3D(-0.5,  0.5,  0.5),
    CVector3D(-0.5, -0.5,  0.5),

    CVector3D(-0.5,  0.5,  0.5),
    CVector3D(-0.5,  0.5, -0.5),
    CVector3D(-0.5, -0.5, -0.5),

    CVector3D(-0.5, -0.5, -0.5),
    CVector3D(-0.5, -0.5,  0.5),
    CVector3D(-0.5,  0.5,  0.5),

    CVector3D( 0.5,  0.5,  0.5),
    CVector3D( 0.5,  0.5, -0.5),
    CVector3D( 0.5, -0.5, -0.5),

    CVector3D( 0.5, -0.5, -0.5),
    CVector3D( 0.5, -0.5,  0.5),
    CVector3D( 0.5,  0.5,  0.5),

    CVector3D(-0.5, -0.5, -0.5),
    CVector3D( 0.5, -0.5, -0.5),
    CVector3D( 0.5, -0.5,  0.5),

    CVector3D( 0.5, -0.5,  0.5),
    CVector3D(-0.5, -0.5,  0.5),
    CVector3D(-0.5, -0.5, -0.5),

    CVector3D(-0.5,  0.5, -0.5),
    CVector3D( 0.5,  0.5, -0.5),
    CVector3D( 0.5,  0.5,  0.5),

    CVector3D( 0.5,  0.5,  0.5),
    CVector3D(-0.5,  0.5,  0.5),
    CVector3D(-0.5,  0.5, -0.5)
  };

  static Shape3DData::TexCoords texCoords = {
    CVector2D(0.0, 0.0),
    CVector2D(1.0, 0.0),
    CVector2D(1.0, 1.0),

    CVector2D(1.0, 1.0),
    CVector2D(0.0, 1.0),
    CVector2D(0.0, 0.0),

    CVector2D(0.0, 0.0),
    CVector2D(1.0, 0.0),
    CVector2D(1.0, 1.0),

    CVector2D(1.0, 1.0),
    CVector2D(0.0, 1.0),
    CVector2D(0.0, 0.0),

    CVector2D(1.0, 0.0),
    CVector2D(1.0, 1.0),
    CVector2D(0.0, 1.0),

    CVector2D(0.0, 1.0),
    CVector2D(0.0, 0.0),
    CVector2D(1.0, 0.0),

    CVector2D(1.0, 0.0),
    CVector2D(1.0, 1.0),
    CVector2D(0.0, 1.0),

    CVector2D(0.0, 1.0),
    CVector2D(0.0, 0.0),
    CVector2D(1.0, 0.0),

    CVector2D(0.0, 1.0),
    CVector2D(1.0, 1.0),
    CVector2D(1.0, 0.0),

    CVector2D(1.0, 0.0),
    CVector2D(0.0, 0.0),
    CVector2D(0.0, 1.0),

    CVector2D(0.0, 1.0),
    CVector2D(1.0, 1.0),
    CVector2D(1.0, 0.0),

    CVector2D(1.0, 0.0),
    CVector2D(0.0, 0.0),
    CVector2D(0.0, 1.0)
  };

  shapeData_.setPoints   (points);
  shapeData_.setTexCoords(texCoords);

  Shape3DObj::init();
}

}
