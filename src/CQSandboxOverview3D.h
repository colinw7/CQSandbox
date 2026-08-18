#ifndef Overview_H
#define Overview_H

#include <CWindowRange2D.h>
//#include <CDisplayRange2D.h>
#include <CMatrix3DH.h>
#include <CBBox3D.h>
#include <CPoint3D.h>
#include <CRGBA.h>

#include <QFrame>

class CGeomObject3D;
class CGeomFace3D;
class CQRubberBand;
class CGLCamera;

namespace CQSandbox {

class App;
class Shape;
class Camera;
class Light;
class Text;

class Model3DObj;
class ParticleList3DObj;
class Path3DObj;
class Shape3DObj;
class Sprite3DObj;
class Surface3DObj;
class Text3DObj;
class Object3D;

class Overview3D : public QFrame {
  Q_OBJECT

 public:
  enum class ViewType {
    NONE,
    XY,
    ZY,
    XZ,
    THREED
  };

  enum class SelectType {
    OBJECT,
    FACE,
    EDGE,
    POINT
  };

  enum class EditType {
    SELECT,
    CAMERA,
    LIGHT
  };

 private:
  struct CameraShape {
    CPoint3D p11;
    CPoint3D p12;
    CPoint3D p21;
    CPoint3D p22;
  };

  struct SelectAtData;

 public:
  Overview3D(App *app);
 ~Overview3D();

  void init();

  //---

  App *app() { return app_; }

  //---

  bool isEqualScale() const { return equalScale_; }
  void setEqualScale(bool b) { equalScale_ = b; updateRange(); }

  const EditType &editType() const { return editType_; }
  void setEditType(const EditType &v) { editType_ = v; updateState(); }

  const SelectType &selectType() const { return selectType_; }
  void setSelectType(const SelectType &v) { selectType_ = v; updateState(); }

  bool isWireframe() const { return wireframe_; }
  void setWireframe(bool b) { wireframe_ = b; update(); }

  bool isSolid() const { return solid_; }
  void setSolid(bool b) { solid_ = b; update(); }

  bool isCameraVisible() const { return cameraVisible_; }
  void setCameraVisible(bool b) { cameraVisible_ = b; update(); }

  bool isLightsVisible() const { return lightsVisible_; }
  void setLightsVisible(bool b) { lightsVisible_ = b; update(); }

  bool isBasisVisible() const { return basisVisible_; }
  void setBasisVisible(bool b) { basisVisible_ = b; update(); }

  //---

  void resizeEvent(QResizeEvent *) override;
  void paintEvent (QPaintEvent  *) override;

