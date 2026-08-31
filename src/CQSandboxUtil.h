#ifndef CQSandboxUtil_H
#define CQSandboxUtil_H

#include <CQSandboxGeom.h>

#include <CQTclUtil.h>
#include <CQSVGUtil.h>
#include <CGLVector2D.h>
#include <CGLVector3D.h>
#include <CBBox3D.h>
#include <CGLColor.h>
#include <CWindowRange2D.h>
#include <CPoint3D.h>

#include <QPainterPath>
#include <QString>
#include <QVector3D>

namespace CQSandbox {

namespace Util {

inline QString intToString(int i) {
  return QString::number(i);
}

inline QString realToString(double r) {
  return QString::number(r);
}

inline QString boolToString(bool b) {
  return QString(b ? "1" : "0");
}

//---

inline bool stringToInt(const QString &s, int &i) {
  bool ok;
  i = s.toInt(&ok);
  return ok;
}

inline int stringToInt(const QString &s) {
  int i;
  if (! stringToInt(s, i))
    i = 0;
  return i;
}

inline bool stringToReal(const QString &s, double &r) {
  bool ok;
  r = s.toDouble(&ok);
  return ok;
}

inline double stringToReal(const QString &s) {
  double r;
  if (! stringToReal(s, r))
    r = 0.0;
  return r;
}

inline bool stringToBool(const QString &s) {
  auto s1 = s.toLower();
  if (s1 == "false" || s1 == "no" || s1 == "0")
    return false;
  if (s1 == "true" || s1 == "yes" || s1 == "1")
    return true;
  return /*default*/true;
}

//---

inline QColor stringToColor(CQTcl *tcl, const QString &str) {
  if (str == "none")
    return Qt::transparent;

  QStringList strs;
  (void) tcl->splitList(str, strs);

  if (strs.size() == 3 || strs.size() == 4) {
    auto r = stringToReal(strs[0]);
    auto g = stringToReal(strs[1]);
    auto b = stringToReal(strs[2]);

    auto a = (strs.size() == 4 ? stringToReal(strs[3]) : 1.0);

    return QColor(r*255, g*255, b*255, a*255);
  }

  return QColor(str);
}

inline QString colorToString(const QColor &c) {
  return c.name();
}

inline QColor RGBAToQColor(const CRGBA &c) {
  return QColor(int(255*c.getRed  ()),
                int(255*c.getGreen()),
                int(255*c.getBlue ()),
                int(255*c.getAlpha()));
}

inline CRGBA QColorToRGBA(const QColor &c) {
  return CRGBA(c.red()/255.0, c.green()/255.0, c.blue()/255.0, c.alpha()/255.0);
}

//---

inline QString rangeToString(CQTcl *tcl, const CDisplayRange2D &range) {
  double x1, y1, x2, y2;
  range.getWindowRange(&x1, &y1, &x2, &y2);

  QStringList strs;
  strs << Util::realToString(x1);
  strs << Util::realToString(y1);
  strs << Util::realToString(x2);
  strs << Util::realToString(y2);

  return tcl->mergeList(strs);
}

inline void stringToRange(CQTcl *tcl, CDisplayRange2D &range, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  if (strs.size() == 4) {
    auto x1 = Util::stringToReal(strs[0]);
    auto y1 = Util::stringToReal(strs[1]);
    auto x2 = Util::stringToReal(strs[2]);
    auto y2 = Util::stringToReal(strs[3]);

    range.setWindowRange(x1, y1, x2, y2);
  }
}

inline Rect stringToRect(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  Point ll, ur;

  if (strs.size() > 4 && strs[2] == "px") {
    ll.x.units = Units::PIXEL;
    ll.y.units = Units::PIXEL;
    ur.x.units = Units::PIXEL;
    ur.y.units = Units::PIXEL;
  }

  if (strs.size() >= 4) {
    auto x1 = Util::stringToReal(strs[0]);
    auto y1 = Util::stringToReal(strs[1]);
    auto x2 = Util::stringToReal(strs[2]);
    auto y2 = Util::stringToReal(strs[3]);

    ll.x.value = std::min(x1, x2);
    ll.y.value = std::min(y1, y2);
    ur.x.value = std::max(x1, x2);
    ur.y.value = std::max(y1, y2);
  }

  Rect rect;

  rect.ll = ll;
  rect.ur = ur;

  return rect;
}

inline QString rectToString(const Rect &r) {
  auto x1str = QString::number(r.ll.x.value);
  auto y1str = QString::number(r.ll.y.value);
  auto x2str = QString::number(r.ur.x.value);
  auto y2str = QString::number(r.ur.y.value);

  auto str = x1str + " " + y1str + " " + x2str + " " + y2str;

  if (r.ll.x.units == Units::PIXEL)
    str += " px";

  return str;
}

inline QString coordToString(const Coord &coord) {
  auto str = QString::number(coord.value);

  if (coord.units == Units::PIXEL)
    str += "px";

  return str;
}

inline Coord stringToCoord(const QString &str) {
  Coord coord;

  auto str1 = str;

  if (str1.right(2) == "px") {
    coord.units = Units::PIXEL;

    str1 = str1.mid(0, str1.length() - 2);
  }

  coord.value = Util::stringToReal(str1);

  return coord;
}

inline Point stringToPoint(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  Point p;

  if (strs.size() > 2 && strs[2] == "px") {
    p.x.units = Units::PIXEL;
    p.y.units = Units::PIXEL;
  }

  if (strs.size() >= 2) {
    auto x = Util::stringToReal(strs[0]);
    auto y = Util::stringToReal(strs[1]);

    p.x.value = x;
    p.y.value = y;
  }

  return p;
}

inline QString pointToString(const Point &p) {
  auto xstr = QString::number(p.x.value);
  auto ystr = QString::number(p.y.value);

  auto str = xstr + " " + ystr;

  if (p.x.units == Units::PIXEL)
    str += " px";

  return str;
}

inline QString point2DToString(const CPoint2D &p) {
  auto xstr = QString::number(p.x);
  auto ystr = QString::number(p.y);

  return xstr + " " + ystr;
}

inline QString point3DToString(const CPoint3D &p) {
  auto xstr = QString::number(p.x);
  auto ystr = QString::number(p.y);
  auto zstr = QString::number(p.z);

  return xstr + " " + ystr + " " + zstr;
}

inline bool stringToPoint3D(CQTcl *tcl, const QString &str, CPoint3D &p) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  if (strs.size() < 3)
    return false;

