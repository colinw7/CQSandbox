#ifndef CQSandboxShape3DData_H
#define CQSandboxShape3DData_H

#include <CGLVector3D.h>
#include <CGLVector2D.h>

#include <vector>

class CShape3D;

namespace CQSandbox {

class Shape3DData {
 public:
  using Points    = std::vector<CGLVector3D>;
  using Indices   = std::vector<unsigned int>;
  using TexCoords = std::vector<CGLVector2D>;

  struct VertexData {
    CGLVector3D position;
    CGLVector3D normal;
    CGLVector2D texCoord;

    VertexData(const CGLVector3D &position, const CGLVector3D &normal,
               const CGLVector2D &texCoord) :
     position(position), normal(normal), texCoord(texCoord) {
    }
  };

 public:
  Shape3DData() = default;
 ~Shape3DData();

  Shape3DData(const Shape3DData &) = delete;
  Shape3DData &operator=(const Shape3DData &) = delete;

  const Points &points() const { return points_; }
  void setPoints(const Points &points) { points_ = points; }

  const Points &normals() const { return normals_; }
  void setNormals(const Points &normals) { normals_ = normals; }

  const Indices &indices() const { return indices_; }
  void setIndices(const Indices &indices) { indices_ = indices; }

  const TexCoords &texCoords() const { return texCoords_; }
  void setTexCoords(const TexCoords &texCoords) { texCoords_ = texCoords; }

  CShape3D *geom() const { return geom_; }

  void addCone(double r, double h);
  void addCylinder(double r, double h);
  void addCube(double sx, double sy, double sz);
  void addSphere(double r);

  void addBodyRev(double *x, double *y, uint num_xy, uint num_patches);

  static void addBodyRevI(double *x, double *y, uint num_xy, uint num_patches,
                          std::vector<VertexData> &vertexDatas, std::vector<unsigned int> &indices);

 private:
  Points    points_;
  Points    normals_;
  Indices   indices_;
  TexCoords texCoords_;

  CShape3D *geom_ { nullptr };
};

}

#endif
