#ifndef CQSandboxUtil_H
#define CQSandboxUtil_H

#include <CPoint3D.h>

#include <QString>

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

//---

}

}

#endif
