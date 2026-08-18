#ifndef CQSandboxUtil_H
#define CQSandboxUtil_H

#include <CQTclUtil.h>
#include <CGLVector2D.h>
#include <CGLVector3D.h>
#include <CBBox3D.h>
#include <CGLColor.h>

#include <CPoint3D.h>

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

inline CVector3D stringToVector3D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CVector3D p;

  if (strs.size() >= 3) {
    auto x = stringToReal(strs[0]);
    auto y = stringToReal(strs[1]);
    auto z = stringToReal(strs[2]);

    p = CVector3D(x, y, z);
  }

  return p;
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

}

}

#endif
