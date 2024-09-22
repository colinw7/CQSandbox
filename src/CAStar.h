#ifndef CASTAR_H
#define CASTAR_H

#include <list>
#include <map>
#include <set>

#include <cassert>
#include <iostream>

template<typename Location>
class CAStar {
 public:
  struct Node {
    Location loc;
    Node*    parent              { nullptr };
    double   costFromStart       { 0.0 };
    double   estimatedCostToGoal { 0.0 };
    double   totalCost           { 0.0 };

    Node(const Location &l) :
     loc(l) {
    }

    virtual ~Node() { }
  };

 public:
  using NodeList     = std::list<Node *>;
  using LocationList = std::list<Location>;

 public:
  CAStar() { }

  virtual ~CAStar() { }

  //--- By Location

  // do A* search from start to goal
  bool search(const Location &startLoc, const Location &goalLoc, NodeList &pathNodes);

  // smallest/optimal cost to goal
  virtual double pathCostEstimate(const Location &startLoc, const Location &goalLoc);

  // cost to move from specified location to new location
  virtual double traverseCost(Node *node, Node *newNode);

  virtual LocationList getNextLocations(Node *node) const;

  //--- By Node

  // do A* search from start to goal
  bool search(Node *startNode, Node *goalNode, NodeList &pathNodes);

  // smallest/optimal cost to goal
  virtual double pathCostEstimate(Node *startNode, Node *goalNode);

  // cost to move from specified location to new location
  virtual double traverseCost(const Location &loc, const Location &newLoc);

  virtual NodeList getNextNodes(Node *node) const;

  //------

  bool isOpenNode(Node *node) const;
  bool isClosedNode(Node *node) const;

  virtual Node *getBestOpenNode();

  virtual Node *lookupNode(const Location &loc) const;

  virtual Node *createNode(const Location &loc) const;

  //------

  void printNode(Node *node) const;

 protected:
  using NodeMap = std::map<Location,Node *>;
  using NodeSet = std::set<Node *>;

  mutable NodeMap nodeMap_; // map of location to node

  NodeSet openNodes_;   // open nodes
  NodeSet closedNodes_; // closed nodes
};

//---

template<typename Location>
bool
CAStar<Location>::
search(const Location &startLoc, const Location &goalLoc, NodeList &pathNodes)
{
  // initialize start node and goal nodes
  auto *startNode = lookupNode(startLoc);
  auto *goalNode  = lookupNode(goalLoc);

 if (! startNode || ! goalNode)
   return false;

  return search(startNode, goalNode, pathNodes);
}

template<typename Location>
bool
CAStar<Location>::
search(Node *startNode, Node *goalNode, NodeList &pathNodes)
{
  // clear node map
  nodeMap_.clear();

  nodeMap_[startNode->loc] = startNode;
  nodeMap_[goalNode ->loc] = goalNode;

  // clear open and closed
  openNodes_  .clear();
  closedNodes_.clear();

  startNode->parent              = nullptr;
  startNode->costFromStart       = 0.0;
  startNode->estimatedCostToGoal = pathCostEstimate(startNode, goalNode);
  startNode->totalCost           = startNode->estimatedCostToGoal;

  openNodes_.insert(startNode);

  // process the list until we get to the goal or fail
  while (! openNodes_.empty()) {
    // remove node from open with lowest total cost
    Node *node = getBestOpenNode();

    // if at goal then we're done
    if (node == goalNode) {
      // construct path backward from Node to startLoc
      pathNodes.push_front(node);

      while (node->parent) {
        node = node->parent;

        pathNodes.push_front(node);
      }

      return true;
    }

    // remove open node
    openNodes_.erase(node);

    // push node onto closed
    closedNodes_.insert(node);

    // get successor nodes of this node
    NodeList nextNodes = getNextNodes(node);

    typename NodeList::const_iterator pn1, pn2;

    for (pn1 = nextNodes.begin(), pn2 = nextNodes.end(); pn1 != pn2; ++pn1) {
      Node *nextNode = *pn1;

      // if closed then skip
      if (isClosedNode(nextNode))
        continue;

      // get cost to this next node
      double nextCost = node->costFromStart + traverseCost(node, nextNode);

      bool isOpen = isOpenNode(nextNode);

      // better if not already open, or already open and this is a better path to this node
      bool isBetter = (! isOpen || nextCost < nextNode->costFromStart);

      if (isBetter) {
        if (isOpen)
          openNodes_.erase(nextNode);

        nextNode->parent              = node;
        nextNode->costFromStart       = nextCost;
        nextNode->estimatedCostToGoal = pathCostEstimate(nextNode, goalNode);
        nextNode->totalCost           = nextNode->costFromStart + nextNode->estimatedCostToGoal;

        openNodes_.insert(nextNode);
      }
    }
  }

  return false;
}

