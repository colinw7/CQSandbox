#ifndef CQSandbox_H
#define CQSandbox_H

#include <CQSandboxGroupObj.h>

#include <CTclUtil.h>
#include <CWindowRange2D.h>
#include <CMathUtil.h>
#include <CRGBA.h>
#include <CArray2D.h>

#include <QFrame>
#include <QVariant>
#include <QPen>
#include <QBrush>
#include <QPainterPath>

#include <optional>

class CQTcl;
class QTimer;

namespace CQSandbox {

class App;
class Canvas;
class ParticleSystem;
class Particle;
class Viewport;

//---

class RendererObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  RendererObj(Canvas *canvas);

  const char *typeName() const override { return "renderer"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

 private:
  QBrush brush_;
  QPen   pen_;
  QFont  font_;
};

//---

#ifdef CQSANDBOX_CIRCLES
class CirclesMgr;

class CirclesGroupObj : public GroupObj {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  CirclesGroupObj(Canvas *canvas, const Rect2D &rect);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  CirclesMgr *mgr_ { nullptr };
};
#endif

//---

class RectObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  RectObj(Canvas *canvas, const Rect2D &rect);

  const char *typeName() const override { return "rect"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Rect2D rect_;
};

//---

class CircleObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  CircleObj(Canvas *canvas, const Point2D &center, const Coord &radius);

  const char *typeName() const override { return "circle"; }

  const AnimatePoint2D &center() const { return center_; }
  void setCenter(const AnimatePoint2D &c) { center_ = c; }
  void setTargetCenter(const Point2D &c) { center_.setTarget(c); }

  const AnimateCoord &radius() const { return radius_; }
  void setRadius(const AnimateCoord &r) { radius_ = r; }
  void setTargetRadius(const Coord &r) { radius_.setTarget(r); }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  bool step() override;

  void draw(QPainter *) override;

 protected:
  AnimatePoint2D center_;
  AnimateCoord   radius_;
};

//---

class LineObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  LineObj(Canvas *canvas, const Point2D &p1, const Point2D &p2);

  const char *typeName() const override { return "line"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point2D p1_;
  Point2D p2_;
};

//---

class ImageObj : public Object {
  Q_OBJECT

 public:
  enum Position {
    TOP_LEFT,
    CENTER,
    RECT
  };

  static bool create(Canvas *canvas, const QStringList &args);

  ImageObj(Canvas *canvas, const Point2D &pos, const QImage &image);

  const char *typeName() const override { return "image"; }

  const QImage &image() const { return image_; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point2D  pos_;
  Rect2D   rect_;
  Position posType_ { Position::TOP_LEFT };
  QImage   image_;
};

//---

class PointListObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  PointListObj(Canvas *canvas, const Coord &radius);

  const char *typeName() const override { return "pointList"; }

  const AnimateCoord &radius() const { return radius_; }
  void setRadius(const AnimateCoord &r) { radius_ = r; }
  void setTargetRadius(const Coord &r) { radius_.setTarget(r); }

  bool isConnected() const { return connected_; }
  void setConnected(bool b) { connected_ = b; }

  bool isShowPoints() const { return showPoints_; }
  void setShowPoints(bool b) { showPoints_ = b; }

  bool isFillUnder() const { return fillUnder_; }
  void setFillUnder(bool b) { fillUnder_ = b; }

  const OptCoord &fillUnderY() const { return fillUnderY_; }
  void setFillUnderY(const OptCoord &v) { fillUnderY_ = v; }

  double angle() const { return angle_; }
  void setAngle(double r) { angle_ = r; }

  double scale() const { return scale_; }
  void setScale(double r) { scale_ = r; }

  const Point2D &center() const { return center_; }
  void setCenter(const Point2D &o) { center_ = o; }

  const Point2D &offset() const { return offset_; }
  void setOffset(const Point2D &o) { offset_ = o; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  QPainterPath calcPath() const override { return path_; }

  bool step() override;

  void draw(QPainter *) override;

 protected:
  using Points = std::vector<Point2D>;

  Points       points_;
  AnimateCoord radius_;
  bool         connected_  { false };
  bool         showPoints_ { false };
  bool         fillUnder_  { false };
  OptCoord     fillUnderY_;
  double       angle_      { 0.0 };
  double       scale_      { 1.0 };
  Point2D      center_     { 0, 0 };
  Point2D      offset_     { 0, 0 };
  QPainterPath path_;
};

//---

class ParticleObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  ParticleObj(Canvas *canvas, const Point2D &pos);

  const char *typeName() const override { return "particle"; }

  const Particle *particle() const { return particle_; }
  void setParticle(Particle *p);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point2D   pos_;
  Particle* particle_ { nullptr };
};

//---

class EditObj : public Object {
  Q_OBJECT

 public:
  EditObj(Canvas *canvas, const QString &name);

  const char *typeName() const override { return "edit"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  QString name_;
  QString proc_;
};

//---

class RealEdit : public EditObj {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  RealEdit(Canvas *canvas, const Point2D &p, const QString &name);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  void draw(QPainter *) override;

