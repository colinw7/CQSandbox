#include <CQSandboxFieldRunners3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#ifdef CQSANDBOX_FIELD_RUNNERS
#include <CFieldRunners.h>
#endif

namespace CQSandbox {

#ifdef CQSANDBOX_FIELD_RUNNERS
bool
FieldRunners3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new FieldRunners3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

FieldRunners3DObj::
FieldRunners3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
  enum { NUM_ROWS = 15 };
  enum { NUM_COLS = 26 };

  runners_ = new CFieldRunners;

  runners_->init();

  runners_->build(NUM_ROWS, NUM_COLS);
}

void
FieldRunners3DObj::
init()
{
  Object3D::init();
}

void
FieldRunners3DObj::
tick()
{
  runners_->update();
}

QVariant
FieldRunners3DObj::
getValue(const QString &name, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  struct Index {
    int ix { -1 };
    int iy { -1 };

    bool isValid() { return (ix >= 0 && iy >= 0); }
  };

  auto argsToIndex = [&]() {
    Index ind;

    if (args.size() < 1) {
      app->errorMsg("Missing index for " + name);
      return ind;
    }

    QStringList strs;
    (void) tcl->splitList(args[0], strs);

    if (strs.size() != 2) {
      app->errorMsg("Missing index for " + name);
      return ind;
    }

    ind.ix = Util::stringToInt(strs[0]);
    ind.iy = Util::stringToInt(strs[1]);

    return ind;
  };

  if (name == "cell_bg") {
    (void) argsToIndex();
  }
  return Object3D::getValue(name, args);
}

bool
FieldRunners3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto setTexture = [&](const QString &id, const QString &filename) {
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
  };

  if      (name == "map") {
    runners_->loadMap(value.toStdString());
  }
  else if (name.left(8) == "texture.") {
    auto id = name.mid(8);

    setTexture(id, value);
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
FieldRunners3DObj::
render()
{
  auto resizeSprites = [&](Sprites &sprites, uint n) {
    while (sprites.size() > n) {
      auto *obj = sprites.back();

      sprites.pop_back();

      canvas_->removeObject(obj);

      delete obj;
    }

    while (sprites.size() < n) {
      auto *obj = new Sprite3DObj(canvas_);

      (void) canvas_->addNewObject(obj);

      obj->init();

      sprites.push_back(obj);
    }
  };

  //---

  auto nr = runners_->getNumRows();
  auto nc = runners_->getNumCols();

  auto dx = 1.0/nc;
  auto dy = 1.0/nr;

  auto mapX = [&](int ix) { return CMathUtil::map(ix, 0, nc - 1, -1, 1); };
  auto mapY = [&](int iy) { return CMathUtil::map(iy, 0, nr - 1, 1, -1); };

  //---

  {
  auto n = uint(std::max(nr*nc, 0));

  resizeSprites(bgSprites_, n);

  int i = 0;

  for (int r = 0; r < nr; ++r) {
    auto y = mapY(r);

    for (int c = 0; c < nc; ++c, ++i) {
      auto x = mapX(c);

      CFieldRunners::FieldCell *cell;
      runners_->getCell(CFieldRunners::CellPos(r, c), &cell);

      auto *sprite = bgSprites_[i];

      sprite->setPosition(CPoint3D(x, y, 1.0));
      sprite->setScale   (dx, dy, 1.0);

      auto cellType    = CFieldRunners::CellType::EMPTY;
      auto cellSubType = CFieldRunners::CellSubType::NONE;

      if (cell) {
        cellType    = cell->type();
        cellSubType = cell->subType();
      }

      QString id;

      if      (cellType == CFieldRunners::CellType::BORDER)
        id = "border";
      else if (cellType == CFieldRunners::CellType::BLOCK)
        id = "block";
      else if (cellType == CFieldRunners::CellType::GUN)
        id = "gun";
      else if (cellType == CFieldRunners::CellType::EMPTY) {
        if      (cellSubType == CFieldRunners::CellSubType::GRASS)
          id = "grass";
        else if (cellSubType == CFieldRunners::CellSubType::STONE)
          id = "stone";
        else
          id = "grass";
      }

      auto p = textures_.find(id);

      if (p != textures_.end())
        sprite->setTexture((*p).second);
    }
  }
  }

  //---

  {
  uint nr = runners_->getNumRunners();

  resizeSprites(runnerSprites_, nr);

  for (uint i = 0; i < nr; ++i) {
    auto *runner = runners_->getRunner(i);

    const auto &pos = runner->getPos();

    auto *sprite = runnerSprites_[i];

    auto x = mapX(pos.col);
    auto y = mapY(pos.row);

    sprite->setPosition(CPoint3D(x, y, 0.5));
    sprite->setScale   (dx, dy, 1.0);

    QString id;

    if (runner->type() == CFieldRunners::RunnerType::SOLDIER)
      id = "soldier";

    auto p = textures_.find(id);

    if (p != textures_.end())
      sprite->setTexture((*p).second);
  }
  }
}
#endif

}