template<typename Location>
bool
CAStar<Location>::
isOpenNode(Node *node) const
{
  return openNodes_.find(node) != openNodes_.end();
}

template<typename Location>
bool
CAStar<Location>::
isClosedNode(Node *node) const
{
  return closedNodes_.find(node) != closedNodes_.end();
}

template<typename Location>
typename CAStar<Location>::Node *
CAStar<Location>::
getBestOpenNode()
{
  Node   *minNode = nullptr;
  double  minCost = 0.0;

  typename NodeSet::iterator p1, p2;

  for (p1 = openNodes_.begin(), p2 = openNodes_.end(); p1 != p2; ++p1) {
    Node   *node = *p1;
    double  cost = node->totalCost;

    if (minNode == nullptr || cost < minCost) {
      minNode = node;
      minCost = cost;
    }
  }

  //printNode(minNode);

  return minNode;
}

template<typename Location>
double
CAStar<Location>::
pathCostEstimate(Node *startNode, Node *goalNode)
{
  return pathCostEstimate(startNode->loc, goalNode->loc);
}

template<typename Location>
double
CAStar<Location>::
pathCostEstimate(const Location &, const Location &)
{
  assert(false);

  return 0.0;
}

template<typename Location>
double
CAStar<Location>::
traverseCost(Node *node, Node *newNode)
{
  return traverseCost(node->loc, newNode->loc);
}

template<typename Location>
double
CAStar<Location>::
traverseCost(const Location &, const Location &)
{
  assert(false);

  return 0.0;
}

template<typename Location>
typename CAStar<Location>::NodeList
CAStar<Location>::
getNextNodes(Node *node) const
{
  NodeList nextNodes;

  LocationList nextLocations = getNextLocations(node);

  typename LocationList::const_iterator pl1, pl2;

  for (pl1 = nextLocations.begin(), pl2 = nextLocations.end(); pl1 != pl2; ++pl1) {
    const Location &nextLocation = *pl1;

    Node *nextNode = lookupNode(nextLocation);

    nextNodes.push_back(nextNode);
  }

  return nextNodes;
}

template<typename Location>
typename CAStar<Location>::LocationList
CAStar<Location>::
getNextLocations(Node *) const
{
  assert(false);

  return LocationList();
}

template<typename Location>
typename CAStar<Location>::Node *
CAStar<Location>::
lookupNode(const Location &loc) const
{
  typename NodeMap::iterator p = nodeMap_.find(loc);

  if (p != nodeMap_.end())
    return (*p).second;

  Node *node = createNode(loc);

  nodeMap_[loc] = node;

  assert(nodeMap_.find(loc) != nodeMap_.end());

  return node;
}

template<typename Location>
typename CAStar<Location>::Node *
CAStar<Location>::
createNode(const Location &loc) const
{
  return new Node(loc);
}

template<typename Location>
void
CAStar<Location>::
printNode(Node *node) const
{
  std::cout << node->loc << std::endl;
}

#endif
