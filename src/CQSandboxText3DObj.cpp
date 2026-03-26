#include <CQSandboxText3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLUtil.h>

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

bool
Text3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Text3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Text3DObj::
Text3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

void
Text3DObj::
init()
{
  Object3D::init();

  //---

  if (! s_program) {
#if 0
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "attribute highp vec4 position;\n"
      "attribute highp vec2 texCoord0;\n"
      "attribute lowp  vec3 color;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "varying highp vec2 uv0;\n"
      "varying lowp vec3 col;\n"
      "void main() {\n"
      "  gl_Position = projection * view * model * position;\n"
      "  uv0 = texCoord0;\n"
      "  col = color;\n"
      "}\n";

    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "varying lowp vec3 col;\n"
      "varying highp vec2 uv0;\n"
      "uniform sampler2D textureId;\n"
      "void main() {\n"
       " vec4 tc = texture(textureId, uv0);\n"
      "  gl_FragColor = vec4(col.r, col.g, col.b, tc.r);\n"
      "}\n";
#endif

    s_program = new TextShaderProgram(this);

#if 0
    s_program->addVertexCode  (vertexShaderSource);
    s_program->addFragmentCode(fragmentShaderSource);
#else
    s_program->addVertexFile  (canvas_->buildDir() + "/shaders/text.vs");
    s_program->addFragmentFile(canvas_->buildDir() + "/shaders/text.fs");
#endif

    s_program->link();

    //---

    s_program->posAttr = s_program->attributeLocation("position");
    Q_ASSERT(s_program->posAttr != -1);

    s_program->colAttr = s_program->attributeLocation("color");
  //Q_ASSERT(s_program->colAttr != -1);

    s_program->texPosAttr = s_program->attributeLocation("texCoord0");
    Q_ASSERT(s_program->texPosAttr != -1);

    s_program->setProjectionUniform();
    s_program->setViewUniform();

    s_program->textureUniform = s_program->uniformLocation("textureId");
    Q_ASSERT(s_program->textureUniform != -1);
  }

  initFont();
}

void
Text3DObj::
initFont()
{
  if (s_fontData)
    return;

  s_fontData = new FontData;

  //---

  auto fontData = readFile("fonts/OpenSans-Regular.ttf");
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

  glGenTextures(1, &s_fontData->texture);
  glBindTexture(GL_TEXTURE_2D, s_fontData->texture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s_fontData->atlasWidth, s_fontData->atlasHeight,
               0, GL_RED, GL_UNSIGNED_BYTE, atlasData.get());
  glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

  canvas_->glGenerateMipmap(GL_TEXTURE_2D);
}

std::vector<uint8_t>
Text3DObj::
readFile(const char *path) const
{
  std::ifstream file(path, std::ios::binary | std::ios::ate);

  if (! file.is_open())
    assert(false);

  const auto size = file.tellg();

  file.seekg(0, std::ios::beg);
  auto bytes = std::vector<uint8_t>(size);
  file.read(reinterpret_cast<char *>(&bytes[0]), size);
  file.close();

  return bytes;
}

void
Text3DObj::
setText(const QString &text)
{
  text_ = text;

  textDataValid_  = false;
}

QVariant
Text3DObj::
getValue(const QString &name, const QStringList &args)
{
  if (name == "text")
    return text();
  else
    return Object3D::getValue(name, args);
}

bool
Text3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if (name == "text")
    setText(value);
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

  xscale_ = size_;
  yscale_ = size_;
  zscale_ = 1.0;

  auto matrixFlags = ModelMatrixFlags::TRANSLATE | ModelMatrixFlags::SCALE;

  if (isRotated())
    matrixFlags |= ModelMatrixFlags::ROTATE;

  setModelMatrix(matrixFlags);

  //------

  s_program->bind();

  s_program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  //---

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glEnable(GL_TEXTURE_2D);

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

  //matrix1.translate(pos_.x, pos_.y, pos_.z - 2);
//matrix1.rotate(rotation_, rotation_, rotation_, 0);
//matrix1.scale(size_, size_, 1.0f);

  glBindTexture(GL_TEXTURE_2D, s_fontData->texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
  glActiveTexture(GL_TEXTURE0);

  s_program->setUniformValue(s_program->textureUniform, GL_TEXTURE0);

  canvas_->glBindVertexArray(glData_.vao);
  canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glData_.indexBuffer);
  glDrawElements(GL_TRIANGLES, glData_.indexElementCount, GL_UNSIGNED_SHORT, nullptr);

  //---

  canvas_->glBindBuffer(GL_ARRAY_BUFFER, 0);
  canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  canvas_->glBindVertexArray(0);

  canvas_->glDisableVertexAttribArray(s_program->posAttr);
  canvas_->glDisableVertexAttribArray(s_program->texPosAttr);
  canvas_->glDisableVertexAttribArray(s_program->colAttr);

  //---

  s_program->release();

  glPopAttrib();
}

}
