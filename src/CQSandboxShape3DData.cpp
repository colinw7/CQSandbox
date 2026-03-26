#include <CQSandboxShape3DData.h>

#include <CCone3D.h>
#include <CCylinder3D.h>
#include <CShape3D.h>
#include <CSphere3D.h>
#include <CBox3D.h>

#include <GL/gl.h>

namespace CQSandbox {

Shape3DData::
~Shape3DData()
{
  delete geom_;
}

void
Shape3DData::
addCone(double r, double h)
{
  uint stacks = 20;
  uint slices = 20;

  std::vector<double> x, y;

  x.resize(stacks);
  y.resize(stacks);

  x[0         ] = r; y[0         ] = 0;
  x[stacks - 1] = 0; y[stacks - 1] = h;

  double dx = stacks > 2 ? (x[stacks - 1] - x[0])/(stacks - 1) : 0;
  double dy = stacks > 2 ? (y[stacks - 1] - y[0])/(stacks - 1) : 0;

  for (uint i = 1; i < stacks - 1; ++i) {
    x[i] = x[i - 1] + dx;
    y[i] = y[i - 1] + dy;
  }

  addBodyRev(&x[0], &y[0], stacks, slices);

  delete geom_;
  geom_ = new CCone3D(r);
}

void
Shape3DData::
addCylinder(double r, double h)
{
  uint stacks = 20;
  uint slices = 20;

  std::vector<double> x, y;

  x.resize(stacks);
  y.resize(stacks);

  x[0         ] = r; y[0         ] = 0;
  x[stacks - 1] = r; y[stacks - 1] = h;

  double dx = stacks > 2 ? (x[stacks - 1] - x[0])/(stacks - 1) : 0;
  double dy = stacks > 2 ? (y[stacks - 1] - y[0])/(stacks - 1) : 0;

  for (uint i = 1; i < stacks - 1; ++i) {
    x[i] = x[i - 1] + dx;
    y[i] = y[i - 1] + dy;
  }

  addBodyRev(&x[0], &y[0], stacks, slices);

  delete geom_;
  geom_ = new CCylinder3D(r);
}

void
Shape3DData::
addCube(double sx, double sy, double sz)
{
  static CGLVector3D cube_normal[6] = {
    {-1.0,  0.0,  0.0},
    { 0.0,  1.0,  0.0},
    { 1.0,  0.0,  0.0},
    { 0.0, -1.0,  0.0},
    { 0.0,  0.0,  1.0},
    { 0.0,  0.0, -1.0}
  };

  static GLint cube_faces[6][4] = {
    {0, 1, 2, 3},
    {3, 2, 6, 7},
    {7, 6, 5, 4},
    {4, 5, 1, 0},
    {5, 6, 2, 1},
    {7, 4, 0, 3}
  };

  double xs = sx/2.0;
  double ys = sy/2.0;
  double zs = sz/2.0;

  struct Point {
    float x;
    float y;
    float z;
  };

  std::vector<Point> v; v.resize(8);

  v[0].x = v[1].x = v[2].x = v[3].x = -xs;
  v[4].x = v[5].x = v[6].x = v[7].x =  xs;
  v[0].y = v[1].y = v[4].y = v[5].y = -ys;
  v[2].y = v[3].y = v[6].y = v[7].y =  ys;
  v[0].z = v[3].z = v[4].z = v[7].z = -zs;
  v[1].z = v[2].z = v[5].z = v[6].z =  zs;

  points_   .resize(36);
  texCoords_.resize(36);
  normals_  .resize(36);

  int k = 0;

  for (int i = 5; i >= 0; --i) {
    const auto &normal = cube_normal[i];

    auto addPoint = [&](const Point &p, const CGLVector2D &t) {
      points_   [k] = CGLVector3D(p.x, p.y, p.z);
      texCoords_[k] = t;
      normals_  [k] = normal;
      ++k;
    };

    addPoint(v[cube_faces[i][0]], CGLVector2D(0.0, 0.0));
    addPoint(v[cube_faces[i][1]], CGLVector2D(1.0, 0.0));
    addPoint(v[cube_faces[i][2]], CGLVector2D(1.0, 1.0));

    addPoint(v[cube_faces[i][2]], CGLVector2D(1.0, 1.0));
    addPoint(v[cube_faces[i][3]], CGLVector2D(0.0, 1.0));
    addPoint(v[cube_faces[i][0]], CGLVector2D(0.0, 0.0));
  }

  delete geom_;
  geom_ = new CBox3D(sx, sy, sz);
}

void
Shape3DData::
addSphere(double radius)
{
  uint stacks = 20;
  uint slices = 20;

  std::vector<double> x, y;

  x.resize(stacks);
  y.resize(stacks);

  double a = -M_PI*0.5;

  double da = M_PI/(stacks - 1);

  for (uint i = 0; i < stacks; ++i) {
    x[i] = radius*std::cos(a);
    y[i] = radius*std::sin(a);

    a += da;
  }

  addBodyRev(&x[0], &y[0], stacks, slices);

  delete geom_;
  geom_ = new CSphere3D(radius);
}

void
Shape3DData::
addBodyRev(double *x, double *y, uint num_xy, uint num_patches)
{
  std::vector<VertexData>   vertices;
  std::vector<unsigned int> indices;

  addBodyRevI(&x[0], &y[0], num_xy, num_patches, vertices, indices);

  auto np = vertices.size();
  auto ni = indices.size();

  points_ .resize(np);
  normals_.resize(np);
  indices_.resize(ni);

  for (uint i = 0; i < np; ++i) {
    points_ [i] = vertices[i].position;
    normals_[i] = vertices[i].normal;
  }

  for (uint i = 0; i < ni; ++i) {
    indices_[i] = indices[i];
  }
}

void
Shape3DData::
addBodyRevI(double *x, double *y, uint num_xy, uint num_patches,
            std::vector<VertexData> &vertexDatas, std::vector<unsigned int> &indices)
{
  std::vector<double> c, s;

  c.resize(num_patches);
  s.resize(num_patches);

  double theta           = 0.0;
  double theta_increment = 2.0*M_PI/num_patches;

  for (uint i = 0; i < num_patches; i++) {
    c[i] = cos(theta);
    s[i] = sin(theta);

    theta += theta_increment;
  }

  uint num_vertices = 0;

  std::vector<uint> index1, index2;

  index1.resize(num_patches + 1);
  index2.resize(num_patches + 1);

  uint *pindex1 = &index1[0];
  uint *pindex2 = &index2[0];

  if (fabs(x[0]) < 1E-6) {
    CGLVector3D p(0.0, float(y[0]), 0.0);

    vertexDatas.push_back(VertexData(p, p, CGLVector2D(0, 0)));

    for (uint i = 0; i <= num_patches; i++)
      pindex1[i] = num_vertices;

    num_vertices++;
  }
  else {
    for (uint i = 0; i < num_patches; i++) {
      CGLVector3D p(float(x[0]*c[i]), float(y[0]), float(-x[0]*s[i]));

      vertexDatas.push_back(VertexData(p, p, CGLVector2D(0, 0)));

      pindex1[i] = num_vertices;

      num_vertices++;
    }

    pindex1[num_patches] = pindex1[0];
  }

  for (uint j = 1; j < num_xy; j++) {
    double ty = (num_xy > 1 ? 1.0*j/(num_xy - 1) : 0.0);

    if (fabs(x[j]) < 1E-6) {
      CGLVector3D p(0.0, float(y[j]), 0.0);

      vertexDatas.push_back(VertexData(p, p, CGLVector2D(0, float(ty))));

      for (uint i = 0; i <= num_patches; ++i)
        pindex2[i] = num_vertices;

      ++num_vertices;
    }
    else {
      for (uint i = 0; i < num_patches; i++) {
        double tx = (num_patches > 1 ? 1.0*i/(num_patches - 1) : 0.0);

        CGLVector3D p(float(x[j]*c[i]), float(y[j]), float(-x[j]*s[i]));

        vertexDatas.push_back(VertexData(p, p, CGLVector2D(float(tx), float(ty))));

        pindex2[i] = num_vertices;

        num_vertices++;
      }

      pindex2[num_patches] = pindex2[0];
    }

    if (pindex1[0] != pindex1[1]) {
      if (pindex2[0] == pindex2[1]) {
        for (uint i = 0; i < num_patches; ++i) {
          indices.push_back(GLushort(pindex1[i + 1]));
          indices.push_back(GLushort(pindex2[i    ]));
          indices.push_back(GLushort(pindex1[i    ]));
        }
      }
      else {
        for (uint i = 0; i < num_patches; ++i) {
          indices.push_back(GLushort(pindex1[i + 1]));
          indices.push_back(GLushort(pindex2[i + 1]));
          indices.push_back(GLushort(pindex2[i    ]));

          indices.push_back(GLushort(pindex1[i + 1]));
          indices.push_back(GLushort(pindex2[i    ]));
          indices.push_back(GLushort(pindex1[i    ]));
        }
      }
    }
    else {
      if (pindex2[0] != pindex2[1]) {
        for (uint i = 0; i < num_patches; ++i) {
          indices.push_back(GLushort(pindex2[i + 1]));
          indices.push_back(GLushort(pindex2[i    ]));
          indices.push_back(GLushort(pindex1[i    ]));
        }
      }
    }

    uint *pindex = pindex2;

    pindex2 = pindex1;
    pindex1 = pindex;
  }
}

}
