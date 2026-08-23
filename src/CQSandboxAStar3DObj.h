#ifndef CQSandboxAStar3DObj_H
#define CQSandboxAStar3DObj_H

#include <CQSandboxObject3D.h>

#include <CAStar.h>

class CQAStarModel;

namespace CQSandbox {

class AStar3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  AStar3DObj(Canvas3D *canvas, uint nx, uint ny);

  const char *typeName() const override { return "array"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  //---

  void init() override;

  //---

  uint nx() const { return nx_; }
  uint ny() const { return ny_; }

 protected:
  struct CellPos {
    int row { -1 };
    int col { -1 };

    CellPos() { }

    CellPos(int row1, int col1) :
     row(row1), col(col1) {
    }

    // equality
    bool operator==(const CellPos &pos) const {
      return (row == pos.row && col == pos.col);
    }

    // inequality
    bool operator!=(const CellPos &pos) const {
      return ! operator==(pos);
    }

    // less than (for sort)
    bool operator<(const CellPos &pos) const {
      return (row < pos.row || (row == pos.row && col < pos.col));
    }

    // print (for debug)
    friend std::ostream &operator<<(std::ostream &os, const CellPos &pos) {
      os << "(" << pos.row << "," << pos.col << ")";

      return os;
    }
  };

  class SearchData : public CAStar<CellPos> {
   protected:
    using AStar = CAStar<CellPos>;

   public:
    SearchData(AStar3DObj *obj);

    // smallest/optimal cost to goal
    double pathCostEstimate(const CellPos &startLoc, const CellPos &goalLoc) override;

    double traverseCost(const CellPos &pos, const CellPos &newPos) override;

    NodeList getNextNodes(Node *node) const override;

    Node *lookupNode(const CellPos &loc) const override;

   private:
    AStar3DObj* obj_ { nullptr };
  };

  class SearchNode : public CAStar<CellPos>::Node {
   public:
    SearchNode(const CellPos &pos) :
     CAStar<CellPos>::Node(pos) {
    }

    bool isEmpty() const { return empty_; }
    void setEmpty(bool b) { empty_ = b; }

    int value() const { return value_; }
    void setValue(int i) { value_ = i; }

   private:
    double value_ { 0 };
    bool   empty_ { true };
  };

 private:
  friend class SearchData;

  SearchNode *getNode(uint x, uint y) const {
    return nodesArray_[y][x];
  }

 private:
  using Nodes      = std::vector<SearchNode *>;
  using NodesArray = std::vector<Nodes>;

  uint       nx_ { 1 };
  uint       ny_ { 1 };
  SearchData searchData_;
  NodesArray nodesArray_;
};

}

#endif