  void mousePressEvent  (QMouseEvent *) override;
  void mouseMoveEvent   (QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;

  void wheelEvent(QWheelEvent *e) override;

  void keyPressEvent(QKeyEvent *e) override;

  //---

  void updateObjects();
  void updateObject(CGeomObject3D *object);
  void updateModel(Model3DObj *obj);
  void updateParticleList(ParticleList3DObj *obj);
  void updatePath(Path3DObj *obj);
  void updateShape(Shape3DObj *obj);
  void updateSprite(Sprite3DObj *obj);
  void updateSurface(Surface3DObj *obj);
  void updateText(Text3DObj *obj);

  void drawObjects();
  void drawModel(Model3DObj *obj);
  void drawObject(CGeomObject3D *object);

  void drawCameras();
  void drawCamera(Camera *camera);

  void drawLights();

  void drawParticleList(ParticleList3DObj *obj);
  void drawPath(Path3DObj *obj);
  void drawShape(Shape3DObj *obj);
  void drawSprite(Sprite3DObj *obj);
  void drawSurface(Surface3DObj *obj);
  void drawText(Text3DObj *obj);

  void drawTexts();
//void drawText(Text *text);

  void drawLight(Light *light);

  void updateBBox();

  void getBasis(CGeomObject3D *object, CVector3D &u, CVector3D &v, CVector3D &w) const;

 private:
  struct ViewData;

  void updateState();

  void updateRange();

  void drawPolygon(const std::vector<CPoint3D> &points) const;

  void drawCone(const CVector3D &p, const CVector3D &d, double a) const;

  void drawLine(const CPoint3D &p1, const CPoint3D &p2, const QString &text="") const;
  void drawVector(const CVector3D &v1, const CVector3D &v2, const QString &text="") const;

  void drawCircle(const CPoint3D &origin, double r, const QString &label) const;
  void drawSphere(const CPoint3D &o, const CPoint3D &r) const;

  void drawPoint(const CVector3D &v, const QString &text="") const;
  void drawPoint(const CPoint3D &p, const QString &text="") const;

  void drawImage(const CPoint3D &p, const QImage &image) const;
  void drawPixmap(const CPoint3D &p, const QPixmap &pixmap) const;

  CPoint2D windowToPixelX(const CPoint2D &p) const;
  CPoint2D windowToPixelY(const CPoint2D &p) const;
  CPoint2D windowToPixelZ(const CPoint2D &p) const;
  CPoint2D windowToPixelP(const CPoint2D &p) const;

  //void getCameraShape(Camera *camera, CameraShape &shape) const;

  QPointF viewQPoint(ViewType viewType, const CPoint3D &p) const;
  CPoint2D viewPoint(ViewType viewType, const CPoint3D &p) const;

  CPoint2D pixelToView(ViewType viewType, const QPointF &p) const;

  void setCameraPosition(const CPoint2D &p);
  void setCameraOrigin(const CPoint2D &p);

  void setLightPosition(const CPoint2D &p);
  void setLightDirection(const CPoint2D &p);

  void selectObjectAt(const CPoint2D &p, bool clear) ;
  void selectObjectAt1(const ViewData &view, const CPoint3D &p, bool clear);

  void selectModelAt(Model3DObj *obj, const ViewData &view,
                     const QPointF &p1, SelectAtData &selectAtData);
  void selectShapeAt(Shape3DObj *obj, const ViewData &view,
                     const QPointF &p1, SelectAtData &selectAtData);

  void selectObjectIn(const CPoint2D &p, const QRect &r, bool clear);
  bool selectModelIn(Model3DObj *obj, ViewType viewType, const QRectF &r);
  bool selectShapeIn(Shape3DObj *obj, ViewType viewType, const QRectF &r);

 public Q_SLOTS:
  void cameraChangeSlot();
  void lightChangeSlot();

  void invalidate();

 private:
  struct ViewData {
    CDisplayRange2D* range { nullptr };
    QRectF           rect;
    int              ind  { -1 };
    ViewType         type { ViewType::NONE };
    QString          name;

    CPoint2D pixelToView(const QPointF &p) const {
      CPoint2D p1(p.x(), p.y());
      CPoint2D p2;

      if      (type == ViewType::XY) {
        (void) pressRange(p1, p2);
      }
      else if (type == ViewType::ZY) {
        (void) pressRange(p1, p2);
      }
      else if (type == ViewType::XZ) {
        (void) pressRange(p1, p2);
      }
      else
        assert(false);

      return p2;
    }

    bool pressRange(const CPoint2D &p, CPoint2D &p1) const {
      double xmin, ymin, xmax, ymax;
      range->getPixelRange(&xmin, &ymin, &xmax, &ymax);

      if (xmin > xmax) std::swap(xmin, xmax);
      if (ymin > ymax) std::swap(ymin, ymax);

      bool rc = (p.x >= xmin && p.y >= ymin && p.x <= xmax && p.y <= ymax);

      double x1, y1;
      range->pixelToWindow(p.x, p.y, &x1, &y1);

      p1 = CPoint2D(x1, y1);

      return rc;
    }

    CPoint2D viewPoint(const CPoint3D &p) const {
      CPoint2D p1;

      if      (type == ViewType::XY)
        p1 = CPoint2D(p.x, p.y);
      else if (type == ViewType::ZY)
        p1 = CPoint2D(p.z, p.y);
      else if (type == ViewType::XZ)
        p1 = CPoint2D(p.x, p.z);
      else
        assert(false);

      return p1;
    }
  };

  //---

  struct Face {
    CGeomFace3D*          face { nullptr };
    QColor                color;
    std::vector<CPoint3D> points;
    int                   ind { -1 };
  };

  using Faces = std::vector<Face>;

  using ObjFaces = std::map<CGeomObject3D *, Faces>;

  //---

  struct DrawData {
    QPainter* painter { nullptr };

    CMatrix3DH projectionMatrix;
    CMatrix3DH viewMatrix;
    CMatrix3DH pvMatrix;
    ObjFaces   objFaces;
    CBBox3D    bbox;

    double near { 0.0 };
    double far  { 1.0 };

    QColor pointColor { Qt::red };
    double pointSize  { 8 };
  };

  //---

  struct MouseData {
    bool     pressed   { false };
    bool     isShift   { false };
    bool     isControl { false };
    int      button    { Qt::LeftButton };
    CPoint2D pressPos;
    CPoint2D movePos1;
    CPoint2D movePos2;

    bool      raySet { false };
    CPoint3D  rayOrigin;
    CVector3D rayDirection;
  };

  //---

  using ObjFaceData  = std::map<Object3D *, std::vector<Face>>;
  using AreaObjFaces = std::map<double, ObjFaceData>;

  struct SelectAtData {
    AreaObjFaces areaObjFaces;
  };

  //---

  App *app_ { nullptr };

  ViewData xview_;
  ViewData yview_;
  ViewData zview_;
  ViewData pview_;

  std::vector<ViewData *> views_;   // all views
  std::vector<ViewData *> views2d_; // 2d views

  int ind_ { -1 };

  bool equalScale_ { true };
  bool valid_      { false };

  EditType   editType_   { EditType::CAMERA };
  SelectType selectType_ { SelectType::OBJECT };

  bool cameraVisible_ { false };
  bool lightsVisible_ { false };
  bool basisVisible_  { false };

  CBBox3D bbox_;

  MouseData mouseData_;

  DrawData drawData_;

  int w_ { 100 };
  int h_ { 100 };

  bool   bboxSet_ { false };
  double xs_ { 0.0 }, ys_ { 0.0 }, zs_ { 0.0 };

  bool wireframe_ { true };
  bool solid_     { false };

  QPixmap lightPixmap_;

  CQRubberBand* rubberBand_ { nullptr };
};

}

#endif
