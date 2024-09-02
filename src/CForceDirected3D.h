#ifndef CForceDirected3D_H
#define CForceDirected3D_H

#include <Springy3D.h>

/*!
 * \brief Force directed graph data
 */
class CForceDirected3D {
 public:
  using NodeP   = std::shared_ptr<Springy3D::Node>;
  using EdgeP   = std::shared_ptr<Springy3D::Edge>;
  using GraphP  = std::shared_ptr<Springy3D::Graph>;
  using LayoutP = std::shared_ptr<Springy3D::Layout>;
  using SpringP = std::shared_ptr<Springy3D::Spring>;
  using PointP  = std::shared_ptr<Springy3D::Point>;

 public:
  CForceDirected3D() {
    initialized_ = false;
  }

  virtual ~CForceDirected3D() { }

  double stiffness() const { return stiffness_; }
  void setStiffness(double r) { stiffness_ = r; if (layout_) layout_->setStiffness(stiffness_); }

  double repulsion() const { return repulsion_; }
  void setRepulsion(double r) { repulsion_ = r; if (layout_) layout_->setRepulsion(repulsion_); }

  double damping() const { return damping_; }
  void setDamping(double r) { damping_ = r; if (layout_) layout_->setDamping(damping_); }

  double centerAttract() const { return centerAttract_; }
  void setCenterAttract(double r) {
    centerAttract_ = r; if (layout_) layout_->setCenterAttract(centerAttract_); }

  void resetPlacement() {
    layout_->resetNodes();
  }

  void reset() {
    initialized_ = false;

    graph_  = GraphP();
    layout_ = LayoutP();
  }

  // create and add node
  NodeP newNode() {
    init();

    return graph_->newNode();
  }

  NodeP getNode(int id) {
    init();

    return graph_->getNode(id);
  }

  // create and add edge
  EdgeP newEdge(NodeP node1, NodeP node2) {
    assert(node1 && node2);

    init();

    return graph_->newEdge(node1, node2);
  }

  // add existing node
  void addNode(NodeP node) {
    init();

    graph_->addNode(node);
  }

  // add existing edge
  void addEdge(EdgeP edge) {
    init();

    graph_->addEdge(edge);
  }

  Springy3D::Edges edges() const { init(); return graph_->edges(); }
  Springy3D::Nodes nodes() const { init(); return graph_->nodes(); }

  Springy3D::Edges getEdges(NodeP node) const {
    init();

    return graph_->getEdges(node);
  }

  Springy3D::Edges getEdges(NodeP node1, NodeP node2) const {
    init();

    return graph_->getEdges(node1, node2);
  }

  double step(double dt) {
    init();

    return layout_->step(dt);
  }

  SpringP spring(EdgeP edge, bool &isTemp) {
    init();

    return layout_->edgeSpring(edge, isTemp);
  }

  PointP point(NodeP node) {
    init();

    return layout_->nodePoint(node);
  }

  Springy3D::NodePoint nearest(const Springy3D::Vector &v) const {
    init();

    return layout_->nearest(v);
  }

  NodeP currentNode() const { return currentNode_; }
  void setCurrentNode(NodeP node) { currentNode_ = node; }

  PointP currentPoint() const { return currentPoint_; }
  void setCurrentPoint(PointP point) { currentPoint_ = point; }

  void calcRange(double &xmin, double &ymin, double &zmin,
                 double &xmax, double &ymax, double &zmax) const {
    init();

    layout_->calcRange(xmin, ymin, zmin, xmax, ymax, zmax);
  }

#if 0
  void adjustRange(double &xmin, double &ymin, double &zmin,
                   double &xmax, double &ymax, double &zmax) {
    init();

    layout_->adjustRange(xmin, ymin, xmax, ymax);
  }
#endif

 protected:
  void init() const {
    if (! initialized_) {
      auto *th = const_cast<CForceDirected3D *>(this);

      th->graph_  = GraphP(makeGraph());
      th->layout_ = LayoutP(makeLayout());

      th->initialized_ = true;
    }
  }

  virtual GraphP makeGraph() const {
    return std::make_shared<Springy3D::Graph>();
  }

  virtual LayoutP makeLayout() const {
    auto *layout = new Springy3D::Layout(graph_.get(), stiffness_, repulsion_, damping_);

    layout->setCenterAttract(centerAttract_);

    return LayoutP(layout);
  }

 private:
  double  stiffness_     { 400.0 };
  double  repulsion_     { 400.0 };
  double  damping_       { 0.5 };
  double  centerAttract_ { 50.0 };
  bool    initialized_   { false };
  GraphP  graph_;
  LayoutP layout_;
  NodeP   currentNode_   { nullptr };
  PointP  currentPoint_  { nullptr };
};

#endif
