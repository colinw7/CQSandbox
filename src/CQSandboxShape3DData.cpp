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

// cone. Base at (0, 0), radius=r, height=h
void
Shape3DData::
addCone(double r, double h, const ConfigData &configData)
{
  uint stacks = 20;
  uint slices = 20;

  if (configData.num_xy > 0)
    stacks = uint(configData.num_xy);

  if (configData.num_patches > 0)
    slices = uint(configData.num_patches);

  //---

  std::vector<double> x, y;

  x.resize(stacks);
  y.resize(stacks);

  x[0         ] = r; y[0         ] = 0;
  x[stacks - 1] = 0; y[stacks - 1] = h;

  auto dx = (stacks > 2 ? (x[stacks - 1] - x[0])/(stacks - 1) : 0.0);
  auto dy = (stacks > 2 ? (y[stacks - 1] - y[0])/(stacks - 1) : 0.0);

  for (uint i = 1; i < stacks - 1; ++i) {
    x[i] = x[i - 1] + dx;
    y[i] = y[i - 1] + dy;
  }

  addBodyRev(&x[0], &y[0], stacks, slices);

  //---

  // cone (-r -> r, -r -> r, 0 -> h)
  delete geom_;
  geom_ = new CCone3D(r, h);

  //---

  auto np = points_.size();

#if 0
  for (uint i = 0; i < np; ++i)
    normals_[i] = dynamic_cast<CCone3D *>(geom_)->pointNormal(points_[i].point());
#endif

  //---

  if (configData.cap_type != CapType::NONE) {
    FaceData faceData;

    auto pos = faceDatas_.back().pos + faceDatas_.back().len;

    if      (configData.cap_type == CapType::NGON) {
      faceData.glMode = GL_LINE_LOOP;

      for (uint i = 0; i < slices; ++i)
        indices_.push_back(i);

      faceData.pos = pos;
      faceData.len = slices;

      faceDatas_.push_back(faceData);
    }
    else if (configData.cap_type == CapType::FAN) {
      faceData.glMode = GL_TRIANGLES;

      int ind1 = np;

      points_ .push_back(CVector3D(0,  0, 0));
      normals_.push_back(CVector3D(0, -1, 0));

      for (uint i = 0; i < slices; ++i) {
        uint i1 = (i + 1) % slices;

        indices_.push_back(i   );
        indices_.push_back(ind1);
        indices_.push_back(i1  );

        faceData.pos = pos;
        faceData.len = 3;

        faceDatas_.push_back(faceData);

        pos += faceData.len;
      }
    }
  }
}

// cylinder. Base at (0, 0), radius=r, height=h
void
Shape3DData::
addCylinder(double r, double h, const ConfigData &configData)
{
  uint stacks = 20;
  uint slices = 20;

  if (configData.num_xy > 0)
    stacks = uint(configData.num_xy);

  if (configData.num_patches > 0)
    slices = uint(configData.num_patches);

  //---

  // add cylinder (no end caps)
  std::vector<double> x, y;

  x.resize(stacks);
  y.resize(stacks);

  x[0         ] = r; y[0         ] = 0;
  x[stacks - 1] = r; y[stacks - 1] = h;

  auto dx = (stacks > 2 ? (x[stacks - 1] - x[0])/(stacks - 1) : 0.0); // TODO: always 0
  auto dy = (stacks > 2 ? (y[stacks - 1] - y[0])/(stacks - 1) : 0.0);

  for (uint i = 1; i < stacks - 1; ++i) {
    x[i] = x[i - 1] + dx;
    y[i] = y[i - 1] + dy;
  }

  addBodyRev(&x[0], &y[0], stacks, slices);

  //---

  if (configData.cap_type != CapType::NONE) {
    FaceData faceData;

    auto pos = faceDatas_.back().pos + faceDatas_.back().len;

    auto np = points_.size();

    if      (configData.cap_type == CapType::NGON) {
      faceData.glMode = GL_LINE_LOOP;

      for (uint i = 0; i < slices; ++i)
        indices_.push_back(i);

      faceData.pos = pos;
      faceData.len = slices;

      faceDatas_.push_back(faceData);

      pos += faceData.len;

      int pi = np - slices;

      for (uint i = 0; i < slices; ++i)
        indices_.push_back(pi + i);

      faceData.pos = pos;
      faceData.len = slices;

      faceDatas_.push_back(faceData);

      pos += faceData.len;
    }
    else if (configData.cap_type == CapType::FAN) {
      faceData.glMode = GL_TRIANGLES;

      int ind1 = np;
      int ind2 = np + 1;

      points_.push_back(CVector3D(0, 0, 0));
      points_.push_back(CVector3D(0, h, 0));

      normals_.push_back(CVector3D(0, -1, 0));
      normals_.push_back(CVector3D(0,  1, 0));

      for (uint i = 0; i < slices; ++i) {
        uint i1 = (i + 1) % slices;

        indices_.push_back(i   );
        indices_.push_back(ind1);
        indices_.push_back(i1  );

        faceData.pos = pos;
        faceData.len = 3;

        faceDatas_.push_back(faceData);

        pos += faceData.len;
      }

      int pi = np - slices;

      for (uint i = 0; i < slices; ++i) {
        uint i1 = (i + 1) % slices;

        indices_.push_back(pi + i1);
        indices_.push_back(ind2   );
        indices_.push_back(pi + i );

        faceData.pos = pos;
        faceData.len = 3;

        faceDatas_.push_back(faceData);

        pos += faceData.len;
      }
    }
  }

  //---

  delete geom_;
  geom_ = new CCylinder3D(r);
}

