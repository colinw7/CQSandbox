#include <CQSandboxAStarObj.h>
#include <CQSandboxCanvas.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

Object *
AStarObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 2)
    return nullptr;

  auto *tcl = canvas->tcl();

  auto nx = Util::stringToInt(args[0]);
  auto ny = Util::stringToInt(args[1]);

  auto *obj = new AStarObj(canvas, nx, ny);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return obj;
}

AStarObj::
AStarObj(Canvas *canvas, uint nx, uint ny) :
 Object(canvas), nx_(nx), ny_(ny), searchData_(this)
{
  nodesArray_.resize(nx_);

  for (uint ix = 0; ix < nx_; ++ix) {
    nodesArray_[ix].resize(ny_);

    for (uint iy = 0; iy < ny_; ++iy)
      nodesArray_[ix][iy] = new SearchNode(CellPos(iy, ix));
  }
}

bool
AStarObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *tcl = canvas()->tcl();

  if      (name == "value") {
    uint ix, iy;

    if      (args.size() == 2) {
      ix = Util::stringToInt(args[0]);
      iy = Util::stringToInt(args[1]);
    }
    else if (args.size() == 1) {
      std::vector<int> a;
      if (! Util::stringToIntArray(tcl, args[0], a) || a.size() != 2)
        return false;

      ix = a[0];
      iy = a[1];
    }
    else
      return false;

    if (ix >= nx_ || iy >= ny_)
      return false;

    auto *node = nodesArray_[ix][iy];

    value = node->value();
  }
  else if (name == "empty") {
    uint ix, iy;

    if      (args.size() == 2) {
      ix = Util::stringToInt(args[0]);
      iy = Util::stringToInt(args[1]);
    }
    else if (args.size() == 1) {
      std::vector<int> a;
      if (! Util::stringToIntArray(tcl, args[0], a) || a.size() != 2)
        return false;

      ix = a[0];
      iy = a[1];
    }
    else
      return false;

    if (ix >= nx_ || iy >= ny_)
      return false;

    auto *node = nodesArray_[ix][iy];

    value = node->isEmpty();
  }
  else if (name == "nx") {
    value = int(nx_);
  }
  else if (name == "ny") {
    value = int(ny_);
  }
  else if (name == "next") {
    if (args.size() < 2)
      return false;

    auto from = Util::stringToPoint2D(tcl, args[0]);
    auto to   = Util::stringToPoint2D(tcl, args[1]);

    SearchData::NodeList pathNodes;
    searchData_.search(CellPos(from.y, from.x), CellPos(to.y, to.x), pathNodes);

    if (pathNodes.size() < 2)
      return false;

    auto p = pathNodes.begin();

    ++p;

    auto *node = dynamic_cast<SearchNode *>(*p);

    value = Util::point2DToString(CPoint2D(node->loc.col, node->loc.row));
  }
  else
    return Object::getValue(name, args, value);

  return true;
}

bool
AStarObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

  if      (name == "value") {
    uint ix, iy;

    if      (args.size() == 2) {
      ix = Util::stringToInt(args[0]);
      iy = Util::stringToInt(args[1]);
    }
    else if (args.size() == 1) {
      std::vector<int> a;
      if (! Util::stringToIntArray(tcl, args[0], a) || a.size() != 2)
        return false;

      ix = a[0];
      iy = a[1];
    }
    else
      return false;

    if (ix >= nx_ || iy >= ny_)
      return false;

    auto r = Util::stringToReal(value);

    auto *node = nodesArray_[ix][iy];

    node->setValue(r);
  }
  else if (name == "empty") {
    uint ix, iy;

    if      (args.size() == 2) {
      ix = Util::stringToInt(args[0]);
      iy = Util::stringToInt(args[1]);
    }
    else if (args.size() == 1) {
      std::vector<int> a;
      if (! Util::stringToIntArray(tcl, args[0], a) || a.size() != 2)
        return false;

      ix = a[0];
      iy = a[1];
    }
    else
      return false;

    if (ix >= nx_ || iy >= ny_)
      return false;

    auto b = Util::stringToBool(value);

    auto *node = nodesArray_[ix][iy];

    node->setEmpty(b);
  }
  else
    return Object::setValue(name, value, args);

  return true;
}

//---

AStarObj::SearchData::
SearchData(AStarObj *obj) :
 obj_(obj)
{
}

double
AStarObj::SearchData::
pathCostEstimate(const CellPos &startLoc, const CellPos &endLoc)
{
  double dx = abs(endLoc.col - startLoc.col);
  double dy = abs(endLoc.row - startLoc.row);

  return dx + dy;
}

double
AStarObj::SearchData::
traverseCost(const CellPos &loc, const CellPos &newLoc)
{
  double dx = abs(loc.col - newLoc.col);
  double dy = abs(loc.row - newLoc.row);

  return dx + dy;
}

AStarObj::SearchData::NodeList
AStarObj::SearchData::
getNextNodes(Node *node) const
{
  NodeList nodes;

  auto addEmptyCell = [&](const CellPos &pos) {
    auto *node = obj_->getNode(pos.row, pos.col);

    if (node->isEmpty())
      nodes.push_back(node);
  };

  //---

  auto &loc = node->loc;

  if (loc.col > 0)
    addEmptyCell(CellPos(loc.row, loc.col - 1));

  if (loc.row > 0)
    addEmptyCell(CellPos(loc.row - 1, loc.col));

  if (loc.row < int(obj_->ny()) - 1)
    addEmptyCell(CellPos(loc.row + 1, loc.col));

  if (loc.col < int(obj_->nx()) - 1)
    addEmptyCell(CellPos(loc.row, loc.col + 1));

  return nodes;
}

AStarObj::SearchData::Node *
AStarObj::SearchData::
lookupNode(const CellPos &pos) const
{
  return obj_->getNode(pos.row, pos.col);
}

}
