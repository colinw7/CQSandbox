#include <CQSandboxText3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLUtil.h>
#include <CQGLState.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#include <fstream>

namespace CQSandbox {

struct FontData {
  uint32_t                            size = 40;
  uint32_t                            atlasWidth = 1024;
  uint32_t                            atlasHeight = 1024;
  uint32_t                            oversampleX = 2;
  uint32_t                            oversampleY = 2;
  uint32_t                            firstChar = ' ';
  uint32_t                            charCount = '~' - ' ';
  std::unique_ptr<stbtt_packedchar[]> charInfo;
  GLuint                              texture = 0;
};

Text3DObj::TextShaderProgram* Text3DObj::s_program  = nullptr;
FontData*                     Text3DObj::s_fontData = nullptr;

Object3D *
Text3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Text3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Text3DObj::
Text3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::TEXT)
{
  setCullFace (false);
  setFrontFace(false);
}

void
Text3DObj::
init()
{
  Object3D::init();

  //---

  initShader();

  (void) initFont();
}

void
Text3DObj::
initShader()
{
  if (! s_program) {
    auto *app = canvas_->app();

    s_program = new TextShaderProgram(this);

    s_program->addVertexFile  (app->buildDir() + "/shaders/text.vs");
    s_program->addFragmentFile(app->buildDir() + "/shaders/text.fs");

    s_program->link();

    //---

    s_program->posAttr = s_program->attributeLocation("position");
    Q_ASSERT(s_program->posAttr != -1);

    s_program->colAttr = s_program->attributeLocation("color");
    Q_ASSERT(s_program->colAttr != -1);

    s_program->texPosAttr = s_program->attributeLocation("texCoord0");
    Q_ASSERT(s_program->texPosAttr != -1);

    s_program->setProjectionUniform();
    s_program->setViewUniform();

    s_program->textureUniform = s_program->uniformLocation("textureId");
    Q_ASSERT(s_program->textureUniform != -1);
  }
}

bool
Text3DObj::
initFont()
{
  if (s_fontData)
    return true;

  s_fontData = new FontData;

  //---

  auto *app = canvas_->app();

  auto name = QString("OpenSans-Regular.ttf");

  auto path = app->buildDir() + "/fonts/" + name;

  std::vector<uint8_t> fontData;
  if (! readFile(path.toLatin1().constData(), fontData))
    return false;

  auto atlasData = std::make_unique<uint8_t[]>(s_fontData->atlasWidth*s_fontData->atlasHeight);

  s_fontData->charInfo = std::make_unique<stbtt_packedchar[]>(s_fontData->charCount);

  stbtt_pack_context context;
  if (! stbtt_PackBegin(&context, atlasData.get(), s_fontData->atlasWidth,
                        s_fontData->atlasHeight, 0, 1, nullptr))
    assert(false);

  stbtt_PackSetOversampling(&context, s_fontData->oversampleX, s_fontData->oversampleY);

  if (! stbtt_PackFontRange(&context, fontData.data(), 0, s_fontData->size, s_fontData->firstChar,
                            s_fontData->charCount, s_fontData->charInfo.get()))
    assert(false);

  stbtt_PackEnd(&context);

  //---

  // allocate texture id
  glGenTextures(1, &s_fontData->texture);
  //if (! CQGLStateInst->checkError("glGenTextures")) return false;

  // set texture type
  glBindTexture(GL_TEXTURE_2D, s_fontData->texture);
  //if (! CQGLStateInst->checkError("glBindTexture")) return false;

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s_fontData->atlasWidth, s_fontData->atlasHeight, 0,
               GL_RED, GL_UNSIGNED_BYTE, atlasData.get());

  glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

  canvas_->glGenerateMipmap(GL_TEXTURE_2D);

  return true;
}

bool
Text3DObj::
readFile(const char *path, std::vector<uint8_t> &bytes) const
{
  std::ifstream file(path, std::ios::binary | std::ios::ate);

  if (! file.is_open())
    return false;

  const auto size = file.tellg();

  file.seekg(0, std::ios::beg);
  bytes = std::vector<uint8_t>(size);
  file.read(reinterpret_cast<char *>(&bytes[0]), size);
  file.close();

  return true;
}

bool
Text3DObj::
bindTexture()
{
  CQGLStateInst->setEnableTexture(true);

  glBindTexture(GL_TEXTURE_2D, textureId());
  //if (! CQGLStateInst->checkError("glBindTexture")) return false;

#if 0
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
#else
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
  //if (! CQGLStateInst->checkError("glTexParameteri")) return false;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  //if (! CQGLStateInst->checkError("glTexParameteri")) return false;

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
  //if (! CQGLStateInst->checkError("glTexParameterf")) return false;

  CQGLStateInst->setActiveTextureNum(0, true);

  return true;
}

