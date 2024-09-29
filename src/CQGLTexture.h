#ifndef CQGL_TEXTURE_H
#define CQGL_TEXTURE_H

#include <QImage>
#include <GL/gl.h>

class CQGLTexture {
 public:
  enum class WrapType {
    CLAMP,
    REPEAT
  };

 public:
  CQGLTexture();
  CQGLTexture(const QImage &image);

 ~CQGLTexture();

  const WrapType &wrapType() const { return wrapType_; }
  void setWrapType(const WrapType &v) { wrapType_ = v; }

  bool useAlpha() const { return useAlpha_; }
  void setUseAlpha(bool b) { useAlpha_ = b; }

  bool load(const QString &fileName, bool flip=false);

  bool load(const QImage &image, bool flip=false);

  void setImage(const QImage &image);

  void bind() const;
  void unbind() const;

  uint getId() const { return id_; }

  void enable(bool b);

  void draw();
  void draw(double x1, double y1, double x2, double y2);
  void draw(double x1, double y1, double z1, double x2, double y2, double z2,
            double tx1=0.0, double ty1=0.0, double tx2=1.0, double ty2=1.0);

 private:
  CQGLTexture(const CQGLTexture &);

  CQGLTexture &operator=(const CQGLTexture &);

  bool init(const QImage &image, bool flip);

 private:
  QImage         image_;
  unsigned char *imageData_ { nullptr };

  uint     id_       { 0 };
  bool     valid_    { false };
  WrapType wrapType_ { WrapType::REPEAT };
  bool     cubeMap_  { false };
  bool     useAlpha_ { true };
};

//---

class CQGLTextureMgr {
 public:
  static CQGLTextureMgr *getInstance() {
    static CQGLTextureMgr *instance;

    if (! instance)
      instance = new CQGLTextureMgr;

    return instance;
  }

  CQGLTexture *load(const QString &fileName, bool flip=false) {
    // Look for the texture in the registry
    auto p = texture_map_.find(fileName);

    if (p != texture_map_.end())
      return (*p).second;

    // If not found, load the texture
    CQGLTexture *texture = new CQGLTexture;

    if (! texture->load(fileName, flip)) {
      delete texture;
      return NULL;
    }

    // The texture has been successfully loaded, register it.
    texture_map_[fileName] = texture;

    return texture;
  }

 private:
  using TextureMap = std::map<QString, CQGLTexture *>;

  TextureMap texture_map_;
};

#endif