void
Shape3DData::
addCube(double sx, double sy, double sz)
{
  // 6 faces
  static CVector3D cube_normal[6] = {
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

  // 8 vertices
  std::vector<CPoint3D> points; points.resize(8);

  double xs = sx/2.0;
  double ys = sy/2.0;
  double zs = sz/2.0;

  points[0].x = points[1].x = points[2].x = points[3].x = -xs;
  points[4].x = points[5].x = points[6].x = points[7].x =  xs;
  points[0].y = points[1].y = points[4].y = points[5].y = -ys;
  points[2].y = points[3].y = points[6].y = points[7].y =  ys;
  points[0].z = points[3].z = points[4].z = points[7].z = -zs;
  points[1].z = points[2].z = points[5].z = points[6].z =  zs;

  points_.resize(points.size());

  for (uint i = 0; i < points.size(); ++i)
    points_[i] = points[i];

  //---

  indices_.resize(36);

  faceDatas_.clear();

  int k = 0;

  for (int i = 5; i >= 0; --i) {
    const auto &normal = cube_normal[i];

    auto addFaceInd = [&](int ind) {
      indices_[k++] = ind;
    };

    FaceData faceData;

    faceData.pos    = k;
    faceData.len    = 3;
    faceData.normal = normal;

    addFaceInd(cube_faces[i][0]);
    addFaceInd(cube_faces[i][1]);
    addFaceInd(cube_faces[i][2]);

    faceDatas_.push_back(faceData);

    faceData.pos    = k;
    faceData.len    = 3;
    faceData.normal = normal;

    addFaceInd(cube_faces[i][2]);
    addFaceInd(cube_faces[i][3]);
    addFaceInd(cube_faces[i][0]);

    faceDatas_.push_back(faceData);
  }

  //---

  // 12 edges
  lineDatas_.clear();

  auto addLine = [&](int i1, int i2) {
    int ii1 = indices_.size();
    indices_.push_back(i1);

    int ii2 = indices_.size();
    indices_.push_back(i2);

    LineData lineData;

    lineData.ind1 = ii1;
    lineData.ind2 = ii2;

    lineDatas_.push_back(lineData);
  };

  addLine(0, 1); addLine(1, 2); addLine(2, 3); addLine(3, 0);
  addLine(0, 4); addLine(1, 5); addLine(2, 6); addLine(3, 7);
  addLine(4, 5); addLine(5, 6); addLine(6, 7); addLine(7, 4);

  //---

  pointDatas_.clear();

  for (uint i = 0; i < points_.size(); ++i) {
    PointData pointData;

    pointData.ind = i;

    pointDatas_.push_back(pointData);
  }

  //---

  delete geom_;
  geom_ = new CBox3D(sx, sy, sz);
}

void
Shape3DData::
addSphere(double radius, const ConfigData &configData)
{
  uint stacks = 20;
  uint slices = 20;

  if (configData.num_xy > 0)
    stacks = uint(configData.num_xy);

  if (configData.num_patches > 0)
    slices = uint(configData.num_patches);

  //---

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
addTorus(double r0, double r1, double power1, double power2, const ConfigData &configData)
{
  uint stacks = 20;
  uint slices = 20;

  if (configData.num_xy > 0)
    stacks = uint(configData.num_xy);

  if (configData.num_patches > 0)
    slices = uint(configData.num_patches);

  //---

  resetShape();

  //---

  double xc = 0.0;
  double yc = 0.0;
  double zc = 0.0;

  auto power = [](double f, double p) {
    double absf = std::abs(f);

    if (absf < 0.00001)
      return 0.0;

    double pp = std::pow(absf, p);

    if (f < 0)
      return -pp;
    else
      return pp;
  };

  auto findPoint = [&](const CPoint3D &p, uint &vind) {
    uint i = 0;

    for (auto &p1 : points_) {
      if (p == p1.point()) {
        vind = i;
        return true;
      }

      ++i;
    }

    return false;
  };

  auto addPoint = [&](const CPoint3D &p) {
    points_.push_back(CVector3D(p));
    normals_.push_back(CVector3D(0, 1, 0));
    return points_.size() - 1;
  };

  // rotate circle of radius r1 at radius r0 around center (xc, yc, zc)
  double du = 360.0/stacks;
  double dv = 360.0/slices;

  auto u = 0.0;

  for (uint iu = 0; iu < stacks; ++iu) {
    auto v = 0.0;

    for (uint iv = 0; iv < slices; ++iv) {
      auto addUV = [&](double u1, double v1) {
        auto theta  = CMathGen::DegToRad(u1);
        auto ctheta = power(std::cos(theta), power1);
        auto stheta = power(std::sin(theta), power1);

        auto phi  = CMathGen::DegToRad(v1);
        auto cphi = power(std::cos(phi), power2);
        auto sphi = power(std::sin(phi), power2);

        auto x = ctheta*(r0 + r1*cphi);
        auto y = stheta*(r0 + r1*cphi);
        auto z = r1*sphi;

        auto point1 = CPoint3D(xc + x, yc + y, zc + z);

        uint vind;
        if (! findPoint(point1, vind)) {
          vind = addPoint(point1);

          auto x1 = ctheta*r0;
          auto y1 = stheta*r0;
          auto z1 = 0.0;

          auto n = CVector3D(CPoint3D(xc + x1, yc + y1, zc + z1), point1);

          normals_[vind] = n;

          PointData pointData;

          pointData.ind = vind;

          pointDatas_.push_back(pointData);
        }

        return vind;
      };

      //----

      auto vertex1 = addUV(u     , v     );
      auto vertex2 = addUV(u + du, v     );
      auto vertex3 = addUV(u + du, v + dv);
      auto vertex4 = addUV(u     , v + dv);

      //----

      FaceData faceData;
      LineData lineData;

      faceData.len = 3;

      //---

      faceData.pos = indices_.size();

      faceDatas_.push_back(faceData);

      indices_.push_back(vertex1);
      indices_.push_back(vertex2);
      indices_.push_back(vertex3);

      //---

      // 1->2
      lineData.ind1 = faceData.pos;
      lineData.ind2 = lineData.ind1 + 1;

      lineDatas_.push_back(lineData);

      // 2->3
      lineData.ind1 = faceData.pos + 1;
      lineData.ind2 = lineData.ind1 + 1;

      lineDatas_.push_back(lineData);

      //---

      faceData.pos = indices_.size();

      faceDatas_.push_back(faceData);

      indices_.push_back(vertex3);
      indices_.push_back(vertex4);
      indices_.push_back(vertex1);

      //---

      // 3->4
      lineData.ind1 = faceData.pos;
      lineData.ind2 = lineData.ind1 + 1;

      lineDatas_.push_back(lineData);

      // 1->4
      indices_.push_back(vertex4);

      lineData.ind1 = faceData.pos + 2;
      lineData.ind2 = lineData.ind1 + 1;

      lineDatas_.push_back(lineData);

      //---

      v += dv;
    }

    u += du;
  }
}

void
Shape3DData::
resetShape()
{
  points_   .clear();
  normals_  .clear();
  indices_  .clear();
  texCoords_.clear();

  faceDatas_ .clear();
  lineDatas_ .clear();
  pointDatas_.clear();
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

  faceDatas_.clear();

  for (uint i = 0; i < np; ++i) {
    points_ [i] = vertices[i].position;
    normals_[i] = vertices[i].normal;
  }

  for (uint i = 0; i < ni; ++i) {
    indices_[i] = indices[i];
  }

  for (uint i = 0; i < ni; i += 3) {
    FaceData faceData;

    faceData.glMode = GL_TRIANGLES;

    faceData.pos = i;
    faceData.len = 3;

    faceDatas_.push_back(faceData);
  }

  //---

  pointDatas_.clear();

  for (uint i = 0; i < np; ++i) {
    PointData pointData;

    pointData.ind = i;

    pointDatas_.push_back(pointData);
  }

  //---

  lineDatas_.clear();

  auto addLine = [&](int i1, int i2) {
    int ii1 = indices_.size();
    indices_.push_back(i1);

    int ii2 = indices_.size();
    indices_.push_back(i2);

    LineData lineData;

    lineData.ind1 = ii1;
    lineData.ind2 = ii2;

    lineDatas_.push_back(lineData);
  };

  for (uint j = 0; j < num_patches; ++j) {
    auto j1 = j*num_xy;

    for (uint i1 = num_xy - 1, i2 = 0; i2 < num_xy; i1 = i2++) {
      addLine(i1 + j1, i2 + j1);
    }

    if (j < num_patches - 1) {
      auto j2 = (j + 1)*num_xy;

      for (uint i = 0; i < num_xy; ++i) {
        addLine(i + j1, i + j2);
      }
    }
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
    c[i] = std::cos(theta);
    s[i] = std::sin(theta);

    theta += theta_increment;
  }

  uint num_vertices = 0;

  std::vector<uint> index1, index2;

  index1.resize(num_patches + 1);
  index2.resize(num_patches + 1);

  uint *pindex1 = &index1[0];
  uint *pindex2 = &index2[0];

  if (fabs(x[0]) < 1E-6) {
    CVector3D p(0.0, y[0], 0.0);

    auto n = p;

    vertexDatas.push_back(VertexData(p, n, CVector2D(0, 0)));

    for (uint i = 0; i <= num_patches; i++)
      pindex1[i] = num_vertices;

    num_vertices++;
  }
  else {
    for (uint i = 0; i < num_patches; i++) {
      CVector3D p(x[0]*c[i], y[0], -x[0]*s[i]);

      //auto n = CVector3D(p.x(), 0, p.z());
      auto n = p;

      vertexDatas.push_back(VertexData(p, n, CVector2D(0, 0)));

      pindex1[i] = num_vertices;

      num_vertices++;
    }

    pindex1[num_patches] = pindex1[0];
  }

  for (uint j = 1; j < num_xy; j++) {
    double ty = (num_xy > 1 ? 1.0*j/(num_xy - 1) : 0.0);

    if (fabs(x[j]) < 1E-6) {
      CVector3D p(0.0, y[j], 0.0);

      auto n = p;

      vertexDatas.push_back(VertexData(p, n, CVector2D(0, ty)));

      for (uint i = 0; i <= num_patches; ++i)
        pindex2[i] = num_vertices;

      ++num_vertices;
    }
    else {
      for (uint i = 0; i < num_patches; i++) {
        double tx = (num_patches > 1 ? 1.0*i/(num_patches - 1) : 0.0);

        CVector3D p(x[j]*c[i], y[j], -x[j]*s[i]);

        //auto n = CVector3D(p.x(), 0, p.z());
        auto n = p;

        vertexDatas.push_back(VertexData(p, n, CVector2D(tx, ty)));

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

void
Shape3DData::
transform(const CMatrix3DH &m)
{
  auto nm = m.inverse().transposed();

  for (auto &p : points_)
    p = m*p;

  for (auto &n : normals_)
    n = (nm*n).normalize();
}

void
Shape3DData::
setBuffer(CQGLBuffer *buffer)
{
  for (auto &faceData : faceDatas_)
    faceData.buffer = buffer;

  for (auto &lineData : lineDatas_)
    lineData.buffer = buffer;

  for (auto &pointData : pointDatas_)
    pointData.buffer = buffer;
}

}
