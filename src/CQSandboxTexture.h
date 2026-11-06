#ifndef Texture_H
#define Texture_H

#include <CGeomTexture.h>
#include <CQGLTexture.h>

class CQGLTexture;

namespace CQSandbox {

class Canvas3D;

class Texture : public CGeomTexture {
 public:
  Texture(Canvas3D *canvas);

  bool load(const QString &filename, bool flip);

  bool isFlipped() const { return flipped_; }
  void setFlipped(bool b) { flipped_ = b; }

  CQGLTexture *glTexture(Canvas3D *canvas);

  bool isWrapped() const { return wrapped_; }
  void setWrapped(bool b);

  void setGlTextures(Canvas3D *canvas, CQGLTexture *t1, CQGLTexture *t2);

 private:
  struct TextureData {
    CQGLTexture* glTexture        { nullptr };
    CQGLTexture* glTextureFlipped { nullptr };
  };

  using CanvasTextureData = std::map<uint, TextureData>;

  Canvas3D *canvas_ { nullptr };

  bool flipped_ { false }; // TODO: per canvas

  CanvasTextureData canvasTextureData_;
  bool              wrapped_ { true };
};

//---

class TextureMgr {
 public:
  using Textures = std::vector<Texture *>;

 public:
  static TextureMgr *instance() {
    static TextureMgr *inst;

    if (! inst)
      inst = new TextureMgr;

    return inst;
  }

 ~TextureMgr() { }

  void addTexture(Texture *texture) {
    textures_.push_back(texture);
  }

  const Textures &textures() const { return textures_; }

  Texture *getTexture(int id) const {
    for (auto *texture : textures_) {
      if (texture->id() == id)
        return texture;
    }

    return nullptr;
  }

  uint getNextId() { return ++lastId_; }

 private:
  TextureMgr() { }

 private:
  Textures textures_;
  uint     lastId_ { 0 };
};

}

#endif