  void move(int, int) override;

 protected:
  Point2D p_;
  double  minValue_ { 0.0 };
  double  maxValue_ { 1.0 };
};

//---

class IntegerEdit : public EditObj {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  IntegerEdit(Canvas *canvas, const Point2D &p, const QString &name);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  void draw(QPainter *) override;

  void move(int, int) override;

  void click(int x, int y) override;

 protected:
  int getIValue() const;
  void setIValue(int i);

 protected:
  Point2D p_;
  int     minValue_ { -9999 };
  int     maxValue_ { 9999 };
  QRect   lrect_;
  QRect   rrect_;
};

//---

class ButtonObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  ButtonObj(Canvas *canvas, const Point2D &p, const QString &name);

  const char *typeName() const override { return "button"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  void draw(QPainter *) override;

  void click(int x, int y) override;

 protected:
  Point2D p_;
  QString name_;
  QString proc_;
};

//---

class Canvas : public QFrame {
  Q_OBJECT

 public:
  using Viewports = std::vector<Viewport *>;

 public:
  Canvas(App *app);

  App* app() const { return app_; }

  CQTcl *tcl() const;

  void resizeEvent(QResizeEvent *) override;

  void paintEvent(QPaintEvent *) override;

  void drawStep();

  void drawParticle(QPainter *, Particle *);

  void fadeImage(QImage &image1, QImage &image2, double f);

  QPainter *painter() const { return painter_; }

  //---

  void mousePressEvent  (QMouseEvent *) override;
  void mouseMoveEvent   (QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;

  void keyPressEvent  (QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;

  bool getKeyPressed(const QString &key) const;

  QString getKeyString(QKeyEvent *e) const;

  //---

  Object *getObjectAtPos(const QPoint &pos) const;
  Object *getObjectByName(const QString &name) const;

  void init();

  void addCommands();

  void play();
  void pause();
  void step();
  void stepInit(bool &buffered);

  void drawBuffered();

  QString addNewObject(Object *obj);

  void addObject(Object *obj);
  void removeObject(Object *obj);

  void createObjCommand(Object *obj);

  Point2D pointToWindow(const Point2D &p) const;

  Rect2D rectToPixel(const Rect2D &rect) const;
  Point2D pointToPixel(const Point2D &p) const;

  QSizeF pixelSizeToWindow(const QSizeF &psize) const;

  Viewport *currentViewport() const;

  Viewport *addViewport();

  QVariant getPaletteValue(const QString &);
  void setPaletteValue(const QString &, const QString &);

  QVariant getStyleValue(const QString &);
  void setStyleValue(const QString &, const QString &);

  const QPen   &stylePen  () const { return stylePen_  ; }
  const QBrush &styleBrush() const { return styleBrush_; }

  //---

  ParticleSystem *psys() const { return psys_; }

  //---

  bool runTclCmd(const QString &cmd);

 protected:
  static int objectCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int viewportCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int canvasProc (void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
  static int paletteProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
  static int styleProc  (void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int viewportProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int drawPointProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int fmulProc (void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv);
  static int fmaProc  (void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv);
  static int hypotProc(void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv);

  static int uiProc(void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv);

  bool getValue(const QString &name, const QStringList &args, QVariant &value);
  bool setValue(const QString &, const QString &, const QStringList &);

  bool exec(const QString &, const QStringList &, QVariant &);

  void updatePixelRanges();

 Q_SIGNALS:
  void objectsChanged();

 protected Q_SLOTS:
  void timerSlot();
  void stepTimerSlot();
  void drawTimerSlot();

 protected:
  using Objects = std::vector<Object *>;

  App* app_ { nullptr };

  CQTcl* tcl_ { nullptr };

  ParticleSystem *psys_ { nullptr };

  QTimer *timer_      { nullptr };
  QTimer *stepTimer_  { nullptr };
  QTimer *drawTimer_  { nullptr };
  bool    running_    { false };
  uint    timerTicks_ { 30 };
  uint    ticks_      { 0 };

  size_t lastInd_ { 0 };

  bool initialized_ { false };
  bool inited_      { false };
  bool initRun_     { false };

  QPen   stylePen_;
  QBrush styleBrush_;

  QPoint  pressPos_;
  Object* pressObj_ { nullptr };
  QPoint  motionPos_;
  bool    pressed_  { false };

  QPainter *painter_            { nullptr };
  bool      drawing_            { false };
  bool      drawBufferedNeeded_ { false };

  bool   buffered_    { false };
  bool   blend_       { false };
  double blendFactor_ { 0.95 };
  QImage bufferImage1_;
  QImage bufferImage2_;
  int    pixelWidth_  { 1 };
  int    pixelHeight_ { 1 };

  //--

  Viewports viewports_;
  Viewport* currentViewport_ { nullptr };
  QString   currentViewportName_;

  Objects allObjects_;

  //---

  using KeyPressed = std::map<QString, bool>;

  KeyPressed keyPressed_;
};

}

#endif