  double x, y, z;
  if (! stringToReal(strs[0], x) || ! stringToReal(strs[1], y) || ! stringToReal(strs[2], z))
    return false;

  p.x = x;
  p.y = y;
  p.z = z;

  return true;
}

//---

inline CVector2D stringToVector2D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CVector2D p;

  if (strs.size() >= 2) {
    auto x = stringToReal(strs[0]);
    auto y = stringToReal(strs[1]);

    p = CVector2D(x, y);
  }

  return p;
}

inline CGLVector2D stringToGLVector2D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CGLVector2D p;

  if (strs.size() >= 2) {
    auto x = stringToReal(strs[0]);
    auto y = stringToReal(strs[1]);

    p = CGLVector2D(x, y);
  }

  return p;
}

inline std::vector<CVector2D> stringToVectors2D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  std::vector<CVector2D> points;

  for (const auto &str : strs) {
    auto p = stringToVector2D(tcl, str);

    points.push_back(p);
  }

  return points;
}

inline CPoint3D stringToPoint3D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CPoint3D p;

  if (strs.size() >= 3) {
    auto x = stringToReal(strs[0]);
    auto y = stringToReal(strs[1]);
    auto z = stringToReal(strs[2]);

    p = CPoint3D(x, y, z);
  }

  return p;
}

inline bool stringToVector3D(CQTcl *tcl, const QString &str, CVector3D &v) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  if (strs.size() < 3)
    return false;

  CVector3D p;

  double x, y, z;
  if (! stringToReal(strs[0], x) || ! stringToReal(strs[1], y) || ! stringToReal(strs[2], z))
    return false;

  v = CVector3D(x, y, z);

  return true;
}

inline CVector3D stringToVector3D(CQTcl *tcl, const QString &str) {
  CVector3D v;
  (void) stringToVector3D(tcl, str, v);
  return v;
}

inline CGLVector3D stringToGLVector3D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CGLVector3D p;

  if (strs.size() >= 3) {
    auto x = stringToReal(strs[0]);
    auto y = stringToReal(strs[1]);
    auto z = stringToReal(strs[2]);

    p = CGLVector3D(x, y, z);
  }

  return p;
}

inline QString vector3DToString(const CVector3D &p) {
  auto xstr = QString::number(p.x());
  auto ystr = QString::number(p.y());
  auto zstr = QString::number(p.z());

  return xstr + " " + ystr + " " + zstr;
}

inline QString vector3DToString(const CGLVector3D &p) {
  auto xstr = QString::number(p.x());
  auto ystr = QString::number(p.y());
  auto zstr = QString::number(p.z());

  return xstr + " " + ystr + " " + zstr;
}

inline std::vector<CVector3D> stringToVectors3D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  std::vector<CVector3D> points;

  for (const auto &str : strs) {
    auto p = stringToVector3D(tcl, str);

    points.push_back(p);
  }

  return points;
}

//---

