#ifndef CQSandboxCanvas2D_H
#define CQSandboxCanvas2D_H

#include <CQSandboxGroup2DObj.h>

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

class CQRubberBand;
class CQTcl;
class QTimer;

namespace CQSandbox {

class  App;
class  Canvas2D;
class  ParticleSystem;
class  Particle;
struct Viewport;

//---

#ifdef CQSANDBOX_CIRCLES
class CirclesMgr;

class CirclesGroupObj : public Group2DObj {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  CirclesGroupObj(Canvas2D *canvas, const Rect2D &rect);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  CirclesMgr *mgr_ { nullptr };
};
#endif

//---

class EditObj : public Object2D {
  Q_OBJECT

 public:
  EditObj(Canvas2D *canvas, const QString &name);

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
  static bool create(Canvas2D *canvas, const QStringList &args);

  RealEdit(Canvas2D *canvas, const Point2D &p, const QString &name);

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
  static bool create(Canvas2D *canvas, const QStringList &args);

  IntegerEdit(Canvas2D *canvas, const Point2D &p, const QString &name);

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

class ButtonObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  ButtonObj(Canvas2D *canvas, const Point2D &p, const QString &name);

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

class Canvas2D : public QFrame {
  Q_OBJECT

  Q_PROPERTY(bool running    READ isRunning  WRITE setRunning)
  Q_PROPERTY(int  timerTicks READ timerTicks WRITE setTimerTicks)

 public:
  enum class Type {
    CAMERA = 0,
    MODEL  = 1,
    GAME   = 2
  };

  using Viewports = std::vector<Viewport *>;

 public:
  Canvas2D(App *app);

  App* app() const { return app_; }

  CQTcl *tcl() const;

  //---

  const Type &type() const { return type_; }
  void setType(const Type &type);

  bool isRunning() const { return running_; }
  void setRunning(bool b) { running_ = b; }

  uint timerTicks() const { return timerTicks_; }
  void setTimerTicks(uint i) { timerTicks_ = i; }

  //---

  void resizeEvent(QResizeEvent *) override;

  void paintEvent(QPaintEvent *) override;

  void drawStep();

  void drawParticle(QPainter *, Particle *);

  void fadeImage(QImage &image1, QImage &image2, double f);

  QPainter *painter() const { return painter_; }

  int pixelWidth () const { return pixelWidth_ ; }
  int pixelHeight() const { return pixelHeight_; }

  //---

  void mousePressEvent  (QMouseEvent *) override;
  void mouseMoveEvent   (QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;

  void keyPressEvent  (QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;

  bool getKeyPressed(const QString &key) const;

  QString getKeyString(QKeyEvent *e) const;

  //---

  void selectObjectAtPoint(const QPoint &p, bool clear=true);
  void selectObjectInsideRect(const QRect &r, bool clear=true);

  void deselectAllObjects();

  void getSelectedObjects(std::vector<Object2D *> &objects) const;

  Object2D *getObjectAtPos(const QPoint &pos) const;
  Object2D *getObjectByName(const QString &name) const;

  //---

  const QStringList &moduleDirs() const { return moduleDirs_; }

  //---

  const QColor &selectedColor() const { return selectedColor_; }
  void setSelectedColor(const QColor &c) { selectedColor_ = c; }

  //---

  void init(const QStringList &tclArgs);

  void addCommands();

  void play();
  void pause();
  void step();
  void stepInit(bool &buffered);

  void drawBuffered();

  QString addNewObject(Object2D *obj);

  void addObject(Object2D *obj);
  void removeObject(Object2D *obj);

  void createObjCommand(Object2D *obj);
  void createObjTclCommand(Object2D *obj);

  Point2D pointToWindow(const Point2D &p) const;

  Rect2D rectToPixel(const Rect2D &rect) const;
  Point2D pointToPixel(const Point2D &p) const;

  QSizeF pixelSizeToWindow(const QSizeF &psize) const;

  Viewport *currentViewport() const;

  Viewport *addViewport();

  QVariant getPaletteValue(const QString &);
  void setPaletteValue(const QString &, const QString &);

  bool getStyleValue(const QString &, QVariant &res);
  bool setStyleValue(const QString &, const QString &);

  const QPen   &stylePen  () const { return stylePen_  ; }
  const QBrush &styleBrush() const { return styleBrush_; }

  //---

  ParticleSystem *psys() const { return psys_; }

  //---

  bool runTclCmd(const QString &cmd);

 protected:
  static int objectCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
  static int objectTclCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int viewportCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int canvasProc (void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
  static int paletteProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
  static int styleProc  (void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int viewportProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int drawPointProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int fmulProc (void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv);
  static int fmaProc  (void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv);
  static int hypotProc(void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv);

  static int helpProc(void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv);

  static int uiProc(void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv);

  bool getValue(const QString &name, const QStringList &args, QVariant &value);
  bool setValue(const QString &, const QString &, const QStringList &);
  bool exec(const QString &, const QStringList &, QVariant &);

  void updatePixelRanges();

 Q_SIGNALS:
  void objectsChanged();

  void runStateChanged();

  void typeChanged();

 protected Q_SLOTS:
  void timerSlot();
  void stepTimerSlot();
  void drawTimerSlot();

  void updateStatus();

 protected:
  using Objects = std::vector<Object2D *>;

  //---

  struct MouseData {
    bool            pressed   { false };
    bool            isShift   { false };
    bool            isControl { false };
    Qt::MouseButton button    { Qt::NoButton };
    QPoint          press     { 0, 0 };
    QPoint          move1     { 0, 0 };
    QPoint          move2     { 0, 0 };
    int             key       { 0 };
    QString         keyStr;
  };

  //---

  App* app_ { nullptr };

  CQTcl* tcl_ { nullptr };

  Type type_ { Type::CAMERA };

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

  QStringList tclArgs_;

  QPen   stylePen_;
  QBrush styleBrush_;

  MouseData mouseData_;

  Object2D* pressObj_ { nullptr };

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

  struct TclCallbacks {
    bool mouseEvent      { true };
    bool keyEvent        { true };
    bool rubberBandEvent { false };
  };

  TclCallbacks tclCallbacks_;

  //--

  Viewports viewports_;
  Viewport* currentViewport_ { nullptr };
  QString   currentViewportName_;

  Objects allObjects_;

  QStringList moduleDirs_;

  //---

  using KeyPressed = std::map<QString, bool>;

  KeyPressed keyPressed_;

  //---

  QColor selectedColor_ { 255, 255, 0, 100 };

  CQRubberBand* rubberBand_ { nullptr };
};

}

#endif