int
Text3DObj::
textureId() const
{
  return s_fontData->texture;
}

//---

void
Text3DObj::
setText(const QString &text)
{
  text_ = text;

  textDataValid_  = false;
}

bool
Text3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if (name == "text") {
    value = text();
  }
  else if (name == "align") {
    value = Util::alignToString(align_);
  }
  else
    return Object3D::getValue(name, args, value);

  return true;
}

bool
Text3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas_->tcl();

  if      (name == "text") {
    setText(value);
  }
  else if (name == "size") {
    double r;
    if (! Util::stringToReal(value, r))
      return false;

    setSize(r);
  }
  else if (name == "color") {
    CGLColor c;
    if (! Util::stringToGLColor(tcl, value, c))
      return false;

    setColor(c);
  }
  else if (name == "align") {
    align_ = Util::stringToAlign(value);
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Text3DObj::
updateTextData()
{
  vertices_.clear();
  uvs_     .clear();
  indexes_ .clear();
  colors_  .clear();

  const auto &color = this->color();

  uint16_t lastIndex = 0;

  float offsetX = 0, offsetY = 0;

  double f = 1.0/s_fontData->size;

  for (const auto &c : text_) {
    const auto glyphInfo = makeGlyphInfo(c.toLatin1(), offsetX, offsetY);

    offsetX = glyphInfo.offsetX;
    offsetY = glyphInfo.offsetY;

    auto addPos = [&](int i) {
      // x, y, z
      auto pos = glyphInfo.positions[i];

      pos.scaleX(f);
      pos.scaleY(f);

      vertices_.emplace_back(pos);

      // u, v
      uvs_.emplace_back(glyphInfo.uvs[i]);

      // color
      colors_.push_back(color);
    };

    addPos(0);
    addPos(1);
    addPos(2);
    addPos(3);

    //---

    indexes_.push_back(lastIndex);
    indexes_.push_back(lastIndex + 1);
    indexes_.push_back(lastIndex + 2);
    indexes_.push_back(lastIndex);
    indexes_.push_back(lastIndex + 2);
    indexes_.push_back(lastIndex + 3);

    lastIndex += 4;
  }

  glData_.indexElementCount = indexes_.size();
}

void
Text3DObj::
initGLData()
{
  if (! glData_.vao)
    canvas_->glGenVertexArrays(1, &glData_.vao);

  if (! glData_.vertexBuffer)
    canvas_->glGenBuffers(1, &glData_.vertexBuffer);

  if (! glData_.uvBuffer)
    canvas_->glGenBuffers(1, &glData_.uvBuffer);

  if (! glData_.colBuffer)
    canvas_->glGenBuffers(1, &glData_.colBuffer);

  if (! glData_.indexBuffer)
    canvas_->glGenBuffers(1, &glData_.indexBuffer);
}

Text3DObj::GlyphInfo
Text3DObj::
makeGlyphInfo(uint32_t character, float offsetX, float offsetY) const
{
  stbtt_aligned_quad quad;

  stbtt_GetPackedQuad(s_fontData->charInfo.get(), s_fontData->atlasWidth, s_fontData->atlasHeight,
                      character - s_fontData->firstChar, &offsetX, &offsetY, &quad, 1);

  const auto xmin =  quad.x0;
  const auto ymin = -quad.y1;
  const auto xmax =  quad.x1;
  const auto ymax = -quad.y0;

  GlyphInfo info{};

  info.offsetX = offsetX;
  info.offsetY = offsetY;

  info.positions[0] = CGLVector3D(xmin, ymin, 0.0f);
  info.positions[1] = CGLVector3D(xmin, ymax, 0.0f);
  info.positions[2] = CGLVector3D(xmax, ymax, 0.0f);
  info.positions[3] = CGLVector3D(xmax, ymin, 0.0f);

  info.uvs[0] = CGLVector2D(quad.s0, quad.t1);
  info.uvs[1] = CGLVector2D(quad.s0, quad.t0);
  info.uvs[2] = CGLVector2D(quad.s1, quad.t0);
  info.uvs[3] = CGLVector2D(quad.s1, quad.t1);

  return info;
}

void
Text3DObj::
render()
{
  if (! textDataValid_) {
    updateTextData();

    initGLData();

    textDataValid_ = true;
  }

  //---

  auto oldFrontFace = CQGLStateInst->setFrontFaceFlag(isFrontFace());
  auto oldCullFace  = CQGLStateInst->setCullFace(isCullFace());

  //---

  setModelMatrix();

  //------

  canvas_->bindProgram(s_program);

  canvas_->setProgramMatrices(s_program, programMatrixData());

  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  //---

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  bool oldTexture = CQGLStateInst->setEnableTexture(true);

  //---

  canvas_->glBindVertexArray(glData_.vao);

  // get GL_ARRAY_BUFFER (vertices) data from vertexBuffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, glData_.vertexBuffer);
  canvas_->glBufferData(GL_ARRAY_BUFFER, sizeof(CGLVector3D)*vertices_.size(),
                        vertices_.data(), GL_STATIC_DRAW);

  // position attribute populated from 3 floats (x, y, z) (data from GL_ARRAY_BUFFER)
  canvas_->glVertexAttribPointer(s_program->posAttr, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  canvas_->glEnableVertexAttribArray(s_program->posAttr);

  //--

  // get GL_ARRAY_BUFFER data from uvBuffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, glData_.uvBuffer);
  canvas_->glBufferData(GL_ARRAY_BUFFER, sizeof(CGLVector2D)*uvs_.size(),
                        uvs_.data(), GL_STATIC_DRAW);

  // texture pos attribute populated from 2 floats (u, v) (data from GL_ARRAY_BUFFER)
  canvas_->glVertexAttribPointer(s_program->texPosAttr, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  canvas_->glEnableVertexAttribArray(s_program->texPosAttr);

  //--

  // get GL_ARRAY_BUFFER data from colBuffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, glData_.colBuffer);
  canvas_->glBufferData(GL_ARRAY_BUFFER, sizeof(CGLColor)*colors_.size(),
                        colors_.data(), GL_STATIC_DRAW);

  // color attribute populated from 3 floats (r, g, b, a)
  canvas_->glVertexAttribPointer(s_program->colAttr, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
  canvas_->glEnableVertexAttribArray(s_program->colAttr);

  //--

  // get GL_ELEMENT_ARRAY_BUFFER from indexBuffer
  canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glData_.indexBuffer);
  canvas_->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t)*glData_.indexElementCount,
                        indexes_.data(), GL_STATIC_DRAW);

  //---

  bindTexture();

  //---

  s_program->setUniformValue(s_program->textureUniform, 0);

  canvas_->glBindVertexArray(glData_.vao);
  canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glData_.indexBuffer);

//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glDrawElements(GL_TRIANGLES, glData_.indexElementCount, GL_UNSIGNED_SHORT, nullptr);

  //---

  canvas_->glBindBuffer(GL_ARRAY_BUFFER, 0);
  canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  canvas_->glBindVertexArray(0);

  canvas_->glDisableVertexAttribArray(s_program->posAttr);
  canvas_->glDisableVertexAttribArray(s_program->texPosAttr);
  canvas_->glDisableVertexAttribArray(s_program->colAttr);

  //---

  canvas_->bindProgram(nullptr);

  CQGLStateInst->setEnableTexture(oldTexture);

  glPopAttrib();

  //---

  CQGLStateInst->setFrontFaceFlag(oldFrontFace);
  CQGLStateInst->setCullFace(oldCullFace);

  //---

  bbox_ = CBBox3D();

  auto db = CPoint3D(0, 0, size()/20.0);

  for (const auto &v : vertices_) {
    auto p1 = v.point() - db;
    auto p2 = p1 + db;

    bbox_ += modelMatrix()*p1;
    bbox_ += modelMatrix()*p2;
  }
}

void
Text3DObj::
setModelMatrix(uint)
{
  modelMatrix_ = CMatrix3DH::identity();

  if (isRotated()) {
    auto o = origin();

    modelMatrix_.translated(float(o.getX()), float(o.getY()), float(o.getZ()));

    modelMatrix_.rotated(xAngle(), CVector3D(1.0, 0.0, 0.0));
    modelMatrix_.rotated(yAngle(), CVector3D(0.0, 1.0, 0.0));
    modelMatrix_.rotated(zAngle(), CVector3D(0.0, 0.0, 1.0));

    modelMatrix_.translated(-float(o.getX()), -float(o.getY()), -float(o.getZ()));
  }

  auto pos = this->position();

  double x = pos.getX();
  double y = pos.getY();

  if      (align_ & Qt::AlignRight  ) x -= bbox_.getXSize();
  else if (align_ & Qt::AlignHCenter) x -= bbox_.getXSize()/2.0;

  if      (align_ & Qt::AlignBottom ) y -= bbox_.getYSize();
  else if (align_ & Qt::AlignVCenter) y -= bbox_.getYSize()/2.0;

  modelMatrix_.translated(x, y, pos.getZ());

  modelMatrix_.scaled(size(), size(), 1.0);
}

}
