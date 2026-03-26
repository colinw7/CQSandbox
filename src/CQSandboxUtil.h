#ifndef CQSandboxUtil_H
#define CQSandboxUtil_H

#include <CQTclUtil.h>
#include <CGLVector2D.h>
#include <CGLVector3D.h>
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

inline int stringToInt(const QString &s) {
  bool ok;
  return s.toInt(&ok);
}

inline double stringToReal(const QString &s) {
  bool ok;
  return s.toDouble(&ok);
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

inline QColor stringToColor(const QString &str) {
  return QColor(str);
}

inline QString colorToString(const QColor &c) {
  return c.name();
}

//---

inline QString point3DToString(const CPoint3D &p) {
  auto xstr = QString::number(p.x);
  auto ystr = QString::number(p.y);
  auto zstr = QString::number(p.z);

  return xstr + " " + ystr + " " + zstr;
}

inline CPoint3D stringToPoint3D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CPoint3D p;

  if (strs.size() >= 3) {
    bool ok;
    auto x = strs[0].toDouble(&ok);
    auto y = strs[1].toDouble(&ok);
    auto z = strs[2].toDouble(&ok);

    p.x = x;
    p.y = y;
    p.z = z;
  }

  return p;
}

//---

inline CGLVector2D stringToVector2D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CGLVector2D p;

  if (strs.size() >= 2) {
    bool ok;
    auto x = strs[0].toDouble(&ok);
    auto y = strs[1].toDouble(&ok);

    p = CGLVector2D(x, y);
  }

  return p;
}

inline std::vector<CGLVector2D> stringToVectors2D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  std::vector<CGLVector2D> points;

  for (const auto &str : strs) {
    auto p = stringToVector2D(tcl, str);

    points.push_back(p);
  }

  return points;
}

inline CGLVector3D stringToVector3D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CGLVector3D p;

  if (strs.size() >= 3) {
    bool ok;
    auto x = strs[0].toDouble(&ok);
    auto y = strs[1].toDouble(&ok);
    auto z = strs[2].toDouble(&ok);

    p = CGLVector3D(x, y, z);
  }

  return p;
}

inline QString vector3DToString(const CGLVector3D &p) {
#if 1
  auto xstr = QString::number(p.x());
  auto ystr = QString::number(p.y());
  auto zstr = QString::number(p.z());

  return xstr + " " + ystr + " " + zstr;
#else
  return QString::fromStdString(CUtil::toString(p));
#endif
}

inline std::vector<CGLVector3D> stringToVectors3D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  std::vector<CGLVector3D> points;

  for (const auto &str : strs) {
    auto p = stringToVector3D(tcl, str);

    points.push_back(p);
  }

  return points;
}

//---

inline CPoint2D stringToPoint2D(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CPoint2D p;

  if (strs.size() >= 2) {
    bool ok;
    auto x = strs[0].toDouble(&ok);
    auto y = strs[1].toDouble(&ok);

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
    bool ok;
    auto i = str.toInt(&ok);

    integers.push_back(i);
  }

  return integers;
}

//---

inline CGLColor stringToColor(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  CGLColor c;

  if (strs.size() >= 3) {
    bool ok;
    auto r = strs[0].toDouble(&ok);
    auto g = strs[1].toDouble(&ok);
    auto b = strs[2].toDouble(&ok);
    auto a = 1.0;

    if (strs.size() >= 4)
      a = strs[3].toDouble(&ok);

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
  auto c = stringToColor(tcl, str);

  QColor c1;

  c1.setRgbF(c.r, c.g, c.b, c.a);

  return c1;
}

inline std::vector<CGLColor> stringToColors(CQTcl *tcl, const QString &str) {
  QStringList strs;
  (void) tcl->splitList(str, strs);

  std::vector<CGLColor> colors;

  for (const auto &str : strs) {
    auto c = stringToColor(tcl, str);

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

}

}

#endif
