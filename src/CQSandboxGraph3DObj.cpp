#include <CQSandboxGraph3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxText3DObj.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>
#include <CQGLUtil.h>

#include <CForceDirected3D.h>
#include <CDotParse.h>

namespace CQSandbox {

ShaderProgram *Graph3DObj::s_program1 = nullptr;
ShaderProgram *Graph3DObj::s_program2 = nullptr;

bool
Graph3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Graph3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Graph3DObj::
Graph3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

void
Graph3DObj::
addDemoNodes()
{
  delete forceDirected_;

  forceDirected_ = new CForceDirected3D;

  //---

  auto node1 = forceDirected_->newNode();
  auto node2 = forceDirected_->newNode();
  auto node3 = forceDirected_->newNode();
  auto node4 = forceDirected_->newNode();

  auto node5 = forceDirected_->newNode();
  auto node6 = forceDirected_->newNode();
  auto node7 = forceDirected_->newNode();
  auto node8 = forceDirected_->newNode();

  node1->setLabel("Node 1");
  node2->setLabel("Node 2");
  node3->setLabel("Node 3");
  node4->setLabel("Node 4");
  node5->setLabel("Node 5");
  node6->setLabel("Node 6");
  node7->setLabel("Node 7");
  node8->setLabel("Node 8");

  auto edge1 = forceDirected_->newEdge(node1, node2);
  auto edge2 = forceDirected_->newEdge(node2, node3);
  auto edge3 = forceDirected_->newEdge(node3, node1);
  auto edge4 = forceDirected_->newEdge(node1, node4);
  auto edge5 = forceDirected_->newEdge(node2, node4);
  auto edge6 = forceDirected_->newEdge(node3, node4);

  auto edge7  = forceDirected_->newEdge(node1, node5);
  auto edge8  = forceDirected_->newEdge(node2, node6);
  auto edge9  = forceDirected_->newEdge(node3, node7);
  auto edge10 = forceDirected_->newEdge(node4, node8);
}

void
Graph3DObj::
initSteps()
{
#if 0
  int initSteps { 1000 };

  for (int i = 0; i < initSteps; ++i)
    forceDirected_->step(stepSize_);
#endif
}

void
Graph3DObj::
init()
{
  Object3D::init();

  //---

  if (! s_program1) {
#if 0
    static const char *vertexShader1 =
      "#version 330 core\n"
      "layout (location = 0) in vec3 point;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "void main()\n"
      "{\n"
      "  gl_Position = projection * view * model * vec4(point.x, point.y, point.z, 1.0);\n"
      "}";
    static const char *geometryShader1 =
      "#version 330 core\n"
      "layout (points) in;\n"
      "layout (line_strip, max_vertices = 5) out;\n"
      "\n"
      "void main() {\n"
      "  gl_Position = gl_in[0].gl_Position + vec4(-0.01, -0.01, 0.0, 0.0);\n"
      "  EmitVertex();\n"
      "  gl_Position = gl_in[0].gl_Position + vec4( 0.01, -0.01, 0.0, 0.0);\n"
      "  EmitVertex();\n"
      "  gl_Position = gl_in[0].gl_Position + vec4( 0.01,  0.01, 0.0, 0.0);\n"
      "  EmitVertex();\n"
      "  gl_Position = gl_in[0].gl_Position + vec4(-0.01,  0.01, 0.0, 0.0);\n"
      "  EmitVertex();\n"
      "  gl_Position = gl_in[0].gl_Position + vec4(-0.01, -0.01, 0.0, 0.0);\n"
      "  EmitVertex();\n"
      "  EndPrimitive();\n"
      "}\n";
    static const char *fragmentShader1 =
      "#version 330 core\n"
      "uniform vec3 lineColor;\n"
      "void main()\n"
      "{\n"
      "  gl_FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
      "}\n";
#endif

    s_program1 = new ShaderProgram(this);

#if 0
    s_program1->addVertexCode  (vertexShader1);
    s_program1->addGeometryCode(geometryShader1);
    s_program1->addFragmentCode(fragmentShader1);
#else
    s_program1->addVertexFile  (canvas_->buildDir() + "/shaders/graph1.vs");
    s_program1->addGeometryFile(canvas_->buildDir() + "/shaders/graph1.gs");
    s_program1->addFragmentFile(canvas_->buildDir() + "/shaders/graph1.fs");
#endif

    s_program1->link();

    //---

#if 0
    static const char *vertexShader2 =
      "#version 330 core\n"
      "layout (location = 1) in vec3 line;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "void main()\n"
      "{\n"
      "  gl_Position = projection * view * model * vec4(line.x, line.y, line.z, 1.0);\n"
      "}";
    static const char *fragmentShader2 =
      "#version 330 core\n"
      "uniform vec3 lineColor;\n"
      "void main()\n"
      "{\n"
      "  gl_FragColor = vec4(lineColor.x, lineColor.y, lineColor.z, 1.0f);\n"
      "}\n";
#endif

    s_program2 = new ShaderProgram(this);

#if 0
    s_program2->addVertexCode  (vertexShader2);
    s_program2->addFragmentCode(fragmentShader2);
#else
    s_program2->addVertexFile  (canvas_->buildDir() + "/shaders/graph2.vs");
    s_program2->addFragmentFile(canvas_->buildDir() + "/shaders/graph2.fs");
#endif

    s_program2->link();
  }

  //---

  canvas_->glGenVertexArrays(1, &pointsArrayId_);

  canvas_->glGenBuffers(1, &pointsBufferId_);
  canvas_->glGenBuffers(1, &linesBufferId_);
}

QVariant
Graph3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Graph3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if (name == "dot_file") {
    (void) loadDotFile(value);
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

bool
Graph3DObj::
loadDotFile(const QString &filename)
{
  delete parse_;

  parse_ = new CDotParse::Parse(filename.toStdString());

  if (! parse_->parse())
    return false;

  //---

  delete forceDirected_;

  forceDirected_ = new CForceDirected3D;

  //---

  using NodeMap = std::map<uint, CForceDirected3D::NodeP>;

  NodeMap nodeMap;

  for (const auto &pg : parse_->graphs()) {
    const auto &graph = pg.second;

    for (const auto &pn : graph->nodes()) {
      const auto &node = pn.second;

      auto node1 = forceDirected_->newNode();

      node1->setLabel(node->name());

      nodeMap[node->id()] = node1;
    }

    for (const auto &edge : graph->edges()) {
      auto *fromNode = edge->fromNode();
      auto *toNode   = edge->toNode  ();

      const auto &node1 = nodeMap[fromNode->id()];
      const auto &node2 = nodeMap[toNode  ->id()];

      (void) forceDirected_->newEdge(node1, node2);
    }
  }

  return true;
}

void
Graph3DObj::
tick()
{
  forceDirected_->step(stepSize_);

  updatePoints();

  updateTextObjs();

  Object3D::tick();
}

void
Graph3DObj::
updatePoints()
{
  double xmin, ymin, zmin, xmax, ymax, zmax;
  forceDirected_->calcRange(xmin, ymin, zmin, xmax, ymax, zmax);

  auto mapPoint = [&](const Springy3D::Point &p) {
    auto x = CMathUtil::map(p.p().x(), xmin, xmax, -0.45, 0.45);
    auto y = CMathUtil::map(p.p().y(), ymin, ymax, -0.45, 0.45);
    auto z = CMathUtil::map(p.p().z(), zmin, zmax, -0.45, 0.45);

    return CGLVector3D(x, y, z);
  };

  if (xmax == xmin) xmax = xmin + 0.01;
  if (ymax == ymin) ymax = ymin + 0.01;
  if (zmax == zmin) zmax = zmin + 0.01;

  auto nodes = forceDirected_->nodes();

  auto nn = nodes.size();

  if (nn != points_.size())
    points_.resize(nn);

  for (size_t i = 0; i < nn; ++i) {
    auto point = forceDirected_->point(nodes[i]);

    points_[i] = mapPoint(point->p());
  }

  auto edges = forceDirected_->edges();

  auto ne = edges.size();

  if (2*ne != linePoints_.size())
    linePoints_.resize(2*ne);

  for (size_t i = 0; i < ne; ++i) {
    auto point1 = forceDirected_->point(edges[i]->source());
    auto point2 = forceDirected_->point(edges[i]->target());

    linePoints_[2*i    ] = mapPoint(point1->p());
    linePoints_[2*i + 1] = mapPoint(point2->p());
  }

  //---

  // bind the Vertex Array Object
  canvas_->glBindVertexArray(pointsArrayId_);

  //---

  // store point data in array buffer
  uint aPos = 0;
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, pointsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, nn*sizeof(CGLVector3D), &points_[0], GL_STATIC_DRAW);

  // set points attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(aPos);

  //---

  // store point data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, linesBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, 2*ne*sizeof(CGLVector3D), &linePoints_[0], GL_STATIC_DRAW);

  // set points attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(1);

  //---

  canvas_->glBindBuffer(GL_ARRAY_BUFFER, 0);
  canvas_->glBindVertexArray(0);

  //---

  updateModelMatrix();
}

void
Graph3DObj::
updateTextObjs()
{
  auto nodes = forceDirected_->nodes();

  auto nn = nodes.size();

  while (textObjs_.size() > nn) {
    auto *obj = textObjs_.back();

    textObjs_.pop_back();

    canvas_->removeObject(obj);

    delete obj;
  }

  while (textObjs_.size() < nn) {
    auto *obj = new Text3DObj(canvas_);

    (void) canvas_->addNewObject(obj);

    obj->init();

    textObjs_.push_back(obj);
  }

  int i = 0;

  for (auto &node : nodes) {
    auto *textObj = textObjs_[i];

    textObj->setText(QString::fromStdString(node->label()));

    const auto &p = points_[i];

    float x, y, z;
    modelMatrix_.multiplyPoint(p.x(), p.y(), p.z(), &x, &y, &z);

    auto p1 = CPoint3D(x, y, z);

    textObj->setPosition(p1);

    ++i;
  }
}

void
Graph3DObj::
render()
{
  s_program1->bind();

  s_program1->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program1->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  s_program1->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  canvas_->glBindVertexArray(pointsArrayId_);

  int np = points_.size();

  glDrawArrays(GL_POINTS, 0, np);
//glDrawArrays(GL_TRIANGLES, 0, np);

  s_program1->release();

  //------

  s_program2->bind();

  s_program2->setUniformValue("lineColor", Util::toVector(lineColor_));

  s_program2->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program2->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  s_program2->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  canvas_->glBindVertexArray(pointsArrayId_);

  int nl = linePoints_.size();

  glDrawArrays(GL_LINES, 0, nl);

  s_program2->release();
}

}
