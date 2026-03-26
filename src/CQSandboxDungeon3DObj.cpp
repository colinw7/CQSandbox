#include <CQSandboxDungeon3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxGroup3DObj.h>
#include <CQSandboxShape3DObj.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CGLCamera.h>
#include <CQGLTexture.h>

#ifdef CQSANDBOX_DUNGEON
#include <CDungeon.h>
#endif

namespace CQSandbox {

#ifdef CQSANDBOX_DUNGEON
bool
Dungeon3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Dungeon3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Dungeon3DObj::
Dungeon3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
  dungeon_ = new CDungeon;

  dungeon_->init();

  group_ = new Group3DObj(canvas_);

  group_->init();

  canvas_->addNewObject(group_);
}

void
Dungeon3DObj::
init()
{
  Object3D::init();
}

void
Dungeon3DObj::
tick()
{
}

QVariant
Dungeon3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Dungeon3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "filename") {
    dungeon_->load(value.toStdString());

    updateObjs();
  }
  else if (name.left(8) == "texture.") {
    auto id = name.mid(8);

    setTexture(id, value);
  }
  else if (name.left(7) == "player.") {
    auto *player = dungeon_->getPlayer();

    auto id = name.mid(7);

    if      (id == "left")
      player->turnLeft();
    else if (id == "right")
      player->turnRight();
    else if (id == "up")
      player->moveForward();
    else if (id == "down")
      player->moveBack();

    updatePlayerCamera(true);
  }
  else if (name == "player_camera") {
    bool isGame = Util::stringToBool(value);

    updatePlayerCamera(isGame);
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Dungeon3DObj::
updatePlayerCamera(bool isGame)
{
  auto *camera = canvas_->camera();

  if (isGame) {
    auto *player = dungeon_->getPlayer();

    auto pos  = player->getPos();
    auto rpos = player->getRoomPos();
    auto dir  = player->getDirection();

    auto nc = dungeon_->getRoomCols();
    auto nr = dungeon_->getRoomRows();

    auto dx1 = dx_/nc;
    auto dy1 = dy_/nr;

    double x = pos.x*dx_ + rpos.x*dx1;
    double y = 0.5*dy_;
    double z = pos.y*dz_ + rpos.y*dy1;

    camera->setPosition(CGLVector3D(x, y, z));

    if      (dir == CCompassType::NORTH)
      camera->setYaw(90);
    else if (dir == CCompassType::SOUTH)
      camera->setYaw(-90);
    else if (dir == CCompassType::WEST)
      camera->setYaw(180);
    else if (dir == CCompassType::EAST)
      camera->setYaw(0);

    camera->setPitch(0);
  }
  else {
    camera->setPosition(CGLVector3D(0, 1, 0));
    camera->setZoom(75);
    camera->setYaw(45);
    camera->setPitch(-45);
  }
}

void
Dungeon3DObj::
setTexture(const QString &id, const QString &filename)
{
  auto p = textures_.find(id);

  if (p != textures_.end()) {
    delete (*p).second;

    textures_.erase(p);
  }

  auto *texture = new CQGLTexture;

  if (! texture->load(filename, /*flip*/false)) {
    delete texture;
    return;
  }

  textures_[id] = texture;
}

CQGLTexture *
Dungeon3DObj::
getTexture(const QString &id) const
{
  auto p = textures_.find(id);
  if (p == textures_.end()) return nullptr;

  return (*p).second;
}

void
Dungeon3DObj::
updateObjs()
{
  const CIBBox2D &bbox = dungeon_->getBBox();

  double x1 = bbox.getXMin();
  double z1 = bbox.getYMin();
  double x2 = bbox.getXMax();
  double z2 = bbox.getYMax();

  double dx = std::abs(x2 - x1);
  double dz = std::abs(z2 - z1);

  s_ = 1.0/std::max(dx, dz);

  //---

  auto nr = dungeon_->getNumRows();
  auto nc = dungeon_->getNumCols();

  dx_ = s_*dx/std::max(nc, 1U);
  dy_ = 0.1;
  dz_ = s_*dz/std::max(nr, 1U);

  //---

  double dw = 0.005;

  const auto &rooms = dungeon_->getRooms();

  for (auto *room : rooms) {
    auto pos = room->getPos();

    double x1 = pos.x*dx_;
    double y1 = 0.0;
    double z1 = pos.y*dz_;
    double x2 = x1 + dx_;
    double y2 = dy_;
    double z2 = z1 + dz_;

    auto *nwall = room->getWall(CCompassType::NORTH);
    auto *swall = room->getWall(CCompassType::SOUTH);
    auto *wwall = room->getWall(CCompassType::WEST );
    auto *ewall = room->getWall(CCompassType::EAST );

    bool nvis = nwall->getVisible();
    bool svis = swall->getVisible();
    bool wvis = wwall->getVisible();
    bool evis = ewall->getVisible();

    auto *nroom = room->getNRoom();
    auto *sroom = room->getSRoom();
    auto *wroom = room->getWRoom();
    auto *eroom = room->getERoom();

    auto addWall = [&](bool vis, double xc, double yc, double zc, double dx, double dy, double dz) {
      auto *nobj = new Shape3DObj(canvas_);

      nobj->init();

      canvas_->addNewObject(nobj);

      group_->addObject(nobj);

      nobj->setPosition(CPoint3D(xc, yc, zc));

      nobj->addCube(dx, dy, dz);

      auto *texture = (vis ? getTexture("wall") : getTexture("door"));

      if (texture)
        nobj->setTexture(texture);
    };

    if (nvis) addWall(true, (x1 + x2)/2.0, (y1 + y2)/2.0, z2, x2 - x1, y2 - y1, dw);
    if (svis) addWall(true, (x1 + x2)/2.0, (y1 + y2)/2.0, z1, x2 - x1, y2 - y1, dw);
    if (wvis) addWall(true, x1, (y1 + y2)/2.0, (z1 + z2)/2.0, dw, y2 - y1, z2 - z1);
    if (evis) addWall(true, x2, (y1 + y2)/2.0, (z1 + z2)/2.0, dw, y2 - y1, z2 - z1);

    if (! nroom && ! nvis)
      addWall(false, (x1 + x2)/2.0, (y1 + y2)/2.0, z2, x2 - x1, y2 - y1, dw);

    if (! sroom && ! svis)
      addWall(false, (x1 + x2)/2.0, (y1 + y2)/2.0, z1, x2 - x1, y2 - y1, dw);

    if (! wroom && ! wvis)
      addWall(false, x1, (y1 + y2)/2.0, (z1 + z2)/2.0, dw, y2 - y1, z2 - z1);

    if (! eroom && ! evis)
      addWall(false, x2, (y1 + y2)/2.0, (z1 + z2)/2.0, dw, y2 - y1, z2 - z1);

    // add floor
    auto addFloor = [&](double xc, double yc, double zc, double dx, double dy, double dz) {
      auto *nobj = new Shape3DObj(canvas_);

      nobj->init();

      canvas_->addNewObject(nobj);

      group_->addObject(nobj);

      nobj->setPosition(CPoint3D(xc, yc, zc));

      nobj->addCube(dx, dy, dz);

      auto *texture = getTexture("floor");

      if (texture)
        nobj->setTexture(texture);
    };

    addFloor((x1 + x2)/2.0, y1, (z1 + z2)/2.0, x2 - x1, dw, z2 - z1);

    group_->initOrigin();
  }
}

void
Dungeon3DObj::
render()
{
}
#endif

}