inline QString bbox3DToString(const CBBox3D &bbox) {
  auto x1str = QString::number(bbox.getXMin());
  auto y1str = QString::number(bbox.getYMin());
  auto z1str = QString::number(bbox.getZMin());
  auto x2str = QString::number(bbox.getXMax());
  auto y2str = QString::number(bbox.getYMax());
  auto z2str = QString::number(bbox.getZMax());

  return x1str + " " + y1str + " " + z1str + " " + x2str + " " + y2str + " " + z2str;
}

//---

inline CPoint2D stringToPoint2D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CPoint2D p;

  if (strs.size() >= 2) {
    auto x = stringToReal(strs[0]);
    auto y = stringToReal(strs[1]);

    p.x = x;
    p.y = y;
  }

  return p;
}

//---

inline bool stringToIntArray(CQTcl *tcl, const QString &str, std::vector<int> &a) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  a.resize(strs.size());

  for (int i = 0; i < strs.size(); ++i) {
    int ii;
    if (! stringToInt(strs[i], ii))
      return false;

    a[i] = ii;
  }

  return true;
}

inline std::vector<unsigned int> stringToUIntArray(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  std::vector<unsigned int> integers;

  for (const auto &str : strs) {
    auto i = stringToInt(str);

    integers.push_back(i);
  }

  return integers;
}

//---

inline CGLColor stringToGLColor(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CGLColor c;

  if (strs.size() >= 3) {
    auto r = stringToReal(strs[0]);
    auto g = stringToReal(strs[1]);
    auto b = stringToReal(strs[2]);
    auto a = 1.0;

    if (strs.size() >= 4)
      a = stringToReal(strs[3]);

    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
  }
  else {
    QColor c1(str);

    c.r = c1.redF  ();
    c.g = c1.greenF();
    c.b = c1.blueF ();
    c.a = 1.0;
  }

  return c;
}

inline QColor stringToQColor(CQTcl *tcl, const QString &str) {
  auto c = stringToGLColor(tcl, str);

  QColor c1;

  c1.setRgbF(c.r, c.g, c.b, c.a);

  return c1;
}

inline std::vector<CGLColor> stringToColors(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  std::vector<CGLColor> colors;

  for (const auto &str : strs) {
    auto c = stringToGLColor(tcl, str);

    colors.push_back(c);
  }

  return colors;
}

inline QString colorToString(const CGLColor &c) {
  auto rstr = QString::number(c.r);
  auto gstr = QString::number(c.g);
  auto bstr = QString::number(c.b);
  auto astr = QString::number(c.a);

  return rstr + " " + gstr + " " + bstr + " " + astr;
}

inline CGLColor qcolorToColor(const QColor &c) {
  return CGLColor(c.redF(), c.greenF(), c.blueF(), c.alphaF());
}

inline QColor colorToQColor(const CGLColor &c) {
  return QColor(c.r*255, c.g*255, c.b*255, c.a*255);
}

//---

inline QVector3D toVector(const CGLColor &c) {
  return QVector3D(c.r, c.g, c.b);
}

//---

inline double degToRad(double d) {
  return M_PI*d/180;
}

inline double radToDeg(double d) {
  return 180.0*d/M_PI;
}

//---

inline QPainterPath stringToPath(const QString &str) {
  QPainterPath path;
  (void) CQSVGUtil::stringToPath(str, path);
  return path;
}

inline QString pathToString(const QPainterPath &path) {
  return CQSVGUtil::pathToString(path);
}

//---

inline QString alignToString(Qt::Alignment align) {
  QString str;

  if (align == Qt::AlignCenter)
    return "Center";

  if      (align & Qt::AlignLeft   ) str += "Left";
  else if (align & Qt::AlignRight  ) str += "Right";
  else if (align & Qt::AlignHCenter) str += "HCenter";

  if      (align & Qt::AlignTop    ) str += "Top";
  else if (align & Qt::AlignBottom ) str += "Bottom";
  else if (align & Qt::AlignVCenter) str += "VCenter";

  return str;
}

inline Qt::Alignment stringToAlign(const QString &str) {
  Qt::Alignment align = Qt::AlignmentFlag(0);

  auto str1 = str.toLower();

  if (str1 == "center")
    return Qt::AlignCenter;

  if      (str1.left(4) == "left"   ) align |= Qt::AlignLeft;
  else if (str1.left(5) == "right"  ) align |= Qt::AlignRight;
  else if (str1.left(7) == "hcenter") align |= Qt::AlignHCenter;
  else                                align |= Qt::AlignHCenter;

  if      (str1.right(3) == "top"    ) align |= Qt::AlignTop;
  else if (str1.right(6) == "bottom" ) align |= Qt::AlignBottom;
  else if (str1.right(7) == "vcenter") align |= Qt::AlignVCenter;
  else                                 align |= Qt::AlignVCenter;

  return align;
}

//---

}

}

#endif
