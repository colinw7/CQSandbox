#ifndef CQSandbox_H
#define CQSandbox_H

#include <CQSandboxObject.h>

#include <CTclUtil.h>
//#include <CDisplayRange2D.h>
#include <CWindowRange2D.h>
#include <CMathUtil.h>
#include <CRGBA.h>
#include <CQuadTree.h>
#include <CArray2D.h>

#include <QFrame>
#include <QVariant>
#include <QPen>
#include <QBrush>
#include <QPainterPath>

#include <optional>

class CQArrow;
class CQAxis;
class CQCsvModel;

class QTimer;

namespace CQSandbox {

class App;
class Canvas;
class ParticleSystem;
class Particle;
class Viewport;

//---

class GroupObj : public Object {
  Q_OBJECT

 public:
  using Objects = std::vector<Object *>;

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  GroupObj(Canvas *canvas, const Rect &rect);

  const char *typeName() const override { return "group"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  const CDisplayRange2D &displayRange() const { return displayRange_; }

  const Objects &objects() const { return objects_; }

  Rect calcRect() const override;

  void draw(QPainter *) override;

  void addObject(Object *obj);
  void removeObject(Object *obj);

  Rect rectToPixel(const Rect &p) const;
  Point pointToPixel(const Point &p) const;

//Point pointToWindow(const Point &p) const override;

 Q_SIGNALS:
  void objectsChanged();

 protected:
  Rect rect_;

  CDisplayRange2D displayRange_;
  Objects         objects_;
};

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

class CirclesMgr;

class CirclesGroupObj : public GroupObj {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  CirclesGroupObj(Canvas *canvas, const Rect &rect);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  CirclesMgr *mgr_ { nullptr };
};

//---

class QuadTreeObj : public GroupObj {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  QuadTreeObj(Canvas *canvas);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 private:
  using QuadTree = CQuadTree<Object, Rect>;

  QuadTree quadTree_;
};

//---

class RectObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  RectObj(Canvas *canvas, const Rect &rect);

  const char *typeName() const override { return "rect"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Rect rect_;
};

//---

class CircleObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  CircleObj(Canvas *canvas, const Point &center, const Coord &radius);

  const char *typeName() const override { return "circle"; }

  const AnimatePoint &center() const { return center_; }
  void setCenter(const AnimatePoint &c) { center_ = c; }
  void setTargetCenter(const Point &c) { center_.setTarget(c); }

  const AnimateCoord &radius() const { return radius_; }
  void setRadius(const AnimateCoord &r) { radius_ = r; }
  void setTargetRadius(const Coord &r) { radius_.setTarget(r); }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  bool step() override;

  void draw(QPainter *) override;

 protected:
  AnimatePoint center_;
  AnimateCoord radius_;
};

//---

class LineObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  LineObj(Canvas *canvas, const Point &p1, const Point &p2);

  const char *typeName() const override { return "line"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point p1_;
  Point p2_;
};

//---

class TextObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  TextObj(Canvas *canvas, const Point &pos, const QString &text);

  const char *typeName() const override { return "text"; }

  const Point &position() const { return pos_; }
  void setPosition(const Point &v) { pos_ = v; }

  const QString &text() const { return text_; }
  void setText(const QString &s) { text_ = s; }

  const QFont &font() const { return font_; }
  void setFont(const QFont &f) { font_ = f; }

  bool isHtml() const { return html_; }
  void setHtml(bool b) { html_ = b; }

  const Qt::Alignment &align() const { return align_; }
  void setAlign(const Qt::Alignment &v) { align_ = v; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point         pos_;
  QString       text_;
  QFont         font_;
  QPen          border_;
  Qt::Alignment align_ { Qt::AlignCenter };
  bool          html_  { false };
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

  ImageObj(Canvas *canvas, const Point &pos, const QImage &image);

  const char *typeName() const override { return "image"; }

  const QImage &image() const { return image_; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point    pos_;
  Rect     rect_;
  Position posType_ { Position::TOP_LEFT };
  QImage   image_;
};

//---

class PathObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  PathObj(Canvas *canvas, const QPainterPath &path);

  const char *typeName() const override { return "path"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  QPainterPath path_;
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

  const Point &center() const { return center_; }
  void setCenter(const Point &o) { center_ = o; }

  const Point &offset() const { return offset_; }
  void setOffset(const Point &o) { offset_ = o; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  QPainterPath calcPath() const override { return path_; }

  bool step() override;

  void draw(QPainter *) override;

 protected:
  using Points = std::vector<Point>;

  Points       points_;
  AnimateCoord radius_;
  bool         connected_  { false };
  bool         showPoints_ { false };
  bool         fillUnder_  { false };
  OptCoord     fillUnderY_;
  double       angle_      { 0.0 };
  double       scale_      { 1.0 };
  Point        center_     { 0, 0 };
  Point        offset_     { 0, 0 };
  QPainterPath path_;
};

//---

class ArrowObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  ArrowObj(Canvas *canvas, const Point &p1, const Point &p2);

  const char *typeName() const override { return "arrow"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point p1_;
  Point p2_;

  CQArrow *arrow_ { nullptr };
};

//---

class AxisObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  AxisObj(Canvas *canvas, const Point &pos, const Coord &len);

  const char *typeName() const override { return "axis"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point pos_;
  Coord len_;

  CQAxis *axis_ { nullptr };
};

//---

class ParticleObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  ParticleObj(Canvas *canvas, const Point &pos);

  const char *typeName() const override { return "particle"; }

  const Particle *particle() const { return particle_; }
  void setParticle(Particle *p);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point     pos_;
  Particle* particle_ { nullptr };
};

//---

class VectorObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  VectorObj(Canvas *canvas);

  const char *typeName() const override { return "vector"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  CVector2D v_;
};

//---

class ArrayObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  ArrayObj(Canvas *canvas, uint dim0, uint dim1);
  ArrayObj(Canvas *canvas, const CArray2D<double> &a);

  const char *typeName() const override { return "array"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  CArray2D<double> a_;
};

//---

class CsvObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  CsvObj(Canvas *canvas, const QString &filename);

  const char *typeName() const override { return "csv"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

 protected:
  QString filename_;

  CQCsvModel *csv_ { nullptr };
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

  RealEdit(Canvas *canvas, const Point &p, const QString &name);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

  void move(int, int) override;

 protected:
  Point  p_;
  double minValue_ { 0.0 };
  double maxValue_ { 1.0 };
};

//---

class IntegerEdit : public EditObj {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  IntegerEdit(Canvas *canvas, const Point &p, const QString &name);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

  void move(int, int) override;

  void click(int x, int y) override;

 protected:
  int getIValue() const;
  void setIValue(int i);

 protected:
  Point p_;
  int   minValue_ { -9999 };
  int   maxValue_ { 9999 };
  QRect lrect_;
  QRect rrect_;
};

//---

class ButtonObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  ButtonObj(Canvas *canvas, const Point &p, const QString &name);

  const char *typeName() const override { return "button"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

  void click(int x, int y) override;

 protected:
  Point   p_;
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

  Point pointToWindow(const Point &p) const;

  Rect rectToPixel(const Rect &rect) const;
  Point pointToPixel(const Point &p) const;

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
